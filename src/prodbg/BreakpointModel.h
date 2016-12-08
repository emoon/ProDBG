#pragma once

// #include <QStandardItemModel>
#include <QVector>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This should likley be a model. Temp for now

class BreakpointModel
{
public:
	struct FileLineBreakpoint
	{
		QString filename;
		int line;
	};

	struct AddressBreakpoint
	{
		uint64_t address;
	};

	bool hasBreakpointFileLine(const QString& filename, int line);

	void toggleFileLineBreakpoint(const QString& filename, int line);
	void toggleAddressBreakpoint(uint64_t address);

	const QVector<FileLineBreakpoint>& getFileLineBreakpoints() { return m_fileLineBreakpoints; }
	const QVector<uint64_t>& getAddressBreakpoints() { return m_addressBreakpoints; }
	
private:

	QVector<FileLineBreakpoint> m_fileLineBreakpoints;
	QVector<uint64_t> m_addressBreakpoints;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
