#pragma once

#include <QtCore/QVector>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: We likely want to separate breakpoints into more types.
// Right now this is source code and disassembly but we want to support breakpoint for hw events as well.
// This should likley be a model. Temp for now

class BreakpointModel {
   public:
    struct FileLineBreakpoint {
        QString filename;
        int line;
    };

    struct AddressBreakpoint {
        uint64_t address;
    };

    bool has_breakpoint_file_line(const QString& filename, int line);
    bool has_breakpoint_address(uint64_t address);

    bool toggle_file_line_breakpoint(const QString& filename, int line);
    bool toggle_address_breakpoint(uint64_t address);

    const QVector<FileLineBreakpoint>& get_file_line_breakpoints() { return m_file_line_breakpoints; }
    const QVector<uint64_t>& get_address_breakpoints() { return m_address_breakpoints; }

   private:
    QVector<FileLineBreakpoint> m_file_line_breakpoints;
    QVector<uint64_t> m_address_breakpoints;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
