# ADR-001: Two-Producer, Single-Consumer Queue for Sensor Data

## Status

Accepted - 2026-04-30

## Context

EdgeVault acquires data from two physically and temporally independent sources

- I2C/ADC sensors (LM75A, DS3231 temperature, battery ADC) polled on a fast cadence (~1 s) by 'sensor_hub_task'
- Modbus-RTU sensors over RS-485 polled on a slow cadence (~% s) by 'modbus_master_task'. Modbus has its own inter-frame error model (CRC failures, no-response timeouts, exception codes).

Downstream, 'data_pipeline_task must apply per-sensor calibration, fan reading out to the SD logger, the PSRAM ring buffer (for the display), and (later) the MQTT publish queue.

The architectural question is how producers and consumers are wired:
is there one shared queue? two queues? or direct task-to-task calls?

## Decision

Use a single FreeRTOS queue ('sensor_data_queue') carrying 'ev::SensorReading' items. Both 'sensor_hub_task' and 'modbus_master_task' are producers. `data_pipeline_task` is the sole
consumer. The `SensorSource` field in each reading distinguishes the
origin.

The queue handle is created in `app_main` and injected into each
task's context struct, not retrieved through a global.

## Alternatives Considered

1. **One queue per source** (`i2c_q`, `modbus_q`).
   Rejected: forces `data_pipeline_task` to either spin polling
   multiple queues or block on one and starve the other. Adds API
   surface (more handles to plumb) without solving a real problem,
   since `SensorReading` already carries source identity.

2. **Direct function calls from producers into the data pipeline.**
   Rejected: couples producer task timing to consumer execution.
   A blocking SD card write inside the pipeline would back-pressure
   the Modbus task and could miss the 3.5 character-time window.
   The queue decouples timing domains, which is the whole point of
   having separate tasks.

3. **Shared ring buffer protected by a mutex** instead of a queue.
   Rejected: FreeRTOS queues already provide blocking semantics,
   ISR-safe variants, and a clean ownership model. Reinventing this
   with a mutex+buffer adds bug surface for no gain.

## Consequences

Positive:

- Producer tasks never block on consumer work; back-pressure is
  visible as `errQUEUE_FULL`, which we handle today by discarding
  the oldest reading.
- Adding a third producer later (e.g., a BME280 over a different
  I2C bus, or a future CAN-attached sensor) requires no changes to
  the consumer.
- Each task is independently testable: a fake producer can write
  into the queue; a fake consumer can drain it.

Negative / what we'd change later:

- Queue depth (`SENSOR_QUEUE_DEPTH = 30`) is a global tuning knob.
  If Modbus polling becomes much slower than the consumer, the
  current "discard oldest" policy may drop fresh I2C readings
  unfairly. If this becomes a problem, the fix is either separate
  queues per priority class, or a priority field on `SensorReading`
  consulted by a smarter discard policy.
- Heterogeneous payloads (e.g., a multi-register Modbus block read
  that returns 10 values atomically) do not fit the single-reading
  schema. If we need atomic multi-register transactions, this ADR
  must be revisited.