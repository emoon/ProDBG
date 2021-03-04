#pragma once

namespace prodbg {

class MemoryView;
class View;

class PluginRegister {
public:
    virtual void register_view(View* view) = 0;
    virtual void register_memory_view(MemoryView* view) = 0;
};

}

