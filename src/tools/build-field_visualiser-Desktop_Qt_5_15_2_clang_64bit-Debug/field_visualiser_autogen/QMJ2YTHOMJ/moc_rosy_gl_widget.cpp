/****************************************************************************
** Meta object code from reading C++ file 'rosy_gl_widget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../field_visualiser/rosy/rosy_gl_widget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rosy_gl_widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_rosy_gl_widget_t {
    QByteArrayData data[1];
    char stringdata0[15];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_rosy_gl_widget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_rosy_gl_widget_t qt_meta_stringdata_rosy_gl_widget = {
    {
QT_MOC_LITERAL(0, 0, 14) // "rosy_gl_widget"

    },
    "rosy_gl_widget"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_rosy_gl_widget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void rosy_gl_widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject rosy_gl_widget::staticMetaObject = { {
    QMetaObject::SuperData::link<field_gl_widget::staticMetaObject>(),
    qt_meta_stringdata_rosy_gl_widget.data,
    qt_meta_data_rosy_gl_widget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *rosy_gl_widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *rosy_gl_widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_rosy_gl_widget.stringdata0))
        return static_cast<void*>(this);
    return field_gl_widget::qt_metacast(_clname);
}

int rosy_gl_widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = field_gl_widget::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE