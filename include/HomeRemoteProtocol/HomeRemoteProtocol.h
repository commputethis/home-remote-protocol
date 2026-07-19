#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace HomeRemoteProtocol {

inline constexpr std::uint8_t ProtocolVersion = 1;
inline constexpr std::uint16_t Magic = 0x4852;  // "HR"

enum class MessageType : std::uint8_t {
    Hello = 0x01,
    StateRequest = 0x02,
    State = 0x03,
    Command = 0x04,
    Battery = 0x05,
    Ack = 0x06,
    Heartbeat = 0x07,
};

enum class Command : std::uint8_t {
    LightToggle = 0x10,
    LightBrightness = 0x11,
    FanToggle = 0x20,
    FanSpeed = 0x21,
};

enum class AckStatus : std::uint8_t {
    Accepted = 0,
    Duplicate = 1,
    Invalid = 2,
    Unsupported = 3,
    Failed = 4,
};

#pragma pack(push, 1)
struct PacketHeader {
    std::uint16_t magic{Magic};
    std::uint8_t version{ProtocolVersion};
    MessageType type{MessageType::Heartbeat};
    std::uint16_t sequence{0};
    std::uint16_t device_id{0};
    std::uint8_t payload_size{0};
};

struct HelloPayload {
    std::uint32_t firmware_version{0};
    std::uint32_t capabilities{0};
    char device_name[32]{};
};

struct AckPayload {
    std::uint16_t acknowledged_sequence{0};
    AckStatus status{AckStatus::Accepted};
};

enum class ValueType : uint8_t;

struct ResourceReference {
    uint16_t resource_id;
    uint16_t property_id;
};

struct ResourceValue {
    ResourceReference target;
    ValueType type;
    uint16_t length;
    // Encoded value follows on the wire.
};
#pragma pack(pop)

template <typename Payload>
struct Packet {
    static_assert(std::is_trivially_copyable<Payload>::value,
                  "ESP-NOW payloads must be trivially copyable");
    PacketHeader header{};
    Payload payload{};
    std::uint16_t checksum{0};
};

inline std::uint16_t crc16(const std::uint8_t* data, std::size_t length) {
    std::uint16_t crc = 0xFFFF;
    for (std::size_t i = 0; i < length; ++i) {
        crc ^= static_cast<std::uint16_t>(data[i]) << 8;
        for (std::uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000) ? static_cast<std::uint16_t>((crc << 1) ^ 0x1021)
                                 : static_cast<std::uint16_t>(crc << 1);
        }
    }
    return crc;
}

template <typename Payload>
Packet<Payload> makePacket(MessageType type,
                           std::uint16_t sequence,
                           std::uint16_t device_id,
                           const Payload& payload) {
    Packet<Payload> packet{};
    packet.header.type = type;
    packet.header.sequence = sequence;
    packet.header.device_id = device_id;
    packet.header.payload_size = static_cast<std::uint8_t>(sizeof(Payload));
    packet.payload = payload;
    packet.checksum = crc16(reinterpret_cast<const std::uint8_t*>(&packet),
                            sizeof(packet) - sizeof(packet.checksum));
    return packet;
}

template <typename Payload>
bool validate(const Packet<Payload>& packet, MessageType expected_type) {
    if (packet.header.magic != Magic ||
        packet.header.version != ProtocolVersion ||
        packet.header.type != expected_type ||
        packet.header.payload_size != sizeof(Payload)) {
        return false;
    }
    const auto expected = crc16(reinterpret_cast<const std::uint8_t*>(&packet),
                                sizeof(packet) - sizeof(packet.checksum));
    return expected == packet.checksum;
}

}  // namespace HomeRemoteProtocol
