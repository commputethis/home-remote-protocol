# ADR 0003: Use a versioned typed message protocol

- Status: Accepted
- Supercedes: commputethis/home-remote-platform ADR 0003
- Date: 2026-07-19

## Context

Single-byte commands are sufficient for a prototype but cannot safely support state synchronization, batteries, acknowledgements, multiple devices, or compatibility checks.

## Decision

Use typed packets with a common header, sequence number, device ID, payload length, and checksum.

## Consequences

- Invalid and incompatible packets can be rejected.
- Duplicate commands can be detected.
- The protocol can evolve without silently misinterpreting messages.
