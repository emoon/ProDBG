#pragma once
#include <QObject>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*Signal_self_i32_void)(void*, self_c int, value );

class QSlotWrapperSignal_self_i32_void : public QObject {
    Q_OBJECT
public:
    QSlotWrapperSignal_self_i32_void(void* data, Signal_self_i32_void func) {
        m_func = func;
        m_data = data;
    }

    Q_SLOT void method(self self_c,int value) {
        m_func(self_c, value, m_data);
    }
private:
    Signal_self_i32_void m_func;
    void* m_data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*Signal_self_void)(void*, self_c );

class QSlotWrapperSignal_self_void : public QObject {
    Q_OBJECT
public:
    QSlotWrapperSignal_self_void(void* data, Signal_self_void func) {
        m_func = func;
        m_data = data;
    }

    Q_SLOT void method(void* self_c) {
        m_func(self_c, m_data);
    }
private:
    Signal_self_void m_func;
    void* m_data;
};

