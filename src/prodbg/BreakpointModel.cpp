#include "BreakpointModel.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BreakpointModel::toggleFileLineBreakpoint(const QString& filename, int line)
{
    for (int i = 0, count = m_fileLineBreakpoints.count(); i < count; ++i) {
        if (m_fileLineBreakpoints[i].line == line && m_fileLineBreakpoints[i].filename == filename) {
            m_fileLineBreakpoints.remove(i);
            return false;
        }
    }

    FileLineBreakpoint bp = { filename, line };

    m_fileLineBreakpoints.append(bp);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BreakpointModel::hasBreakpointFileLine(const QString& filename, int line)
{
    for (auto& bp : m_fileLineBreakpoints) {
        if (bp.line == line && bp.filename == filename) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BreakpointModel::hasBreakpointAddress(uint64_t address)
{
    for (auto& bp : m_addressBreakpoints) {
        if (bp == address) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BreakpointModel::toggleAddressBreakpoint(uint64_t address)
{
    for (int i = 0, count = m_addressBreakpoints.count(); i < count; ++i) {
        if (m_addressBreakpoints[i] == address) {
            m_addressBreakpoints.remove(i);
            return false;
        }
    }

    m_addressBreakpoints.append(address);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
