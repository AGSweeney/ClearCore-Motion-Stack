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
