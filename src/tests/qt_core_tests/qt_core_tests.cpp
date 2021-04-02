#include <QtTest/QtTest>
#include <core/BackendPluginHandler.h>
#include <backend/backend_session.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TestQString : public QObject {
    Q_OBJECT
public:
    Q_SLOT void isThisCalled();
private:
    Q_SLOT void initTestCase();
    Q_SLOT void toUpper();
    BackendSession* m_backend = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestQString::isThisCalled() {

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestQString::initTestCase() {
    //QCOMPARE(BackendPluginHandler::add_plugin("dummy_backend"), true);
    //m_backend = BackendSession::create_backend_session(QStringLiteral("dummy_backend"));
    //QVERIFY(m_backend != nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TestQString::toUpper() {
    //QString str = QStringLiteral("Hello");
    //QCOMPARE(str.toUpper(), QStringLiteral("HELLO"));
    //QTRY_LOOP_IMPL
}

QTEST_MAIN(TestQString)
#include "qt_core_tests.moc"

