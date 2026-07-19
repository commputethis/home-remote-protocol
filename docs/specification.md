# Home Remote Protocol Specification

**Version:** 0.1.0 (Draft)

---

## 1. Introduction

The Home Remote Protocol defines a transport-independent messaging protocol
used by Home Remote devices to exchange state, commands, telemetry, and
configuration information.

The protocol is intentionally independent of:

- ESP-NOW
- Wi-Fi
- Bluetooth
- Ethernet
- Home Assistant
- ESPHome
- LVGL
- any specific hardware platform

Any transport capable of delivering binary packets may carry Home Remote
Protocol messages.

---

## 2. Design Goals

The protocol is designed to be:

- Small enough for embedded devices
- Transport independent
- Versioned
- Extensible
- Backwards compatible whenever practical
- Easy to implement in C++, Python, Rust, Go, or other languages

---

## 3. Terminology

| Term | Definition |
| ----- | ------------ |
| Packet | Complete binary message transmitted over a transport |
| Header | Fixed-length packet metadata |
| Payload | Message-specific binary data |
| Sender | Device originating a packet |
| Receiver | Device processing a packet |
| Broadcast | Packet addressed to all devices |

---

## 4. Packet Layout

Every packet consists of:

```text
+------------+
| Header     |
+------------+
| Payload    |
+------------+
| CRC16      |
+------------+
```

---

## 5. Packet Header

The header SHALL contain:

| Field | Size | Description |
| ---- | ---- | ----------- |
| Magic | 2 bytes | Packet identifier |
| Version | 1 byte | Protocol version |
| Message Type | 1 byte | Type of payload |
| Sequence Number | 2 bytes | Sender sequence |
| Source Device ID | 2 bytes | Originating device |
| Destination Device ID | 2 bytes | Intended recipient or broadcast |
| Payload Length | 2 bytes | Bytes following header |
| Flags | 1 byte | Reserved for future use |

All multi-byte values SHALL be encoded little-endian.

---

## 6. CRC

The packet SHALL end with a CRC-16.

The CRC SHALL cover:

- Header
- Payload

The CRC field itself is excluded.

(Algorithm to be finalized.)

---

## 7. Device IDs

Each device has a unique 16-bit identifier.

Reserved IDs:

| Value | Meaning |
| ------ | -------- |
| 0x0000 | Invalid |
| 0xFFFF | Broadcast |

Future reserved ranges may be defined.

---

## 8. Message Types

Each packet SHALL contain exactly one message.

The payload format is determined entirely by the Message Type field.

Unknown message types SHALL be ignored.

---

## 9. Routing

Packets SHALL be accepted when:

- Destination equals local device
- Destination is broadcast

All other packets SHALL be ignored.

Future routers or repeaters may forward packets.

---

## 10. Sequence Numbers

Each sender maintains an independent 16-bit sequence number.

Sequence numbers SHALL increment by one for every transmitted packet.

Receivers MAY use sequence numbers to detect duplicates.

Sequence numbers wrap from:

65535 → 0

---

## 11. Acknowledgements

Messages requiring reliability SHALL define an acknowledgement.

An ACK contains:

- Original sequence number
- Result code

Future versions may define negative acknowledgements.

---

## 12. Version Compatibility

Unsupported protocol versions SHALL be rejected.

Minor revisions SHOULD remain compatible whenever practical.

Major revisions MAY introduce incompatible changes.

---

## 13. Reserved Fields

Receivers SHALL ignore unknown flag bits.

Reserved values SHALL NOT be repurposed without a protocol revision.

---

## 14. Error Handling

Malformed packets SHALL be discarded.

CRC failures SHALL be discarded.

Unsupported versions SHALL be rejected.

Unknown message types SHALL be ignored.

---

## 15. Transport Requirements

The protocol assumes the transport:

- Preserves packet boundaries
- Delivers packets without modification

Ordering and reliability are transport dependent.

---

## 16. Future Expansion

Reserved for:

- Encryption
- Authentication
- Compression
- Multi-hop routing
- Device discovery
- Time synchronization

---

## Resource model

A resource is a logical object that exposes properties.

Resources and properties are identified by unsigned 16-bit identifiers.
The interpretation of those identifiers is application-defined unless a
future protocol profile explicitly standardizes them.

A protocol implementation MUST NOT assume that a resource represents a
light, fan, sensor, switch, or any other particular device class.

## Value types

The initial protocol version defines these value types:

| Value | Name | Encoded size |
| ------: | ------ | -------------: |
| 0x00 | NONE | 0 bytes |
| 0x01 | BOOL | 1 byte |
| 0x02 | UINT8 | 1 byte |
| 0x03 | UINT16 | 2 bytes |
| 0x04 | INT16 | 2 bytes |
| 0x05 | UINT32 | 4 bytes |
| 0x06 | INT32 | 4 bytes |
| 0x07 | FLOAT32 | 4 bytes |
| 0x08 | UTF8 | variable |
| 0x09 | BYTES | variable |

Multi-byte numeric values use the byte order defined by this specification.

Boolean values MUST be encoded as `0x00` for false and `0x01` for true.
Other Boolean encodings are invalid.

UTF-8 and byte-string values MUST include an explicit length.

Unknown value types MUST cause the containing message to be rejected.

## Resource command payload

A resource command requests a change to one property.

| Field | Size |
| ------ | ------ |
| Resource ID | 2 bytes |
| Property ID | 2 bytes |
| Value Type | 1 byte |
| Value Length | 2 bytes |
| Value | Value Length bytes |

A successful acknowledgement confirms that the command was accepted or
processed. It does not replace a subsequent authoritative state update.

## Resource state payload

A resource state message publishes the authoritative value of one property.

| Field | Size |
| ------ | ------ |
| Resource ID | 2 bytes |
| Property ID | 2 bytes |
| Value Type | 1 byte |
| Value Length | 2 bytes |
| Value | Value Length bytes |

State may be sent in response to a query or published unsolicited.

## Resource query payload

A resource query requests state.

| Field | Size |
| ------ | ------ |
| Resource ID | 2 bytes |
| Property ID | 2 bytes |

Reserved values allow querying all properties of a resource or all
resources visible to the receiver.

## Identifier namespaces

Resource and property identifiers are interpreted within an application
profile.

| Range | Use |
| ------ | ----- |
| 0x0000 | Invalid or unspecified |
| 0x0001–0x7FFF | Application-assigned |
| 0x8000–0xFFFE | Reserved for future standardized profiles |
| 0xFFFF | Wildcard where permitted |

A wildcard value is valid only in message fields that explicitly allow it.
