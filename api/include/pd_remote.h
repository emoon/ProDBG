#ifndef _PRODBG_REMOTEAPI_H_
#define _PRODBG_REMOTEAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

struct PDBackendPlugin;

/**
 * \brief Create the listener that is used to connect to ProDBG
 *
 * This function creates the listener that is needed in order to connect
 * to ProDBG. This needs to be called before any debugging is possible.
 * This code will also verify that @p plugin is valid.
 *
 * \param plugin Pointer to a backend plugin. This needs to be filled in according to the doc of PDBackendPlugin
 * \param waitForConnection Number of seconds to wait for a connection from the Debugger. 0 if no waiting
 * \return returns 1 on success otherwise 0
 */

int PDRemote_create(struct PDBackendPlugin* plugin, int waitForConnection);

/**
 * \brief Updates the connection
 *
 * This function is required to be called now and then in your application as it keeps the connection with the
 * debugger alive (listening to incoming events, etc)
 * \param sleepTime optional amount of time to sleep in miliseconds (useful if called in a tight loop)
 *
 */

int PDRemote_update(int sleepTime);

/**
 * \brief Check connection status
 *
 * This can be used to reduce the number of calls to PDRemote_update
 *
 * Say we want to debug an emulated CPU but when the code is running as usually we dont
 * want to call PDRemote_update for each instruction (as it reads from socket and such and will have overhead)
 * In this case we might only want to read from it every n-instruction or in the main loop on the program
 *
 * But when we are actually connected it the code should call update for each instruction. Say that the code
 * you want to debug has ended up in an infinite loop and the only way the debugger can break then is
 * to be updated inside the loop
 *
 * It's all up to the implementer how this should be handled but this is the recommended way.
 *
 * \returns TRUE if connected otherwise FALSE
 *
 */

int PDRemote_isConnected();

/**
 * \brief Destroys the current connection and listener server.
 *
 * Closes down the listener socket and the connection socket (if open) to the debugger.
 * After calling this function no connection to the debugger will be possible unless PDRemote_create is called again
 *
 */

void PDRemote_destroy();

#ifdef __cplusplus
}
#endif

#endif

