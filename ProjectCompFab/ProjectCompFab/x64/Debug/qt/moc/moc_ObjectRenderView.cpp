/****************************************************************************
** Meta object code from reading C++ file 'ObjectRenderView.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../ObjectRenderView.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ObjectRenderView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
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

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSObjectRenderViewENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSObjectRenderViewENDCLASS = QtMocHelpers::stringData(
    "ObjectRenderView",
    "setSliderSlicerHeight",
    "",
    "value",
    "sliceMesh",
    "std::vector<std::vector<std::vector<glm::dvec3>>>",
    "changeLayerHeight"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSObjectRenderViewENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   32,    2, 0x0a,    1 /* Public */,
       4,    0,   35,    2, 0x0a,    3 /* Public */,
       6,    1,   36,    2, 0x0a,    4 /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    3,
    0x80000000 | 5,
    QMetaType::Void, QMetaType::Double,    3,

       0        // eod
};

Q_CONSTINIT const QMetaObject ObjectRenderView::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSObjectRenderViewENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSObjectRenderViewENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSObjectRenderViewENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ObjectRenderView, std::true_type>,
        // method 'setSliderSlicerHeight'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'sliceMesh'
        QtPrivate::TypeAndForceComplete<std::vector<std::vector<std::vector<glm::dvec3>> >, std::false_type>,
        // method 'changeLayerHeight'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>
    >,
    nullptr
} };

void ObjectRenderView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ObjectRenderView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->setSliderSlicerHeight((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 1: { std::vector<std::vector<std::vector<glm::dvec3>>> _r = _t->sliceMesh();
            if (_a[0]) *reinterpret_cast< std::vector<std::vector<std::vector<glm::dvec3>>>*>(_a[0]) = std::move(_r); }  break;
        case 2: _t->changeLayerHeight((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *ObjectRenderView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ObjectRenderView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSObjectRenderViewENDCLASS.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QOpenGLExtraFunctions"))
        return static_cast< QOpenGLExtraFunctions*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int ObjectRenderView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
