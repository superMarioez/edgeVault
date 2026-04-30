#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace ev {


    template<typename T>
    class RingBuffer {

        static_assert(
            std::is_trivially_copyable<T>::value,
            "FATAL ARCHITECTURE ERROR: RingBuffer only accepts trivially-copyable types");

        public:

            explicit RingBuffer(size_t capacity) : cap_(capacity) {

                // assert on capacity 0
                configASSERT(capacity > 0);

                // allocate in PSRAM
                buffer_ = static_cast<T*>(heap_caps_malloc(cap_ * sizeof(T), MALLOC_CAP_SPIRAM));

                // check for null (PSRAM full)
                ESP_ERROR_CHECK((buffer_) ? ESP_OK : ESP_ERR_NO_MEM);

                // create the mutex
                mutex_ = xSemaphoreCreateMutex();
                ESP_ERROR_CHECK(mutex_? ESP_OK : ESP_ERR_NO_MEM);

                head_ = 0;
                count_ = 0;
            }

            ~RingBuffer() {
                if (mutex_ != nullptr) vSemaphoreDelete(mutex_);
                if (buffer_ != nullptr) heap_caps_free(buffer_);
            }

            RingBuffer(const RingBuffer&) = delete;
            RingBuffer& operator=(const RingBuffer&) = delete;
            RingBuffer(RingBuffer&& other) noexcept = delete;
            RingBuffer& operator=(RingBuffer&& other) noexcept = delete;

            bool push(const T& item) {

                bool ret = xSemaphoreTake(mutex_, pdMS_TO_TICKS(100));
                if (!ret) return false;

                buffer_[head_] = item;
                head_ = (head_ + 1) % cap_;
                count_ = (count_ + 1 >= cap_)? cap_ : count_ + 1 ;

                static_cast<void>(xSemaphoreGive(mutex_));
                return true;
            }

            bool peek_latest(T& out) const {

                bool ret = xSemaphoreTake(mutex_, pdMS_TO_TICKS(100));
                if (!ret) return ret;

                if (!count_) {
                    xSemaphoreGive(mutex_);
                    return false;
                }
                out = buffer_[(head_ - 1 + cap_) % cap_];

                static_cast<void>(xSemaphoreGive(mutex_));
                return true;
            }

            size_t read_last_n(T* out, size_t n) const {

                if (!xSemaphoreTake(mutex_, pdMS_TO_TICKS(100))) return 0;

                size_t cnt = (n > count_)? count_ : n;
                size_t start = (head_ - cnt + cap_) % cap_;

                // wrap check
                if (start + cnt > cap_) {
                    // wraps
                    memcpy(out, buffer_ + start, (cap_ - start) * sizeof(T));
                    memcpy(out + cap_ - start, buffer_, (cnt - (cap_ - start)) * sizeof(T));
                }
                else {
                    memcpy(out, buffer_ + start, cnt * sizeof(T));
                }

                static_cast<void>(xSemaphoreGive(mutex_));

                return cnt;
            }

            size_t count() const {
                if (!xSemaphoreTake(mutex_, pdMS_TO_TICKS(100))) return 0;
                const size_t current_cnt = count_;
                static_cast<void>(xSemaphoreGive(mutex_));
                return current_cnt;
            }

        private:

            T* buffer_;
            size_t cap_;
            size_t head_;
            size_t count_;
            mutable SemaphoreHandle_t mutex_;
    };


}