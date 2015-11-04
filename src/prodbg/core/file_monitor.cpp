#ifndef _WIN32
#include <alloca.h>
#endif

#include "file_monitor.h"
#include <foundation/memory.h>
#include <foundation/array.h>
#include <foundation/event.h>
#include <foundation/fs.h>
#include <foundation/path.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FileNotificationData {
    const char* path;
    const char* fileFilters;
    FMCallback callback;
    void* userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct FileNotificationData* s_notficationData = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileMonitor_addPath(const char* path, const char* fileFilters, FMCallback callback, void* userData) {
    FileNotificationData data = { path, fileFilters, callback, userData };

    array_push(s_notficationData, data);

    fs_monitor(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileMonitor_removePath(const char* path) {
    int size = array_size(s_notficationData);

    for (int i = 0; i < size; ++i) {
        if (!strcmp(s_notficationData[i].path, path)) {
            fs_unmonitor(path);
            array_erase(s_notficationData, i);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateCallbacks(event_t* event) {
    int size = array_size(s_notficationData);
    const char* filename = event->payload;

    for (int i = 0; i < size; ++i) {
        FileNotificationData* noteData = &s_notficationData[i];

        if (!strstr(filename, noteData->path))
            continue;

        if (strcmp(noteData->fileFilters, "*") == 0) {
            noteData->callback(noteData->userData, filename, event->id);
        }else {
            int filter_count = 1;
            char* file_filters = (char*)alloca(strlen(noteData->fileFilters) + 1);
            strcpy(file_filters, noteData->fileFilters);
            const char* delimiter = ";";
            char* found;
            found = strpbrk(file_filters, delimiter);

            while (found != NULL) {
                ++filter_count;
                found = strpbrk(found + 1, delimiter);
            }

            char** filter_extensions = (char**)alloca((size_t)filter_count * sizeof(char*));

            found = strtok(file_filters, delimiter);
            int current_filter = 0;
            while (found != NULL) {
                filter_extensions[current_filter] = (char*)alloca(strlen(found) + 1);
                strcpy(filter_extensions[current_filter++], found);
                found = strtok(NULL, delimiter);
            }

            bool matching_filter = false;
            for (int filter_index = 0; filter_index < filter_count; ++filter_index) {
                char* extension = path_file_extension(filename);
                found = strstr(extension, filter_extensions[filter_index]);
                if (found) {
                    matching_filter = true;
                    break;
                }
            }

            if (matching_filter)
                noteData->callback(noteData->userData, filename, event->id);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileMonitor_update() {
    event_block_t* block = 0;
    event_t* event = 0;

    block = event_stream_process(fs_event_stream());

    while ((event = event_next(block, event))) {
        switch (event->id) {
            case FOUNDATIONEVENT_FILE_CREATED:
            case FOUNDATIONEVENT_FILE_DELETED:
            case FOUNDATIONEVENT_FILE_MODIFIED:
                updateCallbacks(event);
                break;
        }
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileMonitor_close() {
    int size = array_size(s_notficationData);

    for (int i = 0; i < size; ++i) {
        fs_unmonitor(s_notficationData[i].path);
        array_erase(s_notficationData, i);
    }

    array_deallocate(s_notficationData);
}


