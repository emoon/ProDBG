#pragma once

#include <QString>
#include <QVector>
#include "breakpoint_model.h"

class QSettings;

namespace prodbg {

class BreakpointModel;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The purpose of project is to keep track of all the data for a project. A project is the data surrunding a specific file.
// This data can be things as working directory for executables, all window locations, breakpoints, etc
class Project {
public:
    // Project name, this is usually named the same as filename but can have custom name also
    QString m_name;
    // Target filename (may be a file to debug or just to view)
    QString m_filename;
    // Working directory is mainly used for executable debugging as the executable may
    // load files relative to a location and that location is kept here
    QString m_working_directory;
    // When starting an executable/script these arguments will be passed
    QString m_commandline_arguments;
    // Keep track of all fileline breakpoints
    QVector<BreakpointModel::FileLineBreakpoint> m_fileline_breakpoints;
    // Keep track of all address breakpoints
    QVector<BreakpointModel::AddressBreakpoint> m_address_breakpoints;
    // TODO: Store all layouts, plugin data, etc here also

    void read_settings(QSettings& settings);
    void write_settings(QSettings& setting);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
