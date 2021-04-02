#pragma once

class QString;
class QWidegt;

class ViewPlugins {
public:
    struct ViewInstance {
        PDIView* view_plugin;
        QWidget* widget;
    };

    struct MemoryViewInstance {
        PDIMemoryView* view_plugin;
        QWidget* widget;
    };

    static void add_plugins(const &QString path);
    static std::vector<prodbg::View*>& view_plugins();
    static std::vector<prodbg::MemoryView*>& memory_plugins();

    static ViewInstance create_view_by_index(int index);
    static MemoryViewInstance create_memory_view_by_index(int index);
};
