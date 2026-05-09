/****************************************************************************
** Meta object code from reading C++ file 'DeviceService.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/device/DeviceService.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DeviceService.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN12motion_bench6device13DeviceServiceE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12motion_bench6device13DeviceServiceE = QtMocHelpers::stringData(
    "motion_bench::device::DeviceService",
    "connectionChanged",
    "",
    "targetIpChanged",
    "errorChanged",
    "pollIntervalChanged",
    "selectedMotorInstanceChanged",
    "pollingChanged",
    "operatorControlsEnabledChanged",
    "supplyVoltageChanged",
    "monitorDataChanged",
    "discover",
    "QVariantList",
    "timeout_ms",
    "connectDevice",
    "disconnectDevice",
    "startPolling",
    "stopPolling",
    "refreshOnce",
    "writeMotorVelocity",
    "value",
    "writeMoveDistance",
    "writeMoveVelocity",
    "writeAccel",
    "writeDecel",
    "writeControlRegister",
    "writeControlBit",
    "bit_index",
    "enabled",
    "commandPositionalMove",
    "move_distance",
    "commandStopPositionalMove",
    "writeDigitalOutput",
    "instance_id",
    "state",
    "writeCcioEnabled",
    "writeCcioOutputBit",
    "board_index",
    "output_bit",
    "writeMConnectorOutputBit",
    "writeMConnectorTriggerPulses",
    "writeMConnectorPwmA",
    "writeMConnectorPwmB",
    "writeMotorConfigField",
    "field_key",
    "QVariant",
    "connected",
    "targetIp",
    "lastError",
    "pollIntervalMs",
    "selectedMotorInstance",
    "pollingEnabled",
    "operatorControlsEnabled",
    "supplyVoltage",
    "monitorData",
    "QVariantMap"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12motion_bench6device13DeviceServiceE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      33,   14, // methods
       9,  291, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  212,    2, 0x06,   10 /* Public */,
       3,    0,  213,    2, 0x06,   11 /* Public */,
       4,    0,  214,    2, 0x06,   12 /* Public */,
       5,    0,  215,    2, 0x06,   13 /* Public */,
       6,    0,  216,    2, 0x06,   14 /* Public */,
       7,    0,  217,    2, 0x06,   15 /* Public */,
       8,    0,  218,    2, 0x06,   16 /* Public */,
       9,    0,  219,    2, 0x06,   17 /* Public */,
      10,    0,  220,    2, 0x06,   18 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      11,    1,  221,    2, 0x02,   19 /* Public */,
      11,    0,  224,    2, 0x22,   21 /* Public | MethodCloned */,
      14,    0,  225,    2, 0x02,   22 /* Public */,
      15,    0,  226,    2, 0x02,   23 /* Public */,
      16,    0,  227,    2, 0x02,   24 /* Public */,
      17,    0,  228,    2, 0x02,   25 /* Public */,
      18,    0,  229,    2, 0x02,   26 /* Public */,
      19,    1,  230,    2, 0x02,   27 /* Public */,
      21,    1,  233,    2, 0x02,   29 /* Public */,
      22,    1,  236,    2, 0x02,   31 /* Public */,
      23,    1,  239,    2, 0x02,   33 /* Public */,
      24,    1,  242,    2, 0x02,   35 /* Public */,
      25,    1,  245,    2, 0x02,   37 /* Public */,
      26,    2,  248,    2, 0x02,   39 /* Public */,
      29,    1,  253,    2, 0x02,   42 /* Public */,
      31,    0,  256,    2, 0x02,   44 /* Public */,
      32,    2,  257,    2, 0x02,   45 /* Public */,
      35,    1,  262,    2, 0x02,   48 /* Public */,
      36,    3,  265,    2, 0x02,   50 /* Public */,
      39,    2,  272,    2, 0x02,   54 /* Public */,
      40,    1,  277,    2, 0x02,   57 /* Public */,
      41,    1,  280,    2, 0x02,   59 /* Public */,
      42,    1,  283,    2, 0x02,   61 /* Public */,
      43,    2,  286,    2, 0x02,   63 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    0x80000000 | 12, QMetaType::Int,   13,
    0x80000000 | 12,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Bool, QMetaType::Int,   20,
    QMetaType::Bool, QMetaType::Int,   20,
    QMetaType::Bool, QMetaType::Int,   20,
    QMetaType::Bool, QMetaType::Int,   20,
    QMetaType::Bool, QMetaType::Int,   20,
    QMetaType::Bool, QMetaType::UInt,   20,
    QMetaType::Bool, QMetaType::Int, QMetaType::Bool,   27,   28,
    QMetaType::Bool, QMetaType::Int,   30,
    QMetaType::Bool,
    QMetaType::Bool, QMetaType::Int, QMetaType::Bool,   33,   34,
    QMetaType::Bool, QMetaType::Bool,   28,
    QMetaType::Bool, QMetaType::Int, QMetaType::Int, QMetaType::Bool,   37,   38,   28,
    QMetaType::Bool, QMetaType::Int, QMetaType::Bool,   27,   28,
    QMetaType::Bool, QMetaType::Int,   20,
    QMetaType::Bool, QMetaType::Int,   20,
    QMetaType::Bool, QMetaType::Int,   20,
    QMetaType::Bool, QMetaType::QString, 0x80000000 | 45,   44,   20,

 // properties: name, type, flags, notifyId, revision
      46, QMetaType::Bool, 0x00015001, uint(0), 0,
      47, QMetaType::QString, 0x00015103, uint(1), 0,
      48, QMetaType::QString, 0x00015001, uint(2), 0,
      49, QMetaType::Int, 0x00015103, uint(3), 0,
      50, QMetaType::Int, 0x00015103, uint(4), 0,
      51, QMetaType::Bool, 0x00015001, uint(5), 0,
      52, QMetaType::Bool, 0x00015103, uint(6), 0,
      53, QMetaType::Double, 0x00015001, uint(7), 0,
      54, 0x80000000 | 55, 0x00015009, uint(8), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject motion_bench::device::DeviceService::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12motion_bench6device13DeviceServiceE.offsetsAndSizes,
    qt_meta_data_ZN12motion_bench6device13DeviceServiceE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12motion_bench6device13DeviceServiceE_t,
        // property 'connected'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'targetIp'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'lastError'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'pollIntervalMs'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'selectedMotorInstance'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'pollingEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'operatorControlsEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'supplyVoltage'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'monitorData'
        QtPrivate::TypeAndForceComplete<QVariantMap, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DeviceService, std::true_type>,
        // method 'connectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'targetIpChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'errorChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'pollIntervalChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'selectedMotorInstanceChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'pollingChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'operatorControlsEnabledChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'supplyVoltageChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'monitorDataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'discover'
        QtPrivate::TypeAndForceComplete<QVariantList, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'discover'
        QtPrivate::TypeAndForceComplete<QVariantList, std::false_type>,
        // method 'connectDevice'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'disconnectDevice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPolling'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stopPolling'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refreshOnce'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'writeMotorVelocity'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'writeMoveDistance'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'writeMoveVelocity'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'writeAccel'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'writeDecel'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'writeControlRegister'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint32, std::false_type>,
        // method 'writeControlBit'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'commandPositionalMove'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'commandStopPositionalMove'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'writeDigitalOutput'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'writeCcioEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'writeCcioOutputBit'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'writeMConnectorOutputBit'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'writeMConnectorTriggerPulses'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'writeMConnectorPwmA'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'writeMConnectorPwmB'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'writeMotorConfigField'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVariant &, std::false_type>
    >,
    nullptr
} };

void motion_bench::device::DeviceService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DeviceService *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectionChanged(); break;
        case 1: _t->targetIpChanged(); break;
        case 2: _t->errorChanged(); break;
        case 3: _t->pollIntervalChanged(); break;
        case 4: _t->selectedMotorInstanceChanged(); break;
        case 5: _t->pollingChanged(); break;
        case 6: _t->operatorControlsEnabledChanged(); break;
        case 7: _t->supplyVoltageChanged(); break;
        case 8: _t->monitorDataChanged(); break;
        case 9: { QVariantList _r = _t->discover((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 10: { QVariantList _r = _t->discover();
            if (_a[0]) *reinterpret_cast< QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 11: { bool _r = _t->connectDevice();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 12: _t->disconnectDevice(); break;
        case 13: _t->startPolling(); break;
        case 14: _t->stopPolling(); break;
        case 15: { bool _r = _t->refreshOnce();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 16: { bool _r = _t->writeMotorVelocity((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 17: { bool _r = _t->writeMoveDistance((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 18: { bool _r = _t->writeMoveVelocity((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 19: { bool _r = _t->writeAccel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 20: { bool _r = _t->writeDecel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 21: { bool _r = _t->writeControlRegister((*reinterpret_cast< std::add_pointer_t<quint32>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 22: { bool _r = _t->writeControlBit((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 23: { bool _r = _t->commandPositionalMove((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 24: { bool _r = _t->commandStopPositionalMove();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 25: { bool _r = _t->writeDigitalOutput((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 26: { bool _r = _t->writeCcioEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 27: { bool _r = _t->writeCcioOutputBit((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 28: { bool _r = _t->writeMConnectorOutputBit((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 29: { bool _r = _t->writeMConnectorTriggerPulses((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 30: { bool _r = _t->writeMConnectorPwmA((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 31: { bool _r = _t->writeMConnectorPwmB((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 32: { bool _r = _t->writeMotorConfigField((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVariant>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (DeviceService::*)();
            if (_q_method_type _q_method = &DeviceService::connectionChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceService::*)();
            if (_q_method_type _q_method = &DeviceService::targetIpChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceService::*)();
            if (_q_method_type _q_method = &DeviceService::errorChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceService::*)();
            if (_q_method_type _q_method = &DeviceService::pollIntervalChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceService::*)();
            if (_q_method_type _q_method = &DeviceService::selectedMotorInstanceChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceService::*)();
            if (_q_method_type _q_method = &DeviceService::pollingChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceService::*)();
            if (_q_method_type _q_method = &DeviceService::operatorControlsEnabledChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceService::*)();
            if (_q_method_type _q_method = &DeviceService::supplyVoltageChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceService::*)();
            if (_q_method_type _q_method = &DeviceService::monitorDataChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->connected(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->targetIp(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->lastError(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->pollIntervalMs(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->selectedMotorInstance(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->pollingEnabled(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->operatorControlsEnabled(); break;
        case 7: *reinterpret_cast< double*>(_v) = _t->supplyVoltage(); break;
        case 8: *reinterpret_cast< QVariantMap*>(_v) = _t->monitorData(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setTargetIp(*reinterpret_cast< QString*>(_v)); break;
        case 3: _t->setPollIntervalMs(*reinterpret_cast< int*>(_v)); break;
        case 4: _t->setSelectedMotorInstance(*reinterpret_cast< int*>(_v)); break;
        case 6: _t->setOperatorControlsEnabled(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *motion_bench::device::DeviceService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *motion_bench::device::DeviceService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12motion_bench6device13DeviceServiceE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int motion_bench::device::DeviceService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 33)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 33;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 33)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 33;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void motion_bench::device::DeviceService::connectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void motion_bench::device::DeviceService::targetIpChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void motion_bench::device::DeviceService::errorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void motion_bench::device::DeviceService::pollIntervalChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void motion_bench::device::DeviceService::selectedMotorInstanceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void motion_bench::device::DeviceService::pollingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void motion_bench::device::DeviceService::operatorControlsEnabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void motion_bench::device::DeviceService::supplyVoltageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void motion_bench::device::DeviceService::monitorDataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}
QT_WARNING_POP
