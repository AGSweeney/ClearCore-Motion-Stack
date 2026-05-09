#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QtMath>

#include "device/ClearLinkObjectMap.h"
#include "protocol/CipPath.h"

namespace {

bool Expect(bool condition, const QString &message) {
    if (!condition) {
        qCritical() << "FAIL:" << message;
        return false;
    }
    return true;
}

}  // namespace

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    bool ok = true;

    const motion_bench::protocol::CipPath short_path{0x65, 1, 7};
    const QByteArray encoded = motion_bench::protocol::EncodePath(short_path);
    ok &= Expect(encoded.toHex() == "206524013007", "CIP path encoding mismatch for 8-bit segments");

    const motion_bench::protocol::CipPath wide_path{0x0123, 0x0102, 0x0203};
    const QByteArray wide_encoded = motion_bench::protocol::EncodePath(wide_path);
    ok &= Expect(
        wide_encoded.toHex() == "210023012500020131000302",
        "CIP path encoding mismatch for 16-bit segments");

    const QVariant raw_float = 12.5f;
    const QByteArray float_bytes = motion_bench::device::ClearLinkObjectMap::EncodeValue(
        raw_float, motion_bench::device::DataType::kFloat32);
    const QVariant decoded_float = motion_bench::device::ClearLinkObjectMap::DecodeValue(
        float_bytes, motion_bench::device::DataType::kFloat32);
    ok &= Expect(qAbs(decoded_float.toFloat() - 12.5f) < 0.0001f, "Float encode/decode mismatch");

    const auto attributes = motion_bench::device::ClearLinkObjectMap::Attributes();
    ok &= Expect(attributes.contains("motor.control_reg"), "Missing motor.control_reg attribute");
    ok &= Expect(attributes.contains("ccio.io_status_bits"), "Missing ccio.io_status_bits attribute");

    if (!ok) {
        return 1;
    }
    qInfo() << "Protocol smoke test passed.";
    return 0;
}
