#pragma once
#include <QObject>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*Signal_self_i32_void)(void* self_c, int value);

class QSlotWrapperSignal_self_i32_void : public QObject {
    Q_OBJECT
public:
    QSlotWrapperSignal_self_i32_void(void* data, Signal_self_i32_void func) {
        m_func = func;
        m_data = data;
    }

    Q_SLOT void method(int value) {
        m_func(m_data, value);
    }
private:
    Signal_self_i32_void m_func;
    void* m_data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*Signal_self_void)(void* self_c);

class QSlotWrapperSignal_self_void : public QObject {
    Q_OBJECT
public:
    QSlotWrapperSignal_self_void(void* data, Signal_self_void func) {
        m_func = func;
        m_data = data;
    }

    Q_SLOT void method() {
        m_func(m_data);
    }
private:
    Signal_self_void m_func;
    void* m_data;
};

