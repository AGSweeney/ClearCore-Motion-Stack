#pragma once

#include <QtCore/QByteArray>
#include <cstdint>

namespace motion_bench::protocol {

struct CipPath {
    std::uint16_t class_id = 0;
    std::uint16_t instance_id = 0;
    std::uint16_t attribute_id = 0;
};

QByteArray EncodePath(const CipPath &path);

}  // namespace motion_bench::protocol
