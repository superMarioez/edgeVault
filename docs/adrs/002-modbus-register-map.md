# ADR-002: [ Modbus register map configuration strategy ]

## Status

Accepted

## Context

EdgeVault acquires data from modbus slave devices, each device has it's own register map that defines from where to read the data, it's scaling factor, it's unit, circuit breaker thresholds, etc..

The architectural question is where to store/configure those register maps, shall we make it compile-time, defined at boot, or run-time configurations.

## Decision

For now we will stick to the KConfig option (compile-time) hardcoding one register map for one modbus sensor for the sake of testing the functionality of the modbus sensors, later we will migrate to run-time configuration via NVS+HTTP (web-UI).

Each entry will carry slave_id, function code, count, type, scale, unit, circuit-breaker threshold (per device).

The migration trigger would be that 'http_server_task' is oeprational in phase 3, the register map moves to NVS, KConfig values become initial defaults at boots.

## Alternatives Considered

JSON file in SPIFFS - rejected
JSON+SPIFFS requires a JSON parser, defining a schema, and still requires the user to rebuild and reflash the data partition, the cost is similar to building the HTTP UI directly, without getting the benefit of the runtime UX.

## Consequences

- Positive

Getting the modbus protocol working and tested quickly.

- Negative

If later the edgeVault device gets an update via the OTA while it's deployed in a factory, and that update has the NVS+HTTP functionality implemented, if it boots and finds the NVS empty, we will have to write a script for migrating from KConfig to NVS+HTTP UX, taking the already-existing values from KConfig and treating them as defaults for the NVS database, that work is deferred but not free.

If we discover a modbus sensor that needs > 5 registers polled, the KConfig flat-list approach becomes unwieldy and we may need to migrate earlier than planned.

## Date

2026-05-01