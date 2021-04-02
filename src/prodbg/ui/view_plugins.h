#pragma once

#include <vector>

class QString;
class QWidget;
class PDView;
class PDMemoryView;

class ViewPlugins {
public:
    struct ViewInstance {
        PDView* view_plugin;
        QWidget* widget;
    };

    struct MemoryViewInstance {
        PDMemoryView* view_plugin;
        QWidget* widget;
    };

    static void add_plugins(const QString& path);
    static std::vector<PDView*>& view_plugins();
    static std::vector<PDMemoryView*>& memory_plugins();

    static ViewInstance create_view_by_index(int index);
    static MemoryViewInstance create_memory_view_by_index(int index);
};
