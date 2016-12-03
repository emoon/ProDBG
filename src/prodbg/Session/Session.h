#pragma once

class QString;
class QStatusBar;
struct PDReader;
struct PDWriter;
struct PDBackendPlugin;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Session
{
public:
    Session(QStatusBar* statusbar = 0);
    ~Session();

    static Session* createSession(const QString& backendName, QStatusBar* statusbar);
    bool setBackend(const QString& backendName);

    void start();
    void stop();
    void stepIn();
    void stepOver();
    void update();

private:
    void destroyPluginData();

    // Used to show the current status of the backend
    QStatusBar* m_status;

    // Writers/Read for communitaction between backend and UI
    PDWriter* m_writer0;
    PDWriter* m_writer1;
    PDWriter* m_currentWriter;
    PDWriter* m_prevWriter;
    PDReader* m_reader;

    // Current active backend plugin
    PDBackendPlugin* m_backendPlugin;

    // Data owned by current active backend plugin
    void* m_backendPluginData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
