#pragma once

#include <QObject>

typedef void (*SignalNoArgs)(void* data);

class QSignalWrapperNoArgs : public QObject {
	Q_OBJECT
	Q_SIGNAL void method(qint64);
};

class QSlotWrapperNoArgs : public QObject {
	Q_OBJECT
public:
	QSlotWrapperNoArgs(void* data, SignalNoArgs func) {
        m_func = func;
        m_data = data;
    }

	Q_SLOT void method() {
	    m_func(m_data);
    }

private:
	SignalNoArgs m_func;
	void* m_data;
};

