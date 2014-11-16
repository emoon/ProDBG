#include <angelscript.h>
#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void MessageCallback(const asSMessageInfo* msg, void* param)
{
	const char* type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING)
		type = "WARN";
	else if (msg->type == asMSGTYPE_INFORMATION)
		type = "INFO";

	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	// Create the script engine
	asIScriptEngine* engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if (!engine)
	{
		printf("Failed to create script engine\n");
		return -1;
	}

	// The script compiler will write any compiler messages to the callback.
	engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

	return 0;
}