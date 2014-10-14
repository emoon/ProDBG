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
    { "disassembly.so", "disassembly0", 0.1f, 0.1f, 0.2f, 0.2f },
    { "disassembly.so", "disassembly1", 0.2f, 0.2f, 0.3f, 0.4f },
    { "locals.so", "locals", 0.4f, 0.4f, 0.5f, 0.5f },
    { "registers.so", "registers", 0.7f, 0.7f, 0.1f, 0.1f },
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

static void ui_layout_save(void** state)
{
    (void)state;
    assert_true(UILayout_saveLayout(&s_layout, "t2-output/temp_layout.yaml"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cmp_float(float a, float b)
{
	if (fabs(a - b) < 0.0001f)
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_layout_load(void** state)
{
    UILayout layout;
    (void)state;

    assert_true(UILayout_loadLayout(&layout, "t2-output/temp_layout.yaml"));

    assert_int_equal(layout.layoutItemCount, s_layout.layoutItemCount);
    assert_int_equal(layout.basePathCount, s_layout.basePathCount);

    for (int i = 0; i < layout.basePathCount; ++i)
	{
		assert_non_null(layout.pluginBasePaths[i]);
		assert_string_equal(layout.pluginBasePaths[i], s_layout.pluginBasePaths[i]);
	}

	for (int i = 0; i < layout.layoutItemCount; ++i)
	{
		const LayoutItem* a = &layout.layoutItems[i];
		const LayoutItem* b = &s_layout.layoutItems[i];

		assert_non_null(a->pluginFile);
		assert_non_null(a->pluginName);

		assert_string_equal(a->pluginFile, b->pluginFile);
		assert_string_equal(a->pluginName, b->pluginName);

		assert_true(cmp_float(a->x, b->x));
		assert_true(cmp_float(a->y, b->y));
		assert_true(cmp_float(a->width, b->width));
		assert_true(cmp_float(a->height, b->height));
	}
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_session_get_layout(void** state)
{
	UILayout layout;
	(void)state;

	PluginHandler_addStaticPlugin(&s_pluginData);
	imguiSetup(1024, 768);

    ImGuiIO& io = ImGui::GetIO();

    struct Session* session = Session_create();

    struct ViewPluginInstance* i0 = PluginInstance_createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i1 = PluginInstance_createViewPlugin(&s_pluginData);
    struct ViewPluginInstance* i2 = PluginInstance_createViewPlugin(&s_pluginData);

    Session_addViewPlugin(session, i0);
    Session_addViewPlugin(session, i1);
    Session_addViewPlugin(session, i2);

    Session_getLayout(session, &layout, io.DisplaySize.x, io.DisplaySize.y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    const UnitTest tests[] =
    {
        unit_test(ui_layout_save),
        unit_test(ui_layout_load),
        unit_test(ui_session_get_layout),
    };

    return run_tests(tests);
}

