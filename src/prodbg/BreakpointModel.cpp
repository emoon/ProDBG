#include "BreakpointModel.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BreakpointModel::toggleFileLineBreakpoint(const QString& filename, int line)
{
	for (int i = 0, count = m_fileLineBreakpoints.count(); i < count; ++i) {
		if (m_fileLineBreakpoints[i].line == line && 
			m_fileLineBreakpoints[i].filename == filename) { 
			m_fileLineBreakpoints.remove(i);
			return;
		}
	}

	FileLineBreakpoint bp = { filename, line };

	m_fileLineBreakpoints.append(bp);
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

void BreakpointModel::toggleAddressBreakpoint(uint64_t address)
{
	for (int i = 0, count = m_addressBreakpoints.count(); i < count; ++i) {
		if (m_addressBreakpoints[i] == address) {
			m_addressBreakpoints.remove(i);
			return;
		}
	}

	m_addressBreakpoints.append(address);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
