# ADR 0002: Use resource-oriented messages

- Status: Accepted
- Date: 2026-07-19

## Context

Home Remote must represent commands and state for lights, fans, switches,
sensors, and future device types without adding a new wire-level message
for every kind of device.

Domain-specific messages such as `FanState` and `LightCommand` would couple
the protocol to Home Remote Platform concepts and require protocol changes
whenever new device classes are introduced.

## Decision

State and command messages will identify:

- a resource
- a property of that resource
- a typed value

The protocol defines the binary representation of resource identifiers,
property identifiers, value types, and values.

The protocol does not define the application meaning of a particular
resource or property identifier. Those mappings belong to the application
using the protocol.

Control-plane messages such as acknowledgements, discovery, and protocol
errors may continue to use dedicated message types.

## Consequences

- New application resource types generally do not require protocol changes.
- Implementations can use the same serialization and validation code for
  many kinds of devices.
- Applications must maintain a resource and property registry or mapping.
- Unknown resources and properties must be handled safely.
- Value types and encoding rules must be specified precisely.
