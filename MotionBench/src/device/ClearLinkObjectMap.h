/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: ClearLinkObjectMap.h
 * Purpose: ClearLink-oriented CIP object/attribute map and typed encode/decode helpers.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <cstdint>

#include "protocol/CipPath.h"

namespace motion_bench::device {

enum class DataType {
    kBool,
    kSint,
    kUint16,
    kInt16,
    kUint32,
    kInt32,
    kFloat32,
    kByteArray
};

struct ObjectAttribute {
    QString key;
    QString description;
    protocol::CipPath path;
    DataType data_type = DataType::kByteArray;
    bool writable = false;
};

class ClearLinkObjectMap {
public:
    static const QHash<QString, ObjectAttribute> &Attributes();
    static QVariant DecodeValue(const QByteArray &raw, DataType data_type);
    static QByteArray EncodeValue(const QVariant &value, DataType data_type);
};

}  // namespace motion_bench::device
