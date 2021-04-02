#pragma once

class PDIMemoryView;
class PDIView;

class PDIPluginRegister {
public:
    virtual void register_view(PDIView* view) = 0;
    virtual void register_memory_view(PDIMemoryView* memory_view) = 0;
};

