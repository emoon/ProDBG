#include "asdbg.h"
#include "asdbg_engine.h"

namespace asdbg
{

void Engine::registerToStringFunc(const asIObjectType* type, ToStringFunc callback)
{

}

Engine::Engine()
{

}

Engine::~Engine()
{

}

void Engine::takeCommands(asIScriptContext* context)
{

}

void Engine::output(const std::string& text)
{

}

void Engine::lineCallback(asIScriptContext* context)
{

}

void Engine::printHelp()
{

}

void Engine::addFileBreakpoint(const std::string& file, int line)
{

}

void Engine::addFuncBreakpoint(const std::string& func)
{

}

void Engine::listBreakpoints()
{

}

void Engine::listLocalVariables(asIScriptContext* context)
{

}

void Engine::listGlobalVariables(asIScriptContext* context)
{

}

void Engine::listMemberProperties(asIScriptContext* context)
{

}

void Engine::listStatistics(asIScriptContext* context)
{

}

void Engine::printCallstack(asIScriptContext* context)
{

}

void Engine::printValue(const std::string& expression, asIScriptContext* context)
{

}

bool Engine::interpretCommand(const std::string& command, asIScriptContext* context)
{
	return false;
}

bool Engine::checkBreakPoint(asIScriptContext* context)
{
	return false;
}

std::string Engine::toString(void* value, asUINT typeId, bool expandMembers, asIScriptEngine* engine)
{
	return std::string();
}

}