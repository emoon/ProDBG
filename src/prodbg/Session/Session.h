#pragma once

class QString;
class QStatusBar;
struct PDReader;
struct PDWriter;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Session
{
public:
    Session(QStatusBar* statusbar = 0);
    ~Session();

    static Session* createSession(const QString& backendName, const QStatusBar* statusbar);

    void update();

private:
    QStatusBar* m_status;
    PDWriter* m_writer0;
    PDWriter* m_writer1;
    PDWriter* m_currentWriter;
    PDWriter* m_prevWriter;
    PDReader* m_reader;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
