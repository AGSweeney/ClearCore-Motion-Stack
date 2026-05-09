#include "ui/viewmodels/AppViewModel.h"

namespace motion_bench::ui {

AppViewModel::AppViewModel(QObject *parent) : QObject(parent), device_service_(this) {}

device::DeviceService *AppViewModel::deviceService() { return &device_service_; }

QVariantList AppViewModel::discoveryResults() const { return discovery_results_; }
QStringList AppViewModel::discoveryIps() const {
    QStringList ips;
    ips.reserve(discovery_results_.size());
    for (const QVariant &row_var : discovery_results_) {
        const QVariantMap row = row_var.toMap();
        const QString ip = row.value("ipAddress").toString();
        if (!ip.isEmpty()) {
            ips.push_back(ip);
        }
    }
    return ips;
}

void AppViewModel::discover() {
    discovery_results_ = device_service_.discover();
    emit discoveryResultsChanged();
}

}  // namespace motion_bench::ui
