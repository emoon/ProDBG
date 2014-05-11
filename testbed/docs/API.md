Introduction to the ProDBG API
==============================

First a *warning* This documentation and the API is work in progress and may change at any time.

There are several plugin types in ProDBG but we will start with the most important one and that is the regular debugger plugins.


Debugger Plugin
---------------

Plugins needs to implement a C interface and it's easiest to describe how it works with an example:

    typedef struct PDBackendPlugin
    {
        int version;
        const char* name;

        void* (*createInstance)(PDServiceFuncs* serviceFuncs);
        void (*destroyInstance)(void* userData);

        // Updates and Returns the current state of the plugin.
        PDDebugState (*update)(void* userData, PDAction action, PDReader* inEvents, PDWriter* outEvents);

        // Writer functions used for writing data back to the host
        PDReader reader;

        // Writer functions used for writing data back to the host
        PDWriter writer;
        // Create and destroy instance of the plugin

    } PDBackendPlugin;

The idea is to have as few functions for the plugins as possible while providing a rich API that can be extended in the future. The main idea behind the the API is that the ProDBG will request and send information to each plugin. This information is being sent/transfered with the `update` function 

Let's start with implementing a plugin which is an [6502](http://en.wikipedia.org/wiki/MOS_Technology_6502) Debugger

    void* (*createInstance)(PDServiceFuncs* serviceFuncs);

This is being called when a new session (TODO: link to session) for a given plugin type. Typically, you'll use malloc to allocate memory for a data structure, fill in some of the structure's fields with default values, and return the pointer to this structure. If you can't create an instance return NULL.
ServiceFuncs (TODO: add link) will not be described in this example.

    void (*destroyInstance)(void* userData);

This function will be called when an session is closed and the userData pointer is the same as returned in `createInstance`

    PDDebugState (*update)(void* userData, PDAction action, PDReader* inEvents, PDWriter* outEvents);

This is the main function of the debugger API. Here ProDBG will request data from the plugin and the plugin can also send back to tell ProDBG about various data/state of the plugin.

Here is some example code of how it can look

    static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
    {
        int event = 0;
        MyData* debugger = (MyData*)userData;

        // Actions are simple events sent from the debugger that can be break, run, step, etc

        switch (action)
        {
            case PDAction_break : // tell the plugin to break; 
            case PDAction_run : // tell the plugin to break; 
            case PDAction_step : // tell the plugin to break; 
        }

        // This is the main 

        while ((event = PDRead_getEvent(reader)) != 0)
        {
            switch (event)
            {
                case PDEventType_getDisassembly :
                {
                    char temp[65536];

                    // Get disassembly for the plugin and write it back

                    disassembleToBuffer(temp, &start, &instCount);

                    PDWrite_eventBegin(writer, PDEventType_setDisassembly);
                    PDWrite_u16(writer, "address_start", (uint16_t)start);
                    PDWrite_u16(writer, "instruction_count", (uint16_t)instCount);
                    PDWrite_string(writer, "string_buffer", temp);
                    PDWrite_eventEnd(writer);
                }
            }
        }

        return debugger->runState;
    }

The plugin is responsible to send back data ProDBG requests. In some cases the back-end may not provide certain information (disassembly in a scripting lang for example) and can the chose just to ignore that. In the example above ProDBG requests Disassembly to be sent back and that usually happens when the user opens a view associated with a certain type.
A list of of data that ProDBG can request and the expected reply can be found here (TODO: fix link)
