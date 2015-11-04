#include "commands.h"
#include "alloc.h"
#include <string.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct CommandEntry {
    Command command;
    struct CommandEntry* next;
    struct CommandEntry* prev;
} CommandEntry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct CommandList {
    CommandEntry* first;
    CommandEntry* last;
} CommandList;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static CommandList s_undoStack;
static CommandList s_redoStack;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CommandList_addEntry(CommandList* commandList, CommandEntry* command);
//static void CommandList_delEntry(CommandList* commandList, Command* command);
static void CommandList_clear(CommandList* commandList);
static bool CommandList_isEmpty(CommandList* list);
static void CommandList_pop(CommandList* commandList);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct MultiCommandData {
    CommandList list;
};

static struct MultiCommandData* s_multiCommand = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands_init() {
    memset(&s_undoStack, 0, sizeof(CommandList));
    memset(&s_redoStack, 0, sizeof(CommandList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int countEntriesInList(CommandList* list) {
    CommandEntry* command;
    int count = 0;

    for (command = list->first; command; command = command->next)
        count++;

    return count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Commands_undoCount() {
    return countEntriesInList(&s_undoStack);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void execCommand(CommandEntry* entry) {
    // set if we have multi command recording enabled)

    if (s_multiCommand) {
        CommandList_addEntry(&s_multiCommand->list, entry);
    }else {
        CommandList_addEntry(&s_undoStack, entry);
        entry->command.exec(entry->command.user_data);
    }

    CommandList_clear(&s_redoStack);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void execMultiCommand(void* user_data) {
    CommandEntry* entry;
    struct MultiCommandData* data = (struct MultiCommandData*)user_data;

    for (entry = data->list.first; entry; entry = entry->next)
        entry->command.exec(entry->command.user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void undoMultiCommand(void* user_data) {
    CommandEntry* entry;
    struct MultiCommandData* data = (struct MultiCommandData*)user_data;

    for (entry = data->list.first; entry; entry = entry->next)
        entry->command.undo(entry->command.user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands_beginMulti() {
    s_multiCommand = (struct MultiCommandData*)alloc_zero(sizeof(struct MultiCommandData));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands_endMulti() {
    // Check if any command was added during multi command

    if (CommandList_isEmpty(&s_multiCommand->list)) {
        free(s_multiCommand);
        s_multiCommand = 0;
        return;
    }

    CommandEntry* entry = (CommandEntry*)alloc_zero(sizeof(CommandEntry));

    entry->command = { s_multiCommand, execMultiCommand, undoMultiCommand };

    s_multiCommand = 0;

    execCommand(entry);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands_undo() {
    if (CommandList_isEmpty(&s_undoStack))
        return;

    CommandEntry* entry = s_undoStack.last;
    CommandList_pop(&s_undoStack);

    entry->prev = 0;
    entry->next = 0;

    CommandList_addEntry(&s_redoStack, entry);

    entry->command.undo(entry->command.user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands_redo() {
    if (CommandList_isEmpty(&s_redoStack))
        return;

    CommandEntry* entry = s_redoStack.last;
    CommandList_pop(&s_redoStack);

    entry->prev = 0;
    entry->next = 0;

    CommandList_addEntry(&s_undoStack, entry);

    entry->command.exec(entry->command.user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands_execute(Command command) {
    CommandEntry* entry = (CommandEntry*)alloc_zero(sizeof(CommandEntry));
    entry->command = command;

    execCommand(entry);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CommandList_addEntry(CommandList* list, CommandEntry* command) {
    if (list->last) {
        list->last->next = command;
        command->prev = list->last;
        list->last = command;
    }else {
        list->first = command;
        list->last = command;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CommandList_unlinkEntry(CommandList* list, CommandEntry* entry) {
    CommandEntry* prev;
    CommandEntry* next;

    prev = entry->prev;
    next = entry->next;

    if (prev) {
        if (next) {
            prev->next = next;
            next->prev = prev;
        }else {
            prev->next = 0;
            list->last = prev;
        }
    }else {
        if (next) {
            next->prev = 0;
            list->first = next;
        }else {
            list->first = 0;
            list->last = 0;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CommandList_delEntry(CommandList* list, CommandEntry* entry) {
    CommandList_unlinkEntry(list, entry);

    free(entry->command.user_data);
    free(entry);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CommandList_clear(CommandList* list) {
    while (list->last) {
        CommandEntry* entry = list->last;
        CommandList_delEntry(list, entry);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CommandList_pop(CommandList* list) {
    CommandList_unlinkEntry(list, list->last);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool CommandList_isEmpty(CommandList* list) {
    return (!list->first && !list->last);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Commands_clear() {
    CommandList_clear(&s_undoStack);
    CommandList_clear(&s_redoStack);
}

