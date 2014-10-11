#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "core/core.h"
#include "core/session.h"
#include "api/plugin_instance.h"
#include "ui/plugin.h"
#include <pd_ui.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void verify_plugin_interface(void** state)
{
    PDUI ui;

    (void)state;

    prodbg::PluginUI_init("", &ui);

    assert_non_null(ui.columns);
    assert_non_null(ui.nextColumn);
    assert_non_null(ui.button);
    assert_non_null(ui.buttonSize);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    const UnitTest tests[] =
    {
        unit_test(verify_plugin_interface),
    };

    return run_tests(tests);
}

