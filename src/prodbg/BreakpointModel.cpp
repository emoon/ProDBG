#include "BreakpointModel.h"
#include <QSettings>

namespace prodbg {


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BreakpointModel::toggle_file_line_breakpoint(const QString& filename, int line) {
    for (int i = 0, count = m_file_line_breakpoints.count(); i < count; ++i) {
        if (m_file_line_breakpoints[i].line == line && m_file_line_breakpoints[i].filename == filename) {
            m_file_line_breakpoints.remove(i);
            return false;
        }
    }

    FileLineBreakpoint bp = {filename, line};

    m_file_line_breakpoints.append(bp);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BreakpointModel::has_breakpoint_file_line(const QString& filename, int line) {
    for (auto& bp : m_file_line_breakpoints) {
        if (bp.line == line && bp.filename == filename) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BreakpointModel::has_breakpoint_address(uint64_t address) {
    for (auto& bp : m_address_breakpoints) {
        if (bp == address) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BreakpointModel::toggle_address_breakpoint(uint64_t address) {
    for (int i = 0, count = m_address_breakpoints.count(); i < count; ++i) {
        if (m_address_breakpoints[i] == address) {
            m_address_breakpoints.remove(i);
            return false;
        }
    }

    m_address_breakpoints.append(address);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
