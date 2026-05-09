#include "protocol/CipPath.h"

namespace motion_bench::protocol {

namespace {

void AppendLogical8(QByteArray *bytes, std::uint8_t segment_type, std::uint8_t value) {
    bytes->append(static_cast<char>(segment_type));
    bytes->append(static_cast<char>(value));
}

void AppendLogical16(QByteArray *bytes, std::uint8_t segment_type, std::uint16_t value) {
    bytes->append(static_cast<char>(segment_type | 0x01u));
    bytes->append(static_cast<char>(0x00u));
    bytes->append(static_cast<char>(value & 0xFFu));
    bytes->append(static_cast<char>((value >> 8) & 0xFFu));
}

void AppendLogical(QByteArray *bytes, std::uint8_t segment_type, std::uint16_t value) {
    if (value <= 0xFFu) {
        AppendLogical8(bytes, segment_type, static_cast<std::uint8_t>(value));
        return;
    }
    AppendLogical16(bytes, segment_type, value);
}

}  // namespace

QByteArray EncodePath(const CipPath &path) {
    QByteArray encoded;
    AppendLogical(&encoded, 0x20u, path.class_id);
    AppendLogical(&encoded, 0x24u, path.instance_id);
    AppendLogical(&encoded, 0x30u, path.attribute_id);

    if ((encoded.size() % 2) != 0) {
        encoded.append(static_cast<char>(0x00u));
    }
    return encoded;
}

}  // namespace motion_bench::protocol
