# Home Remote Protocol Message Catalog

Version 0.1.0

This document defines every protocol message.

---

## HELLO

Purpose

Announce device presence.

Payload

HelloPayload

Requires ACK

No

---

## DISCOVERY

Purpose

Locate compatible devices.

Payload

None

Requires ACK

No

---

## DISCOVERY_RESPONSE

Purpose

Respond to discovery request.

Payload

DeviceInfoPayload

Requires ACK

No

---

## STATE_REQUEST

Purpose

Request current device state.

Payload

None

Requires ACK

No

Response

STATE

---

## STATE

Purpose

Publish current state.

Payload

StatePayload

Requires ACK

No

May be sent unsolicited.

---

## COMMAND

Purpose

Request a state change.

Payload

CommandPayload

Requires ACK

Yes

---

## ACK

Purpose

Confirm successful processing.

Payload

AckPayload

Requires ACK

No

---

## NACK

Purpose

Report failure.

Payload

AckPayload

Requires ACK

No
