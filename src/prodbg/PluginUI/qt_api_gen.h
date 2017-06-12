#pragma once
#include <QObject>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*Signal_void)(void* priv_data);

class QSlotWrapperSignal_void : public QObject {
    Q_OBJECT
public:
    QSlotWrapperSignal_void(void* data, Signal_void func) {
        m_func = func;
        m_data = data;
    }

    Q_SLOT void method() {
        m_func(m_data);
    }
private:
    Signal_void m_func;
    void* m_data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*Signal_i32_void)(int value, void* priv_data);

class QSlotWrapperSignal_i32_void : public QObject {
    Q_OBJECT
public:
    QSlotWrapperSignal_i32_void(void* data, Signal_i32_void func) {
        m_func = func;
        m_data = data;
    }

    Q_SLOT void method(int value) {
        m_func(value, m_data);
    }
private:
    Signal_i32_void m_func;
    void* m_data;
};

