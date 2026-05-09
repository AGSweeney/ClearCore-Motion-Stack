/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: CipPath.h
 * Purpose: CIP logical path (class/instance/attribute) and EPATH encoding helper.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

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
