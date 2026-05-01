# ADR-001: [Two-producer single-consumer Queue for sensor data]

## Status

Accepted

## Context

EdgeVault acquires it's data from two sources, physically and temporally:

- temperature, humidity, pressure, voltage, etc.. sensors (I2C, ADC, ..)
- Modbus-RTU sensors (UART/RS-485)

Those use different protocols, modbus diverges from normal protocols in it's inter-framing (the 3.5 characters time), and also it's error handling model, it uses CRC, no response timeouts, exception codes.

The architectural question is whether to use a shared queue for both sources/producers, or a separate queues, one for each task.

## Decision

Using a shared queue sensor_data_queue that's constructed in the main context and injected into whatever task's context that needs to use it, producers and consumer.

## Alternatives Considered

1. **Using separate queue for each source** — forces the data pipeline task to spin-poll multiple queues or block on one and starve the other.

2. **Direct function calls from producers into the data pipeline** — couples producer task timing to consumer execution.

## Consequences

Positive:

- Producer tasks never block on consumer work; back-pressure is visible as 'errQUEUE_FULL' which we handle by discarding the oldest reading.

- Adding a third producer later requires no changes to the consumer.

- Each task is independently testable: a fake producer can write into the queue; a fake consumer can drain it.

Negative / May be changed later:

- Queue depth (`SENSOR_QUEUE_DEPTH = 30`) is a global tuning knob. If Modbus polling becomes much slower than the consumer, the current "discard oldest" policy may drop fresh I2C readings unfairly. If this becomes a problem, the fix is either separate queues per priority class, or a priority field on `SensorReading` consulted by a smarter discard policy.

- Heterogeneous payloads (e.g., a multi-register Modbus block read that returns 10 values atomically) do not fit the single-reading schema. If we need atomic multi-register transactions, this ADR
must be revisited.

## Date

2026-05-01