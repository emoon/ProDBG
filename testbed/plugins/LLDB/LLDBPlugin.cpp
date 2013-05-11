#include <ProDBGAPI.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <SBTarget.h>
#include <SBListener.h>
#include <SBProcess.h>
#include <SBDebugger.h>
#include <SBHostOS.h>
#include <SBEvent.h>
#include <SBBreakpoint.h>
#include <SBStream.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct LLDBPlugin
{
	lldb::SBDebugger debugger;
	lldb::SBTarget target;
	lldb::SBListener listener;

} LLDBPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(ServiceFunc* serviceFunc)
{
	printf("Create instance\n");
	LLDBPlugin* plugin = new LLDBPlugin; 
	return plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onBreak(LLDBPlugin* data, void* actionData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStep(LLDBPlugin* data, void* actionData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStepOver(LLDBPlugin* data, void* actionData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onSetCodeBreakpoint(LLDBPlugin* data, void* actionData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void actionCallback(void* userData, PDDebugAction action, void* actionData)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	switch (action)
	{
		case PD_DEBUG_ACTION_BREAK : onBreak(plugin, actionData); break;
		case PD_DEBUG_ACTION_STEP : onStep(plugin, actionData); break;
		case PD_DEBUG_ACTION_STEP_OVER : onStepOver(plugin, actionData); break;
		case PD_DEBUG_ACTION_SET_CODE_BREAKPOINT : onSetCodeBreakpoint(plugin, actionData); break;
	}

}

const char* sections_names[] =
{
	"eSectionTypeInvalid",
	"eSectionTypeCode",
	"eSectionTypeContainer",              // The section contains child sections
	"eSectionTypeData",
	"eSectionTypeDataCString",            // Inlined C string data
	"eSectionTypeDataCStringPointers",    // Pointers to C string data
	"eSectionTypeDataSymbolAddress",      // Address of a symbol in the symbol table
	"eSectionTypeData4",
	"eSectionTypeData8",
	"eSectionTypeData16",
	"eSectionTypeDataPointers",
	"eSectionTypeDebug",
	"eSectionTypeZeroFill",
	"eSectionTypeDataObjCMessageRefs",    // Pointer to function pointer + selector
	"eSectionTypeDataObjCCFStrings",      // Objective C const CFString/NSString objects
	"eSectionTypeDWARFDebugAbbrev",
	"eSectionTypeDWARFDebugAranges",
	"eSectionTypeDWARFDebugFrame",
	"eSectionTypeDWARFDebugInfo",
	"eSectionTypeDWARFDebugLine",
	"eSectionTypeDWARFDebugLoc",
	"eSectionTypeDWARFDebugMacInfo",
	"eSectionTypeDWARFDebugPubNames",
	"eSectionTypeDWARFDebugPubTypes",
	"eSectionTypeDWARFDebugRanges",
	"eSectionTypeDWARFDebugStr",
	"eSectionTypeDWARFAppleNames",
	"eSectionTypeDWARFAppleTypes",
	"eSectionTypeDWARFAppleNamespaces",
	"eSectionTypeDWARFAppleObjC",
	"eSectionTypeEHFrame",
	"eSectionTypeOther"
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool startCallback(void* userData, PDLaunchAction action, void* launchData)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	printf("Trying to start debug session\n");

	lldb::SBDebugger::Initialize();
	lldb::SBHostOS::ThreadCreated ("<lldb-tester.app.main>");
 	plugin->debugger = lldb::SBDebugger::Create(false);
 	plugin->listener = plugin->debugger.GetListener(); 

	/*
	const char* cat[] =
	{
		"all",
		0
	};
	*/

	// very hacky right now but is only used for testing anyway
	plugin->target = plugin->debugger.CreateTarget((const char*)launchData);

	// dump some data
	//
	
#if 0
	for (uint32_t i = 0; i < 1;/*plugin->target.GetNumModules()*/ ++i)
	{
		lldb::SBStream desc;
    	lldb::SBModule module = plugin->target.GetModuleAtIndex(i);

    	module.GetDescription(desc);

    	printf("module desc %s\n", desc.GetData());
    	printf("CompileUnits %d\n", module.GetNumCompileUnits());

		for (size_t k = 0; k < module.GetNumCompileUnits(); ++k)
		{
			lldb::SBCompileUnit unit = module.GetCompileUnitAtIndex((uint32_t)k);

			for (size_t p = 0; p < unit.GetNumSupportFiles(); ++p)
			{
				lldb::SBFileSpec fileSpec = unit.GetSupportFileAtIndex((uint32_t)p);
				printf("support file %s\n", fileSpec.GetFilename());
			}
		}

		for (uint32_t k = 0; k < module.GetNumSymbols(); ++k)
		{
			lldb::SBSymbol symbol = module.GetSymbolAtIndex(k);
			printf("%d %s\n", k, symbol.GetName());
		}

		/*
    	printf("ModuleDesc %s\n", desc.GetData());
    	
    	for (uint32_t p = 0; p < module.GetNumSections(); ++p)
		{
			lldb::SBSection section = module.GetSectionAtIndex(p);
			printSectionsReursive(section, 0);
		}
		*/
	}
#endif


	/*
	if (breakpoint.IsValid())
	{
		printf("Added valid breakpoint\n");
	}

	if (!plugin->debugger.EnableLog("lldb", cat))
	{
		printf("Unable to enable log!\n");
	}
	*/
	
	if (!plugin->target.IsValid())
		return false;

	/*

	printf("Target is valid, launching\n");

	lldb::SBLaunchInfo launchInfo(0);
	lldb::SBError error;

	lldb::SBProcess process = plugin->target.Launch(launchInfo, error);

	if (!error.Success())
	{
		printf("error false\n");
		return;
	}

	if (!process.IsValid())
	{
		printf("process not valid\n");
		return;
	}

	process.GetBroadcaster().AddListener(
			plugin->listener, 
			lldb::SBProcess::eBroadcastBitStateChanged | lldb::SBProcess::eBroadcastBitInterrupt);

    lldb::SBEvent evt;

    while (1)
	{
		plugin->listener.WaitForEvent (UINT32_MAX, evt);
		lldb::StateType state =  lldb::SBProcess::GetStateFromEvent (evt);
		printf("event = %s\n", lldb::SBDebugger::StateAsCString(state));

		if (state == lldb::eStateStopped)
		{
			process.Kill();
			return;
		}

		usleep(10000);
	}
	*/

	return true;

	//process.Destroy();
	//lldb::SBDebugger::Destroy(plugin->debugger);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugPlugin plugin =
{
	createInstance,
	destroyInstance,
	startCallback,
	actionCallback,
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

void InitPlugin(int version, ServiceFunc* serviceFunc, RegisterPlugin* registerPlugin)
{
	printf("Starting to register Plugin!\n");
	registerPlugin(PD_PTYPE_DEBUGGER, &plugin);
}

}

