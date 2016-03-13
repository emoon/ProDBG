#include "pd_view.h"
#include "pd_backend.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Line {
    uint64_t address;
    const char* text;
    bool breakpoint;
    uint8_t addressSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int BlockSize = 64;

struct Block {
    uint64_t id;
    uint64_t address;
    uint64_t addressEnd;
    std::vector<Line> lines;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DissassemblyData {
    std::vector<Block*> blocks;
    uint64_t location;
    uint64_t pc;
    uint8_t locationSize;
    bool requestDisassembly;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc) {
    (void)serviceFunc;
    DissassemblyData* user_data = new DissassemblyData;
    user_data->location = 0;
    user_data->pc = 0;
    user_data->locationSize = 0;
    user_data->requestDisassembly = false;

    (void)uiFuncs;
    (void)serviceFunc;

    return user_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* user_data) {
    delete (DissassemblyData*)user_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Block* findBlock(DissassemblyData* data, uint64_t blockId) {
    for (Block* b : data->blocks) {
        if (b->id == blockId)
            return b;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Block* createBlock(DissassemblyData* data, uint64_t address, uint64_t blockId) {
    Block* block = new Block;

    (void)address;

    block->id = blockId;
    block->address = blockId * BlockSize;
    block->addressEnd = block->address + (uint64_t)BlockSize;

    data->blocks.push_back(block);

    return block;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void insertLineBlock(Block* block, uint64_t address, const char* text) {
    // So this isn't very smart but will do for now

    for (auto i = block->lines.begin(); i != block->lines.end(); ++i) {
        Line& line = *i;

        if (address < line.address) {
            Line newLine = { 0 };
            newLine.address = address;
            newLine.text = strdup(text);
            block->lines.insert(i, newLine);
            return;
        }

        // found matching address, update the disassembly

        if (line.address == address) {
            free((void*)line.text);
            line.text = (const char*)strdup(text);
            return;
        }

        // TODO: Handle the case if a new line is inbetween lines, meaning the code has been modified
        // so the disasssembly is out of data
    }

    // not find, insert the line
    Line newLine = { 0 };
    newLine.address = address;
    newLine.text = strdup(text);
    block->lines.push_back(newLine);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void insertLine(DissassemblyData* data, uint64_t address, const char* text) {
    Block* block = 0;

    // first find the block which this address should be in

    uint64_t blockId = address / BlockSize;

    if (!(block = findBlock(data, blockId)))
        block = createBlock(data, address, blockId);

    insertLineBlock(block, address, text);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setDisassemblyCode(DissassemblyData* data, PDReader* reader) {
    PDReaderIterator it;

    if (PDRead_find_array(reader, &it, "disassembly", 0) == PDReadStatus_NotFound)
        return;

    while (PDRead_get_next_entry(reader, &it)) {
        uint64_t address;
        const char* text;

        PDRead_find_u64(reader, &address, "address", it);
        PDRead_find_string(reader, &text, "line", it);

        insertLine(data, address, text);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Block* findBlockWithPC(DissassemblyData* data, uint64_t pc) {
    for (Block* b : data->blocks) {
        if (pc >= b->address && pc <= b->addressEnd)
            return b;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void renderUI(DissassemblyData* data, PDUI* uiFuncs) {
    Block* block = 0;

    uiFuncs->text("");  // TODO: Temporary

    if (!(block = findBlockWithPC(data, data->pc)))
        return;

    PDVec2 size = uiFuncs->get_window_size();

    for (Line& line : block->lines) {
        if (line.address == data->pc) {
            PDRect rect;
            PDVec2 pos = uiFuncs->get_cursor_pos();
            rect.x = pos.x;
            rect.y = pos.y;
            rect.width = size.x;
            rect.height = 14;
            uiFuncs->fill_rect(rect, PDUI_COLOR(200, 0, 0, 127));
            uiFuncs->set_scroll_here(0.5f);
        }

        uiFuncs->text("0x%04x %s", (uint64_t)line.address, line.text);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateRegisters(DissassemblyData* data, PDReader* reader) {
    PDReaderIterator it;

    if (PDRead_find_array(reader, &it, "registers", 0) == PDReadStatus_NotFound)
        return;

    while (PDRead_get_next_entry(reader, &it)) {
        const char* name = "";

        PDRead_find_string(reader, &name, "name", it);

        if (!strcmp(name, "pc")) {
            PDRead_find_u64(reader, &data->pc, "register", it);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* user_data, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer) {
    uint32_t event;

    DissassemblyData* data = (DissassemblyData*)user_data;

    data->requestDisassembly = false;

    while ((event = PDRead_get_event(inEvents)) != 0) {
        switch (event) {
            case PDEventType_SetDisassembly:
            {
                setDisassemblyCode(data, inEvents);
                break;
            }

            case PDEventType_SetExceptionLocation:
            {
                uint64_t location = 0;

                PDRead_find_u64(inEvents, &location, "address", 0);

                if (location != data->location) {
                    data->location = location;
                    data->requestDisassembly = true;
                }

                PDRead_find_u8(inEvents, &data->locationSize, "address_size", 0);
                break;
            }

            case PDEventType_SetRegisters:
            {
                updateRegisters(data, inEvents);
                break;
            }

        }
    }

    renderUI(data, uiFuncs);

    if (data->requestDisassembly) {
        int pc = (int)(data->pc) & ~(BlockSize - 1);
        PDWrite_event_begin(writer, PDEventType_GetDisassembly);
        PDWrite_u64(writer, "address_start", (uint64_t)pc);
        PDWrite_u32(writer, "instruction_count", (uint32_t)BlockSize / 3);
        PDWrite_event_end(writer);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Disassembly",
    createInstance,
    destroyInstance,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
	registerPlugin(PD_VIEW_API_VERSION, &plugin, private_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

