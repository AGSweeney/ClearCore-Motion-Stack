/****************************************************************************
** Meta object code from reading C++ file 'DeviceIoWorker.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/device/DeviceIoWorker.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DeviceIoWorker.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12motion_bench6device14DeviceIoWorkerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12motion_bench6device14DeviceIoWorkerE = QtMocHelpers::stringData(
    "motion_bench::device::DeviceIoWorker",
    "connectionChanged",
    "",
    "connected",
    "pollingChanged",
    "polling_enabled",
    "errorChanged",
    "error",
    "supplyVoltageUpdated",
    "value",
    "monitorDataUpdated",
    "QVariantMap",
    "monitor_data",
    "setPollIntervalMs",
    "interval_ms",
    "setSelectedMotorInstance",
    "instance_id",
    "setOperatorControlsEnabled",
    "enabled",
    "discover",
    "QVariantList",
    "timeout_ms",
    "connectDevice",
    "target_ip",
    "disconnectDevice",
    "startPolling",
    "stopPolling",
    "refreshOnce",
    "writeMotorVelocity",
    "writeMoveDistance",
    "writeMoveVelocity",
    "writeAccel",
    "writeDecel",
    "writeControlRegister",
    "writeControlBit",
    "bit_index",
    "commandPositionalMove",
    "move_distance",
    "commandStopPositionalMove",
    "writeDigitalOutput",
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
    "QVariant"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12motion_bench6device14DeviceIoWorkerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      32,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  206,    2, 0x06,    1 /* Public */,
       4,    1,  209,    2, 0x06,    3 /* Public */,
       6,    1,  212,    2, 0x06,    5 /* Public */,
       8,    1,  215,    2, 0x06,    7 /* Public */,
      10,    1,  218,    2, 0x06,    9 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      13,    1,  221,    2, 0x0a,   11 /* Public */,
      15,    1,  224,    2, 0x0a,   13 /* Public */,
      17,    1,  227,    2, 0x0a,   15 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      19,    1,  230,    2, 0x02,   17 /* Public */,
      19,    0,  233,    2, 0x22,   19 /* Public | MethodCloned */,
      22,    1,  234,    2, 0x02,   20 /* Public */,
      24,    0,  237,    2, 0x02,   22 /* Public */,
      25,    0,  238,    2, 0x02,   23 /* Public */,
      26,    0,  239,    2, 0x02,   24 /* Public */,
      27,    0,  240,    2, 0x02,   25 /* Public */,
      28,    1,  241,    2, 0x02,   26 /* Public */,
      29,    1,  244,    2, 0x02,   28 /* Public */,
      30,    1,  247,    2, 0x02,   30 /* Public */,
      31,    1,  250,    2, 0x02,   32 /* Public */,
      32,    1,  253,    2, 0x02,   34 /* Public */,
      33,    1,  256,    2, 0x02,   36 /* Public */,
      34,    2,  259,    2, 0x02,   38 /* Public */,
      36,    1,  264,    2, 0x02,   41 /* Public */,
      38,    0,  267,    2, 0x02,   43 /* Public */,
      39,    2,  268,    2, 0x02,   44 /* Public */,
      41,    1,  273,    2, 0x02,   47 /* Public */,
      42,    3,  276,    2, 0x02,   49 /* Public */,
      45,    2,  283,    2, 0x02,   53 /* Public */,
      46,    1,  288,    2, 0x02,   56 /* Public */,
      47,    1,  291,    2, 0x02,   58 /* Public */,
      48,    1,  294,    2, 0x02,   60 /* Public */,
      49,    2,  297,    2, 0x02,   62 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::Double,    9,
    QMetaType::Void, 0x80000000 | 11,   12,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void, QMetaType::Bool,   18,

 // methods: parameters
    0x80000000 | 20, QMetaType::Int,   21,
    0x80000000 | 20,
    QMetaType::Bool, QMetaType::QString,   23,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Bool, QMetaType::Int,    9,
    QMetaType::Bool, QMetaType::Int,    9,
    QMetaType::Bool, QMetaType::Int,    9,
    QMetaType::Bool, QMetaType::Int,    9,
    QMetaType::Bool, QMetaType::Int,    9,
    QMetaType::Bool, QMetaType::UInt,    9,
    QMetaType::Bool, QMetaType::Int, QMetaType::Bool,   35,   18,
    QMetaType::Bool, QMetaType::Int,   37,
    QMetaType::Bool,
    QMetaType::Bool, QMetaType::Int, QMetaType::Bool,   16,   40,
    QMetaType::Bool, QMetaType::Bool,   18,
    QMetaType::Bool, QMetaType::Int, QMetaType::Int, QMetaType::Bool,   43,   44,   18,
    QMetaType::Bool, QMetaType::Int, QMetaType::Bool,   35,   18,
    QMetaType::Bool, QMetaType::Int,    9,
    QMetaType::Bool, QMetaType::Int,    9,
    QMetaType::Bool, QMetaType::Int,    9,
    QMetaType::Bool, QMetaType::QString, 0x80000000 | 51,   50,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject motion_bench::device::DeviceIoWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12motion_bench6device14DeviceIoWorkerE.offsetsAndSizes,
    qt_meta_data_ZN12motion_bench6device14DeviceIoWorkerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12motion_bench6device14DeviceIoWorkerE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DeviceIoWorker, std::true_type>,
        // method 'connectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'pollingChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'errorChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'supplyVoltageUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'monitorDataUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVariantMap &, std::false_type>,
        // method 'setPollIntervalMs'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setSelectedMotorInstance'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setOperatorControlsEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'discover'
        QtPrivate::TypeAndForceComplete<QVariantList, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'discover'
        QtPrivate::TypeAndForceComplete<QVariantList, std::false_type>,
        // method 'connectDevice'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
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

void motion_bench::device::DeviceIoWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DeviceIoWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectionChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->pollingChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->errorChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->supplyVoltageUpdated((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 4: _t->monitorDataUpdated((*reinterpret_cast< std::add_pointer_t<QVariantMap>>(_a[1]))); break;
        case 5: _t->setPollIntervalMs((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->setSelectedMotorInstance((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->setOperatorControlsEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: { QVariantList _r = _t->discover((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 9: { QVariantList _r = _t->discover();
            if (_a[0]) *reinterpret_cast< QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 10: { bool _r = _t->connectDevice((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 11: _t->disconnectDevice(); break;
        case 12: _t->startPolling(); break;
        case 13: _t->stopPolling(); break;
        case 14: { bool _r = _t->refreshOnce();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 15: { bool _r = _t->writeMotorVelocity((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 16: { bool _r = _t->writeMoveDistance((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 17: { bool _r = _t->writeMoveVelocity((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 18: { bool _r = _t->writeAccel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 19: { bool _r = _t->writeDecel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 20: { bool _r = _t->writeControlRegister((*reinterpret_cast< std::add_pointer_t<quint32>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 21: { bool _r = _t->writeControlBit((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 22: { bool _r = _t->commandPositionalMove((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 23: { bool _r = _t->commandStopPositionalMove();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 24: { bool _r = _t->writeDigitalOutput((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 25: { bool _r = _t->writeCcioEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 26: { bool _r = _t->writeCcioOutputBit((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 27: { bool _r = _t->writeMConnectorOutputBit((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 28: { bool _r = _t->writeMConnectorTriggerPulses((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 29: { bool _r = _t->writeMConnectorPwmA((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 30: { bool _r = _t->writeMConnectorPwmB((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 31: { bool _r = _t->writeMotorConfigField((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVariant>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (DeviceIoWorker::*)(bool );
            if (_q_method_type _q_method = &DeviceIoWorker::connectionChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceIoWorker::*)(bool );
            if (_q_method_type _q_method = &DeviceIoWorker::pollingChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceIoWorker::*)(const QString & );
            if (_q_method_type _q_method = &DeviceIoWorker::errorChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceIoWorker::*)(double );
            if (_q_method_type _q_method = &DeviceIoWorker::supplyVoltageUpdated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (DeviceIoWorker::*)(const QVariantMap & );
            if (_q_method_type _q_method = &DeviceIoWorker::monitorDataUpdated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *motion_bench::device::DeviceIoWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *motion_bench::device::DeviceIoWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12motion_bench6device14DeviceIoWorkerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int motion_bench::device::DeviceIoWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 32)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 32;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 32)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 32;
    }
    return _id;
}

// SIGNAL 0
void motion_bench::device::DeviceIoWorker::connectionChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void motion_bench::device::DeviceIoWorker::pollingChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void motion_bench::device::DeviceIoWorker::errorChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void motion_bench::device::DeviceIoWorker::supplyVoltageUpdated(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void motion_bench::device::DeviceIoWorker::monitorDataUpdated(const QVariantMap & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
