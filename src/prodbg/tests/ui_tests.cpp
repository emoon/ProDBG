#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <math.h>
#include <pd_view.h>
#include <pd_ui.h>

#include "api/plugin_instance.h"
#include "core/plugin_handler.h"
#include "core/core.h"
#include "session/session.h"
#include "ui/imgui/imgui.h"
#include "ui/plugin.h"
#include "ui/ui_layout.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static LayoutItem s_layoutItems[] =
{
    { "disassembly.so", "disassembly0", { 0.1f, 0.1f, 0.2f, 0.2f } },
    { "disassembly.so", "disassembly1", { 0.2f, 0.2f, 0.3f, 0.4f } },
    { "locals.so", "locals", { 0.4f, 0.4f, 0.5f, 0.5f } },
    { "registers.so", "registers", { 0.7f, 0.7f, 0.1f, 0.1f } },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* s_basePaths[] = { "pluginPath/temp", "customPath/plugins" };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static UILayout s_layout =
{
    s_basePaths,
    s_layoutItems,
    sizeof_array(s_basePaths),
    sizeof_array(s_layoutItems),
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cmp_float(float a, float b)
{
	if (fabs(a - b) < 0.0001f)
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void compareLayouts(UILayout* v0, UILayout* v1)
{
    assert_int_equal(v0->layoutItemCount, v1->layoutItemCount);
    assert_int_equal(v0->basePathCount, v1->basePathCount);

    for (int i = 0; i < v0->basePathCount; ++i)
	{
		assert_non_null(v0->pluginBasePaths[i]);
		assert_string_equal(v0->pluginBasePaths[i], v1->pluginBasePaths[i]);
	}

	for (int i = 0; i < v0->layoutItemCount; ++i)
	{
		const LayoutItem* a = &v0->layoutItems[i];
		const LayoutItem* b = &v1->layoutItems[i];

		assert_non_null(a->pluginFile);
		assert_non_null(a->pluginName);
		assert_non_null(b->pluginFile);
		assert_non_null(b->pluginName);

		assert_string_equal(a->pluginFile, b->pluginFile);
		assert_string_equal(a->pluginName, b->pluginName);

		assert_true(cmp_float(a->rect.x, b->rect.x));
		assert_true(cmp_float(a->rect.y, b->rect.y));
		assert_true(cmp_float(a->rect.width, b->rect.width));
		assert_true(cmp_float(a->rect.height, b->rect.height));
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_layout_save_load(void** state)
{
    UILayout layout;
    (void)state;

    assert_true(UILayout_saveLayout(&s_layout, "t2-output/temp_layout.yaml"));
    assert_true(UILayout_loadLayout(&layout, "t2-output/temp_layout.yaml"));

	compareLayouts(&layout, &s_layout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* dummyCreateInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)uiFuncs;
    (void)serviceFunc;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dummyDestroyInstance(void* userData)
{
    (void)userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int dummyUpdate(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
    (void)userData;
    (void)uiFuncs;
    (void)inEvents;
    (void)outEvents;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin s_dummyPlugin =
{
    "DummyPlugin",
    dummyCreateInstance,
    dummyDestroyInstance,
    dummyUpdate,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PluginData s_pluginData = { &s_dummyPlugin, PD_VIEW_API_VERSION, "", 0 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void imguiDummyRender(ImDrawList** const cmd_lists, int cmd_lists_count)
{
    (void)cmd_lists;
    (void)cmd_lists_count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void imguiSetup(int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = 1.0f / 60.0f;
    io.PixelCenterOffset = 0.5f;
    io.RenderDrawListsFn = imguiDummyRender;
}

static FloatRect s_t0 = { 80.0f, 80.0f, 120.0f, 120.0f };
static FloatRect s_t1 = { 180.0f, 120.0f, 220.0f, 240.0f };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_session_layout(void** state)
{
	UILayout layout;
	UILayout layout2;
	(void)state;

	PluginHandler_addStaticPlugin(&s_pluginData);
	imguiSetup(1024, 768);

    ImGuiIO& io = ImGui::GetIO();

    Session* session = Session_create();
    Session* session2 = Session_create();

	PluginData* registersPlugin = PluginHandler_findPlugin(0, "registers_plugin", "Registers View", true);
	PluginData* sourcecodePlugin = PluginHandler_findPlugin(0, "sourcecode_plugin", "Source Code View", true);

    ViewPluginInstance* i0 = PluginInstance_createViewPlugin(registersPlugin);
    ViewPluginInstance* i1 = PluginInstance_createViewPlugin(sourcecodePlugin);

    Session_addViewPlugin(session, i0);
    Session_addViewPlugin(session, i1);

    PluginUI_setWindowRect(i0, &s_t0);
    PluginUI_setWindowRect(i1, &s_t1);

    Session_getLayout(session, &layout, io.DisplaySize.x, io.DisplaySize.y);
    UILayout_saveLayout(&layout, "t2-output/temp_layout2.yaml");

    // Tear down

	Session_removeViewPlugin(session, i0);
	Session_removeViewPlugin(session, i1);
	PluginHandler_unloadAllPlugins();

	// Load the layout

    UILayout_loadLayout(&layout2, "t2-output/temp_layout2.yaml");
    compareLayouts(&layout, &layout2);

    int count = 0;

    Session_setLayout(session2, &layout2, io.DisplaySize.x, io.DisplaySize.y);

	ViewPluginInstance** plugins = Session_getViewPlugins(session2, &count);

	assert_int_equal(layout.layoutItemCount, count);

	for (int i = 0; i < layout.layoutItemCount; ++i)
	{
		FloatRect rect;

		PluginUI_getWindowRect(plugins[i], &rect);

		const LayoutItem* item = &layout2.layoutItems[i];

		assert_int_equal((int)(item->rect.x * io.DisplaySize.x), (int)rect.x);
		assert_int_equal((int)(item->rect.y * io.DisplaySize.y), (int)rect.y);
		assert_int_equal((int)(item->rect.width * io.DisplaySize.x), (int)rect.width);
		assert_int_equal((int)(item->rect.height * io.DisplaySize.y), (int)rect.height);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    const UnitTest tests[] =
    {
        unit_test(ui_layout_save_load),
        unit_test(ui_session_layout),
    };

    return run_tests(tests);
}

