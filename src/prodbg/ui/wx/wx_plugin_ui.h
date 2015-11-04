#include "../plugin.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WxPluginUI : public PluginUI {
    void create(void* windowHandle, int width, int height);
    void destroy();

    void init(ViewPluginInstance* instance);
    State updateInstance(ViewPluginInstance* instance, PDReader* reader, PDWriter* writer);

    void preUpdate();   // update before plugin update
    void postUpdate(); // update after plugins

    int getStatusBarSize();
    void setStatusTextNoFormat(const char* text);

private:
    int m_statusSize = 18;
    //char m_statusText[4096];
};

