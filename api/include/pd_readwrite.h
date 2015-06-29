#ifndef PDREADWRITE_H_
#define PDREADWRITE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t PDReaderIterator;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDReadType
{
    /// Default type that represnts no type
    PDReadType_none,
    /// signed 8 bit value
    PDReadType_s8,
    /// unsigned 8 bit value
    PDReadType_u8,
    /// signed 16 bit value
    PDReadType_s16,
    /// unsigned 16 bit value
    PDReadType_u16,
    /// signed 32 bit value
    PDReadType_s32,
    /// unsigned 32 bit value
    PDReadType_u32,
    /// signed 64 bit value,
    PDReadType_s64,
    /// unsigned 64 bit value
    PDReadType_u64,
    /// 32 bit floating point value
    PDReadType_float,
    /// 64 bit floating point value
    PDReadType_double,
    // End of the numberic types
    PDReadType_endNumericTypes,
    /// const char* string (null terminated)
    PDReadType_string,
    /// data array (void*)
    PDReadType_data,
    /// EventType
    PDReadType_event,
    /// Array type
    PDReadType_array,
    /// Array type
    PDReadType_arrayEntry,
    /// total count of types
    PDReadType_count
} PDReadType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDReadStatus
{
    // Read operation completed without any problems
    PDReadStatus_ok = (1 << 8),
    // Read operation completed but value was converted (for example u16 -> u8 which may cause issues)
    PDReadStatus_converted = (2 << 8),
    // Read operation falied due to illeal type (example when reading s8 and type is string)
    PDReadStatus_illegalType = (3 << 8),
    // Read operation failed because the ID supplied to search function couldn't be found
    PDReadStatus_notFound = (4 << 8),
    // Read operation failed (generic error)
    PDReadStatus_fail = (5 << 8),
    // Mask used to get the type
    PDReadStatus_typeMask = 0xff
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDWriteStatus
{
    // Returns if a write operation completed ok
    PDWriteStatus_ok,
    // Generic error if write operation failed
    PDWriteStatus_fail

} PDWriteStatus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDWriter
{
    /**
     * Private data used by the writer
     **/
    void* data;

    /**
     * This starts to write an event. An event is usually of the type PDEventType that can be
     * used to compunicate back to the debugger. There is usually (but not always) a pair
     * of get/setEvents and it's the plugins resposibilty to reply back with data when
     * there is a request from the debugger
     *
     * @param writer writer object.
     * @param event Id of the event. This usually a PDEventType
     *
     * \code
     * PDWrite_eventBegin(writer, PDEvent_setBreakpoint);
     * ...
     * PDWrite_eventEnd();
     * \endcode
     */
    PDWriteStatus (*writeEventBegin)(struct PDWriter* writer, uint16_t event);

    /**
     * This ends an event. Notice that PDWrite_beginEvent needs to be started first
     *
     * @param writer writer object.
     *
     * \code
     * PDWrite_eventBegin(writer, PDEvent_setBreakpoint);
     * ...
     * PDWrite_eventEnd();
     * \endcode
     */
    PDWriteStatus (*writeEventEnd)(struct PDWriter* writer);

    /**
     *
     * NOTICE THAT THIS IS NOT IMPLEMENTED YET.
     *
     * Begins an table with a predefined structure. This is useful when writing
     * a table where all the entries are the same all the time. So in order to save both
     * CPU time and bandwith it's possible to begin an array with a fixed number of slots for
     * each entry.
     *
     * \warning
     * It's worth note when using this minimal error checking will be done when writing
     * the remining value inside the array.
     *
     * \note
     * If you are unsure about this it's better to use the regular PDWriter::writeBeginArray
     * instead which is more flexible.
     *
     * @param write writer object.
     * @param ids a list of Ids that is terminated by a null string.
     *
     * \code
     *
     * static const char* ids[] =
     * {
     *   "address",
     *   "code",
     *   0,
     * };
     *
     * ...
     *
     * PDWrite_headerArrayBegin(writer, ids);
     *
     * for (i to addressCount)
     * {
     *    PDWrite_writeU32(writer, 0, address[i]);
     *    PDWrite_writeString(writer, 0, codes[i]);
     * }
     *
     * PDWrite_headerArrayEnd(writer);
     *
     * \endcode
     *
     */
    PDWriteStatus (*writeHeaderArrayBegin)(struct PDWriter* writer, const char** ids);

    /**
     *
     * Ends writing of a predefined structure. See PDWriter::writeHeaderArrayBegin for more info
     *
     * @param write writer object.
     *
     */
    PDWriteStatus (*writeHeaderArrayEnd)(struct PDWriter* writer);

    /**
     *
     * Starts to write an array to the writer. Arrays can contanin any number of entries but they must be wrapped
     * inside a PDWriter::writeArrayEntryBegin and PDWriter::writeArrayEntryEnd
     *
     * @param writer writer object.
     * @param name Optional name of the array
     *
     * \code
     *
     * PDWrite_arrayBegin(writer, "items");
     *
     * /// Here we need to start with an arrayEntryBegin / arrayEntryEnd (notice that the amount of entries between begin/end can be variable)
     *
     * for (i to itemCount)
     * {
     *    PDWrite_arrayEntryBegin(writer);
     *    PDWrite_string(writer, "test", "some test data"
     *    PDWrite_string(writer, "more_test", "and even more test data"
     *    PDWrite_arrayEntryEnd(writer);
     * }
     *
     * PDWrite_arrayEnd(writer);
     *
     * \endcode
     *
     * Output
     *
     * \code
     *
     * {
     *    {
     *      "test":"some test data",
     *      "more_test":"some test data",
     *    }
     *
     *    {
     *      "test":"some test data",
     *      "more_test":"some test data",
     *    }
     *    ....
     * }
     *
     * \endcode
     *
     */
    PDWriteStatus (*writeArrayBegin)(struct PDWriter* writer, const char* name);

    /**
     *
     * Ends an array. See PDWrite::writeArrayBegin for more info
     *
     * @param writer writer object.
     *
     */
    PDWriteStatus (*writeArrayEnd)(struct PDWriter* writer);

    /**
     *
     * Starts an entry when writing to a table. See example for PDWrite::writeArrayBegin how this works.
     *
     * @param writer writer object.
     *
     */
    PDWriteStatus (*writeArrayEntryBegin)(struct PDWriter* writer);

    /**
     *
     * Ends the writing of an entry. See PDWrite::writeArrayBegin for example
     *
     * @param writer writer object.
     *
     */
    PDWriteStatus (*writeArrayEntryEnd)(struct PDWriter* writer);

    /**
     *
     * Writes an signed 8 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeS8)(struct PDWriter* writer, const char* id, int8_t v);

    /**
     *
     * Writes an unsigned 8 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeU8)(struct PDWriter* writer, const char* id, uint8_t v);

    /**
     *
     * Writes an signed 16 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeS16)(struct PDWriter* writer, const char* id, int16_t v);

    /**
     *
     * Writes an unsigned 16 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeU16)(struct PDWriter* writer, const char* id, uint16_t v);

    /**
     *
     * Writes an signed 32 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeS32)(struct PDWriter* writer, const char* id, int32_t v);

    /**
     *
     * Writes an unsigned 32 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeU32)(struct PDWriter* writer, const char* id, uint32_t v);

    /**
     *
     * Writes an signed 64 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeS64)(struct PDWriter* writer, const char* id, int64_t v);

    /**
     *
     * Writes an unsigned 64 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeU64)(struct PDWriter* writer, const char* id, uint64_t v);

    /**
     *
     * Writes an 32 bit floating point value the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeFloat)(struct PDWriter* writer, const char* id, float v);

    /**
     *
     * Writes an 64 bit floating point value the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeDouble)(struct PDWriter* writer, const char* id, double v);

    /**
     *
     * Writes an null terminated string to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param v value to write
     *
     */
    PDWriteStatus (*writeString)(struct PDWriter* writer, const char* id, const char* v);

    /**
     *
     * Writes an array of data to the writer
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
     * @param data array of data to write
     * @param len size in bytes of how much to write
     *
     */
    PDWriteStatus (*writeData)(struct PDWriter* writer, const char* id, void* data, unsigned int len);

} PDWriter;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDReader
{
    /**
     * Private data for the reader
     */
    void* data;

    /**
     *
     * When events are being sent to a plugin its possible to send more than one
     * at the same time. This means it must be possible to iterate over them and also possible
     * to skip the ones that the plugin may not support. The code will handle tracking of where next
     * event will begin and return PDEventType_none (0) when there are no more events
     *
     * @param reader The reader object.
     * @return The event type and is usually a PDEventType but pluins can use custom events as well
     *
     * \code
     *
     * u16_t event;
     *
     * while ((event = PDRead_getEvent(reader)))
     * {
     *    switch (event)
     *    {
     *       case PDEvent_setBreakpoint : ....
     *       case PDEvent_setExecutable : ....
     *    }
     * }
     * \endcode
     */
    uint32_t (*readGetEvent)(struct PDReader* reader);

    /**
     *
     * Get the next event. See FDRead::readIteratorGetEvent for details and example usage
     *
     * @param reader The reader object.
     * @param it The iterator (initialize with PDRead_iteratorGetEvent)
     * @return The event type and is usually a PDEventType but pluins can use custom events as well
     *
     */
    uint32_t (*readIteratorNextEvent)(struct PDReader* reader, PDReaderIterator* it);

    /**
     *
     * Used to iterater over data within a scope. When accessing data within a a scope there are two
     * ways of doing it and one way is to use iterators and use the corresponing read operation and the other
     * is to use one of the find functions (detailed futher down)
     *
     * @param reader The reader object
     * @param it iterator that will be using when traversing the data
     * @param keyName name of the key for the current key/value
     * @param parentIt The parent iterator (may be the eventIt or an array iterator for example)
     *
     * \note This example code assumes that eventIt (as seen in the PDRead::readIteratorGetEvent example)
     *       has been initialized correctly
     *
     * \code
     *
     * PDReadType type = PDRead_iteratorBegin(reader, &it, &keyName, eventIt);
     *
     * while (type)
     * {
     *    switch (type)
     *    {
     *       case PDReadType_s8:
     *       {
     *          const char* name;
     *          int8_t v = PDRead_s8(reader, &name, it);
     *          printf("key: %s value: %d\n", name, v);
     *       }
     *
     *    }
     *
     *    type = PDRead_iteratorNext(reader, it);
     * }
     * \endcode
     *
     */
    uint32_t (*readIteratorBegin)(struct PDReader* reader, PDReaderIterator* it, const char** keyName, PDReaderIterator parentIt);

    /**
     *
     * Increase the iterator to the next value. See PDRead::readIteratorBegin for example usage.
     *
     * @param reader The reader
     * @param keyName Name of the key for the current key/value
     * @param it The iterator
     * @return the type of the next value and returns PDReadType_none if no more values
     *
     */
    uint32_t (*readIteratorNext)(struct PDReader* reader, const char** keyName, PDReaderIterator* it);

    /**
     *
     *
     *
     *
     *
     */
    int32_t (*readNextEntry)(struct PDReader* reader, PDReaderIterator* arrayIt);

    /** @name Find functions
     *
     * All of them work in the same way except they have different types that they write back.
     *
     * Find the id within the current scope and set the res variable to the (converted value)
     * It's worth to note that this function will only search within the current scope of
     * the iterator (meaning if there are arrays and arrays of arrays) this function will not return those
     *
     * @param reader The reader object.
     * @param res Result in the given type (s8/u16/etc) if the function completed successfully.
     * @param id string of the identifier to search for
     * @return Lower 8-bit is the type of the the value in the struture (meaning that if you do a readFindS8 the value
     *         in the reader may be of a different type) the upper 8-bit describes if the read operator was successful or not
     *         See PDReadStatus for a complete list of codes
     *
     * \code
     * This code shows what happens when trying to read s8 value but the value is actually a
     * string and how you can handle the case be simply doing a readString instead
     *
     * if ((status = PDRead_findS8(reader, &value, "foo", it) >> 8) == PDReadStatus_illegalType)
     * {
     *     type = status & PDReadWrite_TypeMask;
     *
     *        if (type == PDReadWiteType_string)
     *        {
     *          PDRead_readString(reader, &stringVal, it);
     *     }
     * }
     * \endcode
     *
     */
    ///@{
    uint32_t (*readFindS8)(struct PDReader* reader, int8_t* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindU8)(struct PDReader* reader, uint8_t* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindS16)(struct PDReader* reader, int16_t* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindU16)(struct PDReader* reader, uint16_t* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindS32)(struct PDReader* reader, int32_t* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindU32)(struct PDReader* reader, uint32_t* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindS64)(struct PDReader* reader, int64_t* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindU64)(struct PDReader* reader, uint64_t* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindFloat)(struct PDReader* reader, float* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindDouble)(struct PDReader* reader, double* res, const char* id, PDReaderIterator it);
    uint32_t (*readFindString)(struct PDReader* reader, const char** res, const char* id, PDReaderIterator it);
    uint32_t (*readFindData)(struct PDReader* reader, void** data, uint64_t* size, const char* id, PDReaderIterator it);
    uint32_t (*readFindArray)(struct PDReader* reader, PDReaderIterator* arrayIt, const char* id, PDReaderIterator it);
    ///@}

    /**
     *
     * Dumps the whole tree to a human-friendly readable format
     *
     *
     */
    void (*readDumpData)(struct PDReader* reader);

} PDReader;


/**
 *
 * Macros to make Write functions easier to use
 *
 */

#define PDWrite_eventBegin(w, e) w->writeEventBegin(w, e)
#define PDWrite_eventEnd(w) w->writeEventEnd(w)
#define PDWrite_headerArrayBegin(w, ids) w->writeHeaderArrayBegin(w, ids)
#define PDWrite_headerArrayEnd(w) w->writeHeaderArrayEnd(w)
#define PDWrite_arrayBegin(w, name) w->writeArrayBegin(w, name)
#define PDWrite_arrayEnd(w) w->writeArrayEnd(w)
#define PDWrite_arrayEntryBegin(w) w->writeArrayEntryBegin(w)
#define PDWrite_arrayEntryEnd(w) w->writeArrayEntryEnd(w)
#define PDWrite_s8(w, id, v) w->writeS8(w, id, v)
#define PDWrite_u8(w, id, v) w->writeU8(w, id, v)
#define PDWrite_s16(w, id, v) w->writeS16(w, id, v)
#define PDWrite_u16(w, id, v) w->writeU16(w, id, v)
#define PDWrite_s32(w, id, v) w->writeS32(w, id, v)
#define PDWrite_u32(w, id, v) w->writeU32(w, id, v)
#define PDWrite_s64(w, id, v) w->writeS64(w, id, v)
#define PDWrite_u64(w, id, v) w->writeU64(w, id, v)
#define PDWrite_float(w, id, v) w->writeFloat(w, id, v)
#define PDWrite_double(w, id, v) w->writeDouble(w, id, v)
#define PDWrite_string(w, id, v) w->writeString(w, id, v)
#define PDWrite_data(w, id, data, len) w->writeData(w, id, data, len)

/**
 *
 * Macros to make Read functions easier to use
 *
 */

#define PDRead_getEvent(r) r->readGetEvent(r)
#define PDRead_getNextEntry(r, it) r->readNextEntry(r, it)
#define PDRead_iteratorBegin(r, it, keyName, parentIt) r->readIteratorBegin(r, it, keyName, parentIt)
#define PDRead_iteratorNext(r, keyName, it) r->readIteratorNext(r, keyName, it)
#define PDRead_findS8(r, res, id, it) r->readFindS8(r, res, id, it)
#define PDRead_findU8(r, res, id, it) r->readFindU8(r, res, id, it)
#define PDRead_findS16(r, res, id, it) r->readFindS16(r, res, id, it)
#define PDRead_findU16(r, res, id, it) r->readFindU16(r, res, id, it)
#define PDRead_findS32(r, res, id, it) r->readFindS32(r, res, id, it)
#define PDRead_findU32(r, res, id, it) r->readFindU32(r, res, id, it)
#define PDRead_findS64(r, res, id, it) r->readFindS64(r, res, id, it)
#define PDRead_findU64(r, res, id, it) r->readFindU64(r, res, id, it)
#define PDRead_findFloat(r, res, id, it) r->readFindFloat(r, res, id, it)
#define PDRead_findDouble(r, res, id, it) r->readFindDouble(r, res, id, it)
#define PDRead_findString(r, res, id, it) r->readFindString(r, res, id, it)
#define PDRead_findData(r, res, size, id, it) r->readFindData(r, res, size, id, it)
#define PDRead_findArray(r, arrayIt, id, it) r->readFindArray(r, arrayIt, id, it)
#define PDRead_dumpData(r) r->readDumpData(r)

#ifdef __cplusplus
}
#endif

#endif

