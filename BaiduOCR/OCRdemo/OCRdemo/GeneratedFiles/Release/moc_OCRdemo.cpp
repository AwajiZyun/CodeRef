/****************************************************************************
** Meta object code from reading C++ file 'OCRdemo.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../OCRdemo.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'OCRdemo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OCRdemo_t {
    QByteArrayData data[21];
    char stringdata0[229];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OCRdemo_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OCRdemo_t qt_meta_stringdata_OCRdemo = {
    {
QT_MOC_LITERAL(0, 0, 7), // "OCRdemo"
QT_MOC_LITERAL(1, 8, 12), // "onBtnLoadImg"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 11), // "onBtnIDCard"
QT_MOC_LITERAL(4, 34, 15), // "onBtnIDCardBack"
QT_MOC_LITERAL(5, 50, 13), // "onBtnBankCard"
QT_MOC_LITERAL(6, 64, 11), // "onBtnHKCard"
QT_MOC_LITERAL(7, 76, 11), // "onBtnTWCard"
QT_MOC_LITERAL(8, 88, 14), // "onBtnHousehold"
QT_MOC_LITERAL(9, 103, 13), // "onBtnPassport"
QT_MOC_LITERAL(10, 117, 14), // "onBtnDriverLic"
QT_MOC_LITERAL(11, 132, 15), // "onBtnDriverLic2"
QT_MOC_LITERAL(12, 148, 8), // "onBtnCam"
QT_MOC_LITERAL(13, 157, 15), // "onImageCaptured"
QT_MOC_LITERAL(14, 173, 2), // "id"
QT_MOC_LITERAL(15, 176, 3), // "img"
QT_MOC_LITERAL(16, 180, 12), // "onImageReady"
QT_MOC_LITERAL(17, 193, 5), // "state"
QT_MOC_LITERAL(18, 199, 13), // "onSigResponse"
QT_MOC_LITERAL(19, 213, 7), // "resCode"
QT_MOC_LITERAL(20, 221, 7) // "resData"

    },
    "OCRdemo\0onBtnLoadImg\0\0onBtnIDCard\0"
    "onBtnIDCardBack\0onBtnBankCard\0onBtnHKCard\0"
    "onBtnTWCard\0onBtnHousehold\0onBtnPassport\0"
    "onBtnDriverLic\0onBtnDriverLic2\0onBtnCam\0"
    "onImageCaptured\0id\0img\0onImageReady\0"
    "state\0onSigResponse\0resCode\0resData"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OCRdemo[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x0a /* Public */,
       3,    0,   85,    2, 0x0a /* Public */,
       4,    0,   86,    2, 0x0a /* Public */,
       5,    0,   87,    2, 0x0a /* Public */,
       6,    0,   88,    2, 0x0a /* Public */,
       7,    0,   89,    2, 0x0a /* Public */,
       8,    0,   90,    2, 0x0a /* Public */,
       9,    0,   91,    2, 0x0a /* Public */,
      10,    0,   92,    2, 0x0a /* Public */,
      11,    0,   93,    2, 0x0a /* Public */,
      12,    0,   94,    2, 0x0a /* Public */,
      13,    2,   95,    2, 0x0a /* Public */,
      16,    1,  100,    2, 0x0a /* Public */,
      18,    2,  103,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QImage,   14,   15,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void, QMetaType::Int, QMetaType::QByteArray,   19,   20,

       0        // eod
};

void OCRdemo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<OCRdemo *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onBtnLoadImg(); break;
        case 1: _t->onBtnIDCard(); break;
        case 2: _t->onBtnIDCardBack(); break;
        case 3: _t->onBtnBankCard(); break;
        case 4: _t->onBtnHKCard(); break;
        case 5: _t->onBtnTWCard(); break;
        case 6: _t->onBtnHousehold(); break;
        case 7: _t->onBtnPassport(); break;
        case 8: _t->onBtnDriverLic(); break;
        case 9: _t->onBtnDriverLic2(); break;
        case 10: _t->onBtnCam(); break;
        case 11: _t->onImageCaptured((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QImage(*)>(_a[2]))); break;
        case 12: _t->onImageReady((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->onSigResponse((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QByteArray(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject OCRdemo::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_OCRdemo.data,
    qt_meta_data_OCRdemo,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *OCRdemo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OCRdemo::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OCRdemo.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int OCRdemo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
