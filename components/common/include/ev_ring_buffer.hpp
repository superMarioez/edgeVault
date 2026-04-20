#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <cstddef>
#include <cstring>


namespace ev {


    template<typename T>
    class RingBuffer {

        public:
        RingBuffer(size_t capacity);
        ~RingBuffer();

        RingBuffer(const RingBuffer&) = delete;
        RingBuffer& operator=(const RingBuffer&) = delete;
        RingBuffer(RingBuffer&& other) noexcept = delete;
        RingBuffer& operator=(RingBuffer&& other) noexcept = delete;

        void push(const T& item);
        void peek_latest(T& out) const;
        size_t read_last_n(T* out, size_t n) const;
        size_t count() const;

        private:
        T* buffer_;
        size_t cap_;
        size_t head_;
        size_t count_;
        SemaphoreHandle_t mutex_;
    };


}