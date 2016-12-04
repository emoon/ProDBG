#pragma once

class QString;
struct PDReader;
struct PDWriter;
struct PDBackendPlugin;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Session
{
public:
    Session();
    ~Session();

    static Session* createSession(const QString& backendName);
    bool setBackend(const QString& backendName);

    void start();
    void stop();
    void stepIn();
    void stepOver();
    void update();

private:
    void destroyPluginData();

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
