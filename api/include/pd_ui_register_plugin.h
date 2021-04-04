#pragma once

class PDMemoryView;
class PDView;

class PDRegisterViewPlugin {
public:
    virtual void register_view(PDView* view) = 0;
    virtual void register_memory_view(PDMemoryView* memory_view) = 0;
};

