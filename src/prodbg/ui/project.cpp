#include "project.h"
#include "breakpoint_model.h"
#include <QSettings>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QString ADDRESS = QStringLiteral("address");
static QString ADDRESS_BREAKPOINTS = QStringLiteral("address_breakpoints");
static QString COMMANDLINE_ARGUMENTS = QStringLiteral("commandline_arguments");
static QString FILELINE_BREAKPOINTS = QStringLiteral("fileline_breakpoints");
static QString FILENAME = QStringLiteral("filename");
static QString LINE = QStringLiteral("line");
static QString PROJECT_NAME = QStringLiteral("project_name");
static QString WORKING_DIRECTORY = QStringLiteral("working_directory");

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Project::read_settings(QSettings& settings) {
    m_name = settings.value(PROJECT_NAME).toString();
    m_filename = settings.value(FILENAME).toString();
    m_working_directory = settings.value(WORKING_DIRECTORY).toString();
    m_commandline_arguments = settings.value(COMMANDLINE_ARGUMENTS).toString();

    m_fileline_breakpoints.clear();
    m_address_breakpoints.clear();

    int fl_count = settings.beginReadArray(FILELINE_BREAKPOINTS);
    m_fileline_breakpoints.resize(fl_count);

    for (int i = 0; i < fl_count; ++i) {
        settings.setArrayIndex(i);
        m_fileline_breakpoints[i].filename = settings.value(FILENAME).toString();
        m_fileline_breakpoints[i].line = settings.value(LINE).toInt();
    }

    settings.endArray();

    int address_count = settings.beginReadArray(FILELINE_BREAKPOINTS);
    m_address_breakpoints.resize(address_count);

    for (int i = 0; i < address_count; ++i) {
        settings.setArrayIndex(i);
        m_address_breakpoints[i].address = settings.value(ADDRESS).toInt();
    }

    settings.endArray();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Project::write_settings(QSettings& settings) {
    settings.setValue(PROJECT_NAME, m_name);
    settings.setValue(FILENAME, m_filename);
    settings.setValue(WORKING_DIRECTORY, m_working_directory);
    settings.setValue(COMMANDLINE_ARGUMENTS, m_commandline_arguments);

    int fl_count = m_fileline_breakpoints.count();

    if (fl_count > 0) {
        settings.beginWriteArray(FILELINE_BREAKPOINTS);

        for (int i = 0; i < fl_count; ++i) {
            settings.setArrayIndex(i);
            settings.setValue(FILENAME, m_fileline_breakpoints[i].filename);
            settings.setValue(LINE, m_fileline_breakpoints[i].line);
        }

        settings.endArray();
    }

    int address_count = m_address_breakpoints.count();

    if (address_count > 0) {
        settings.beginWriteArray(ADDRESS_BREAKPOINTS);

        for (int i = 0; i < address_count; ++i) {
            settings.setArrayIndex(i);
            settings.setValue(ADDRESS, (qulonglong)m_address_breakpoints[i].address);
        }

        settings.endArray();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
