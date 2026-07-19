# Home Remote Protocol Wire Format

**Document version:** 0.1.0-draft  
**Protocol version:** 1  
**Status:** Draft

This document defines the byte-level encoding of Home Remote Protocol packets. It is normative for all protocol implementations. The higher-level meaning and use of each message type is defined in [`message-catalog.md`](message-catalog.md).

## 1. Conformance language

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHALL**, **SHALL NOT**, **SHOULD**, **SHOULD NOT**, **RECOMMENDED**, **MAY**, and **OPTIONAL** indicate requirement levels.

## 2. General encoding rules

- Every packet is a contiguous sequence of bytes.
- All multi-byte integers MUST use little-endian byte order.
- Packet and payload structures MUST be serialized explicitly. Implementations MUST NOT transmit compiler-native structures directly because padding and alignment are implementation-dependent.
- Packet boundaries are supplied by the transport. The protocol does not include a start delimiter or escaping scheme.
- A packet MUST contain exactly one header, zero or one payload, and one CRC field.

## 3. Packet layout

```text
+----------------------+ offset 0
| Fixed header         | 13 bytes
+----------------------+ offset 13
| Payload              | payload_length bytes
+----------------------+ offset 13 + payload_length
| CRC-16               | 2 bytes
+----------------------+
```

The total packet length is:

```text
15 + payload_length bytes
```

The minimum packet length is 15 bytes.

## 4. Fixed header

| Offset | Size | Field | Encoding |
|------:|----:|---|---|
| `0x00` | 2 | Magic | Two literal bytes: `0x48 0x52` (`HR`) |
| `0x02` | 1 | Protocol version | Unsigned 8-bit integer |
| `0x03` | 1 | Message type | Unsigned 8-bit integer |
| `0x04` | 2 | Sequence number | Unsigned 16-bit little-endian integer |
| `0x06` | 2 | Source device ID | Unsigned 16-bit little-endian integer |
| `0x08` | 2 | Destination device ID | Unsigned 16-bit little-endian integer |
| `0x0A` | 2 | Payload length | Unsigned 16-bit little-endian integer |
| `0x0C` | 1 | Flags | Bit field; all bits reserved in protocol version 1 |

### 4.1 Magic

The magic field MUST contain:

```text
48 52
```

A receiver MUST discard a packet whose magic field does not match.

### 4.2 Protocol version

Protocol version 1 is encoded as:

```text
01
```

A receiver MUST reject packets with an unsupported protocol version.

### 4.3 Message type

The message type determines the payload structure. Message type assignments are defined in [Section 8](#8-message-type-assignments).

A receiver that does not recognize a message type MUST ignore the message. It MAY report an unsupported-message error when a reply is appropriate.

### 4.4 Sequence number

Each sender maintains its own unsigned 16-bit sequence counter.

- The counter SHOULD increment by one for each newly originated packet.
- Retransmission of the same logical packet MUST reuse the original sequence number.
- The counter wraps from `0xFFFF` to `0x0000`.
- A receiver MAY use the source device ID, sequence number, and message type together to identify duplicate retransmissions.

Sequence number `0x0000` is valid.

### 4.5 Device IDs

Device IDs are unsigned 16-bit values.

| Value | Meaning |
|---:|---|
| `0x0000` | Invalid or unassigned device ID |
| `0x0001`–`0xFFFE` | Assignable device IDs |
| `0xFFFF` | Broadcast destination |

The source device ID MUST NOT be `0x0000` or `0xFFFF`.

A receiver accepts a packet when the destination device ID is either:

- the receiver's own device ID; or
- `0xFFFF`.

A receiver that is not acting as a router MUST ignore all other destination IDs.

### 4.6 Payload length

Payload length is the number of payload bytes following the fixed header. It excludes both the fixed header and the CRC field.

A receiver MUST verify that:

```text
received_packet_length == 15 + payload_length
```

A packet that fails this check is malformed and MUST be discarded.

### 4.7 Flags

All flag bits are reserved in protocol version 1.

- Senders MUST set the flags field to `0x00`.
- Receivers MUST ignore unknown flag bits after validating all mandatory fields.
- A future protocol revision may assign meanings to individual bits.

## 5. CRC-16

Packets end with a CRC-16/CCITT-FALSE checksum.

| Parameter | Value |
|---|---|
| Width | 16 bits |
| Polynomial | `0x1021` |
| Initial value | `0xFFFF` |
| Input reflected | No |
| Output reflected | No |
| Final XOR | `0x0000` |
| Check value for ASCII `123456789` | `0x29B1` |

The CRC covers:

```text
fixed header + payload
```

The CRC field itself is excluded from the calculation.

The computed CRC is appended as a little-endian unsigned 16-bit value:

```text
crc_low_byte, crc_high_byte
```

A receiver MUST discard a packet whose CRC does not match.

## 6. Resource model

Resource-oriented messages address one property of one logical resource.

| Field | Size | Description |
|---|---:|---|
| Resource ID | 2 bytes | Application-defined logical resource identifier |
| Property ID | 2 bytes | Application-defined property identifier |

Resource and property identifiers use independent namespaces from device IDs.

### 6.1 Identifier ranges

| Value or range | Meaning |
|---:|---|
| `0x0000` | Invalid or unspecified |
| `0x0001`–`0x7FFF` | Application-assigned |
| `0x8000`–`0xFFFE` | Reserved for future standardized profiles |
| `0xFFFF` | Wildcard, only where explicitly permitted |

The protocol defines how IDs are encoded. The application profile defines what each assigned ID means.

## 7. Typed value encoding

A resource value is encoded as:

| Relative offset | Size | Field |
|---:|---:|---|
| `0x00` | 1 | Value type |
| `0x01` | 2 | Value length |
| `0x03` | N | Value bytes |

Value length is an unsigned 16-bit little-endian integer.

### 7.1 Value type assignments

| Value | Name | Required value length | Encoding |
|---:|---|---:|---|
| `0x00` | `NONE` | 0 | No value bytes |
| `0x01` | `BOOL` | 1 | `0x00` false; `0x01` true |
| `0x02` | `UINT8` | 1 | Unsigned 8-bit integer |
| `0x03` | `UINT16` | 2 | Unsigned 16-bit little-endian integer |
| `0x04` | `INT16` | 2 | Two's-complement 16-bit little-endian integer |
| `0x05` | `UINT32` | 4 | Unsigned 32-bit little-endian integer |
| `0x06` | `INT32` | 4 | Two's-complement 32-bit little-endian integer |
| `0x07` | `FLOAT32` | 4 | IEEE 754 binary32, little-endian byte order |
| `0x08` | `UTF8` | variable | UTF-8 bytes without a terminating NUL |
| `0x09` | `BYTES` | variable | Uninterpreted bytes |

Validation rules:

- `BOOL` values other than `0x00` and `0x01` are invalid.
- Fixed-width types MUST have exactly the required value length.
- `NONE` MUST have a value length of zero.
- UTF-8 data MUST be valid UTF-8.
- Unknown value types MUST cause the containing message to be rejected.

## 8. Message type assignments

| Value | Name | Payload |
|---:|---|---|
| `0x01` | `HELLO` | Hello payload |
| `0x02` | `ACK` | Acknowledgement payload |
| `0x03` | `NACK` | Acknowledgement payload |
| `0x04` | `ERROR` | Error payload |
| `0x10` | `RESOURCE_QUERY` | Resource query payload |
| `0x11` | `RESOURCE_STATE` | Resource value payload |
| `0x12` | `RESOURCE_COMMAND` | Resource value payload |
| `0x00`, `0x05`–`0x0F`, `0x13`–`0x7F` | Reserved | None assigned |
| `0x80`–`0xFE` | Experimental or application-private | Application-defined |
| `0xFF` | Reserved | None assigned |

Application-private message types MUST NOT be required for baseline protocol interoperability.

## 9. Payload formats

### 9.1 HELLO payload

The initial HELLO payload identifies the sender's supported protocol range.

| Relative offset | Size | Field |
|---:|---:|---|
| `0x00` | 1 | Minimum supported protocol version |
| `0x01` | 1 | Maximum supported protocol version |

Payload length MUST be 2.

For an implementation supporting only protocol version 1:

```text
01 01
```

HELLO does not define application capabilities or resource metadata. Those may be added later through separate messages.

### 9.2 ACK payload

| Relative offset | Size | Field |
|---:|---:|---|
| `0x00` | 2 | Acknowledged sequence number |
| `0x02` | 1 | Acknowledged message type |
| `0x03` | 1 | Result code |

Payload length MUST be 4.

ACK result codes:

| Value | Name | Meaning |
|---:|---|---|
| `0x00` | `OK` | Message processed successfully |
| `0x01` | `DUPLICATE` | Previously processed; no state-changing action repeated |
| `0x02`–`0xFF` | Reserved | Not assigned in protocol version 1 |

An ACK confirms processing status. It does not replace an authoritative `RESOURCE_STATE` update.

### 9.3 NACK payload

NACK uses the same four-byte layout as ACK.

NACK result codes:

| Value | Name | Meaning |
|---:|---|---|
| `0x01` | `MALFORMED_PAYLOAD` | Payload layout or length is invalid |
| `0x02` | `UNSUPPORTED_MESSAGE` | Message type is not supported |
| `0x03` | `UNKNOWN_RESOURCE` | Resource ID is not recognized |
| `0x04` | `UNKNOWN_PROPERTY` | Property ID is not recognized for the resource |
| `0x05` | `TYPE_MISMATCH` | Value type is invalid for the property |
| `0x06` | `VALUE_OUT_OF_RANGE` | Value is outside the accepted range |
| `0x07` | `NOT_WRITABLE` | Property cannot be changed |
| `0x08` | `BUSY` | Receiver cannot process the request now |
| `0x09` | `INTERNAL_ERROR` | Receiver encountered an internal failure |
| `0x0A`–`0xFF` | Reserved | Not assigned in protocol version 1 |

### 9.4 ERROR payload

ERROR reports a protocol- or application-level problem that is not necessarily tied to an acknowledged request.

| Relative offset | Size | Field |
|---:|---:|---|
| `0x00` | 2 | Error code |
| `0x02` | 2 | Detail length |
| `0x04` | N | UTF-8 detail text |

Payload length MUST equal `4 + detail_length`.

Error detail text MAY be empty. Embedded devices MAY omit descriptive text to conserve bandwidth.

### 9.5 RESOURCE_QUERY payload

| Relative offset | Size | Field |
|---:|---:|---|
| `0x00` | 2 | Resource ID |
| `0x02` | 2 | Property ID |

Payload length MUST be 4.

Wildcards are permitted as follows:

| Resource ID | Property ID | Meaning |
|---:|---:|---|
| specific | specific | Query one property |
| specific | `0xFFFF` | Query all readable properties of one resource |
| `0xFFFF` | `0xFFFF` | Query all readable properties visible to the receiver |

`resource_id = 0xFFFF` with a specific property ID is reserved and MUST NOT be sent in protocol version 1.

A receiver answers a query with one or more `RESOURCE_STATE` messages. Ordering of multiple state messages is not guaranteed.

### 9.6 RESOURCE_STATE payload

| Relative offset | Size | Field |
|---:|---:|---|
| `0x00` | 2 | Resource ID |
| `0x02` | 2 | Property ID |
| `0x04` | 1 | Value type |
| `0x05` | 2 | Value length |
| `0x07` | N | Value bytes |

Payload length MUST equal `7 + value_length`.

Wildcard resource and property IDs are not valid in a state message.

A state message contains an authoritative value as understood by its sender. It may be transmitted:

- in response to `RESOURCE_QUERY`;
- after successful processing of `RESOURCE_COMMAND`; or
- unsolicited when state changes.

### 9.7 RESOURCE_COMMAND payload

RESOURCE_COMMAND uses the same layout as RESOURCE_STATE.

Payload length MUST equal `7 + value_length`.

Wildcard resource and property IDs are not valid in a command message.

A command requests that the receiver change one property. A receiver MUST validate:

- the resource ID;
- the property ID;
- whether the property is writable;
- the value type;
- the value length; and
- the value range.

A successful command SHOULD be followed by an ACK and an authoritative `RESOURCE_STATE` message. The state message is authoritative if its value differs from the requested value.

## 10. Packet validation order

A receiver SHOULD validate a packet in this order:

1. Minimum packet length.
2. Magic value.
3. Supported protocol version.
4. Declared payload length versus received packet length.
5. CRC.
6. Destination device ID.
7. Message type.
8. Message-specific payload structure.
9. Resource, property, type, and value constraints.
10. Duplicate detection and application processing.

Packets failing checks 1 through 5 MUST be discarded without application processing.

A receiver SHOULD avoid replying to malformed broadcast packets to prevent reply storms.

## 11. Size limits

The wire format permits payloads up to 65,535 bytes, but a transport or implementation may impose a smaller limit.

Every implementation MUST publish its maximum accepted packet size.

A sender MUST NOT transmit a packet larger than the negotiated or documented transport limit. Protocol version 1 does not define fragmentation or reassembly.

## 12. Example: resource query

The following example assumes:

```text
sequence number       = 0x002A
source device ID      = 0x0101
destination device ID = 0x0201
resource ID           = 0x0100
property ID           = 0x0002
```

Packet before CRC:

```text
Offset  Bytes        Meaning
------  -----------  --------------------------------
00      48 52        Magic "HR"
02      01           Protocol version 1
03      10           RESOURCE_QUERY
04      2A 00        Sequence 0x002A
06      01 01        Source device 0x0101
08      01 02        Destination device 0x0201
0A      04 00        Payload length 4
0C      00           Flags
0D      00 01        Resource 0x0100
0F      02 00        Property 0x0002
11      ?? ??        CRC-16/CCITT-FALSE, little-endian
```

## 13. Example: Boolean command

The following payload requests that resource `0x0100`, property `0x0001`, be set to true:

```text
00 01        Resource ID 0x0100
01 00        Property ID 0x0001
01           BOOL
01 00        Value length 1
01           true
```

The resource-command payload length is 8 bytes.

## 14. Compatibility rules

- A protocol version change is required when the fixed header layout changes incompatibly.
- Reserved fields and values MUST remain unused until formally assigned.
- New message types may be added without changing the protocol version when older receivers can safely ignore them.
- New value types may be added only when unknown types are safely rejected.
- Existing message and value assignments MUST NOT be redefined.
- Application resource and property mappings are versioned by the application profile, not by this wire-format document.

## 15. Open decisions before v0.1.0 release

The following items should be resolved before this draft is marked stable:

- Maximum packet size for each supported transport.
- Timeout and retry policy for acknowledged messages.
- Duplicate-cache duration and capacity guidance.
- Whether protocol version negotiation needs more than HELLO's minimum/maximum range.
- Whether ERROR remains in the baseline catalog or is deferred.

