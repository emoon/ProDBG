#ifndef PDREADWRITE_H_
#define PDREADWRITE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t PDReaderIterator;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDReadType {
    /// Default type that represnts no type
    PDReadType_None,
    /// signed 8 bit value
    PDReadType_S8,
    /// unsigned 8 bit value
    PDReadType_U8,
    /// signed 16 bit value
    PDReadType_S16,
    /// unsigned 16 bit value
    PDReadType_U16,
    /// signed 32 bit value
    PDReadType_S32,
    /// unsigned 32 bit value
    PDReadType_U32,
    /// signed 64 bit value,
    PDReadType_S64,
    /// unsigned 64 bit value
    PDReadType_U64,
    /// 32 bit floating point value
    PDReadType_Float,
    /// 64 bit floating point value
    PDReadType_Double,
    // End of the numberic types
    PDReadType_EndNumericTypes,
    /// const char* string (null terminated)
    PDReadType_String,
    /// data array (void*)
    PDReadType_Data,
    /// EventType
    PDReadType_Event,
    /// Array type
    PDReadType_Array,
    /// Array type
    PDReadType_ArrayEntry,
    /// total count of types
    PDReadType_Count
} PDReadType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDReadStatus {
    // Read operation completed without any problems
    PDReadStatus_Ok = (1 << 8),
    // Read operation completed but value was converted (for example u16 -> u8 which may cause issues)
    PDReadStatus_Converted = (2 << 8),
    // Read operation falied due to illeal type (example when reading s8 and type is string)
    PDReadStatus_IllegalType = (3 << 8),
    // Read operation failed because the ID supplied to search function couldn't be found
    PDReadStatus_NotFound = (4 << 8),
    // Read operation failed (generic error)
    PDReadStatus_Fail = (5 << 8),
    // Mask used to get the type
    PDReadStatus_TypeMask = 0xff
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDWriteStatus {
    // Returns if a write operation completed ok
    PDWriteStatus_ok,
    // Generic error if write operation failed
    PDWriteStatus_Fail

} PDWriteStatus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDWriter {
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
     * PDWrite_event_begin(writer, PDEvent_setBreakpoint);
     * ...
     * PDWrite_event_end();
     * \endcode
     */
    PDWriteStatus (*write_event_begin)(struct PDWriter* writer, uint16_t event);

    /**
     * This ends an event. Notice that PDWrite_beginEvent needs to be started first
     *
     * @param writer writer object.
     *
     * \code
     * PDWrite_event_begin(writer, PDEvent_setBreakpoint);
     * ...
     * PDWrite_event_end();
     * \endcode
     */
    PDWriteStatus (*write_event_end)(struct PDWriter* writer);

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
     * PDWrite_header_array_begin(writer, ids);
     *
     * for (i to addressCount)
     * {
     *    PDWrite_write_u32(writer, 0, address[i]);
     *    PDWrite_write_string(writer, 0, codes[i]);
     * }
     *
     * PDWrite_header_array_end(writer);
     *
     * \endcode
     *
     */
    PDWriteStatus (*write_header_array_begin)(struct PDWriter* writer, const char** ids);

    /**
     *
     * Ends writing of a predefined structure. See PDWriter::write_header_array_begin for more info
     *
     * @param write writer object.
     *
     */
    PDWriteStatus (*write_header_array_end)(struct PDWriter* writer);

    /**
     *
     * Starts to write an array to the writer. Arrays can contanin any number of entries but they must be wrapped
     * inside a PDWriter::write_array_entry_begin and PDWriter::write_array_entry_end
     *
     * @param writer writer object.
     * @param name Optional name of the array
     *
     * \code
     *
     * PDWrite_array_begin(writer, "items");
     *
     * /// Here we need to start with an arrayEntryBegin / arrayEntryEnd (notice that the amount of entries between begin/end can be variable)
     *
     * for (i to itemCount)
     * {
     *    PDWrite_array_entry_begin(writer);
     *    PDWrite_string(writer, "test", "some test data"
     *    PDWrite_string(writer, "more_test", "and even more test data"
     *    PDWrite_entry_end(writer);
     * }
     *
     * PDWrite_array_end(writer);
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
    PDWriteStatus (*write_array_begin)(struct PDWriter* writer, const char* name);

    /**
     *
     * Ends an array. See PDWrite::write_array_begin for more info
     *
     * @param writer writer object.
     *
     */
    PDWriteStatus (*write_array_end)(struct PDWriter* writer);

    /**
     *
     * Starts an entry when writing to a table. See example for PDWrite::write_array_begin how this works.
     *
     * @param writer writer object.
     *
     */
    PDWriteStatus (*write_array_entry_begin)(struct PDWriter* writer);

    /**
     *
     * Ends the writing of an entry. See PDWrite::write_array_begin for example
     *
     * @param writer writer object.
     *
     */
    PDWriteStatus (*write_array_entry_end)(struct PDWriter* writer);

    /**
     *
     * Writes an signed 8 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_s8)(struct PDWriter* writer, const char* id, int8_t v);

    /**
     *
     * Writes an unsigned 8 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_u8)(struct PDWriter* writer, const char* id, uint8_t v);

    /**
     *
     * Writes an signed 16 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_s16)(struct PDWriter* writer, const char* id, int16_t v);

    /**
     *
     * Writes an unsigned 16 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_u16)(struct PDWriter* writer, const char* id, uint16_t v);

    /**
     *
     * Writes an signed 32 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_s32)(struct PDWriter* writer, const char* id, int32_t v);

    /**
     *
     * Writes an unsigned 32 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_u32)(struct PDWriter* writer, const char* id, uint32_t v);

    /**
     *
     * Writes an signed 64 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_s64)(struct PDWriter* writer, const char* id, int64_t v);

    /**
     *
     * Writes an unsigned 64 bit value to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_u64)(struct PDWriter* writer, const char* id, uint64_t v);

    /**
     *
     * Writes an 32 bit floating point value the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_float)(struct PDWriter* writer, const char* id, float v);

    /**
     *
     * Writes an 64 bit floating point value the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_double)(struct PDWriter* writer, const char* id, double v);

    /**
     *
     * Writes an null terminated string to the writer.
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param v value to write
     *
     */
    PDWriteStatus (*write_string)(struct PDWriter* writer, const char* id, const char* v);

    /**
     *
     * Writes an array of data to the writer
     *
     * @param writer writer object
     * @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::write_header_array_begin
     * @param data array of data to write
     * @param len size in bytes of how much to write
     *
     */
    PDWriteStatus (*write_data)(struct PDWriter* writer, const char* id, void* data, unsigned int len);

} PDWriter;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDReader {
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
     * while ((event = PDRead_get_event(reader)))
     * {
     *    switch (event)
     *    {
     *       case PDEvent_setBreakpoint : ....
     *       case PDEvent_setExecutable : ....
     *    }
     * }
     * \endcode
     */
    uint32_t (*read_get_event)(struct PDReader* reader);

    /**
     *
     * Get the next event. See FDRead::readIteratorGetEvent for details and example usage
     *
     * @param reader The reader object.
     * @param it The iterator (initialize with PDRead_iteratorGetEvent)
     * @return The event type and is usually a PDEventType but pluins can use custom events as well
     *
     */
    uint32_t (*read_iterator_next_event)(struct PDReader* reader, PDReaderIterator* it);

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
     * PDReadType type = PDRead_iterator_begin(reader, &it, &keyName, eventIt);
     *
     * while (type)
     * {
     *    switch (type)
     *    {
     *       case PDReadType_S8:
     *       {
     *          const char* name;
     *          int8_t v = PDRead_s8(reader, &name, it);
     *          printf("key: %s value: %d\n", name, v);
     *       }
     *
     *    }
     *
     *    type = PDRead_iterator_Next(reader, it);
     * }
     * \endcode
     *
     */
    uint32_t (*read_iterator_begin)(struct PDReader* reader, PDReaderIterator* it, const char** keyName, PDReaderIterator parentIt);

    /**
     *
     * Increase the iterator to the next value. See PDRead::read_iterator_begin for example usage.
     *
     * @param reader The reader
     * @param keyName Name of the key for the current key/value
     * @param it The iterator
     * @return the type of the next value and returns PDReadType_None if no more values
     *
     */
    uint32_t (*read_iterator_next)(struct PDReader* reader, const char** keyName, PDReaderIterator* it);

    /**
     *
     *
     *
     *
     *
     */
    int32_t (*read_next_entry)(struct PDReader* reader, PDReaderIterator* arrayIt);

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
     * @return Lower 8-bit is the type of the the value in the struture (meaning that if you do a read_find_s8 the value
     *         in the reader may be of a different type) the upper 8-bit describes if the read operator was successful or not
     *         See PDReadStatus for a complete list of codes
     *
     * \code
     * This code shows what happens when trying to read s8 value but the value is actually a
     * string and how you can handle the case be simply doing a readString instead
     *
     * if ((status = PDRead_find_s8(reader, &value, "foo", it) >> 8) == PDReadStatus_IllegalType)
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
    uint32_t (*read_find_s8)(struct PDReader* reader, int8_t* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_u8)(struct PDReader* reader, uint8_t* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_s16)(struct PDReader* reader, int16_t* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_u16)(struct PDReader* reader, uint16_t* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_s32)(struct PDReader* reader, int32_t* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_u32)(struct PDReader* reader, uint32_t* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_s64)(struct PDReader* reader, int64_t* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_u64)(struct PDReader* reader, uint64_t* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_float)(struct PDReader* reader, float* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_double)(struct PDReader* reader, double* res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_string)(struct PDReader* reader, const char** res, const char* id, PDReaderIterator it);
    uint32_t (*read_find_data)(struct PDReader* reader, void** data, uint64_t* size, const char* id, PDReaderIterator it);
    uint32_t (*read_find_array)(struct PDReader* reader, PDReaderIterator* arrayIt, const char* id, PDReaderIterator it);
    ///@}

    /**
     *
     * Dumps the whole tree to a human-friendly readable format
     *
     *
     */
    void (*read_dump_data)(struct PDReader* reader);

} PDReader;


/**
 *
 * Macros to make Write functions easier to use
 *
 */

#define PDWrite_event_begin(w, e) w->write_event_begin(w, e)
#define PDWrite_event_end(w) w->write_event_end(w)
#define PDWrite_header_array_begin(w, ids) w->write_header_array_begin(w, ids)
#define PDWrite_header_array_end(w) w->write_header_array_end(w)
#define PDWrite_array_begin(w, name) w->write_array_begin(w, name)
#define PDWrite_array_end(w) w->write_array_end(w)
#define PDWrite_array_entry_begin(w) w->write_array_entry_begin(w)
#define PDWrite_entry_end(w) w->write_array_entry_end(w)
#define PDWrite_s8(w, id, v) w->write_s8(w, id, v)
#define PDWrite_u8(w, id, v) w->write_u8(w, id, v)
#define PDWrite_s16(w, id, v) w->write_s16(w, id, v)
#define PDWrite_u16(w, id, v) w->write_u16(w, id, v)
#define PDWrite_s32(w, id, v) w->write_s32(w, id, v)
#define PDWrite_u32(w, id, v) w->write_u32(w, id, v)
#define PDWrite_s64(w, id, v) w->write_s64(w, id, v)
#define PDWrite_u64(w, id, v) w->write_u64(w, id, v)
#define PDWrite_float(w, id, v) w->write_float(w, id, v)
#define PDWrite_double(w, id, v) w->write_double(w, id, v)
#define PDWrite_string(w, id, v) w->write_string(w, id, v)
#define PDWrite_data(w, id, data, len) w->write_data(w, id, data, len)

/**
 *
 * Macros to make Read functions easier to use
 *
 */

#define PDRead_get_event(r) r->read_get_event(r)
#define PDRead_get_next_entry(r, it) r->read_next_entry(r, it)
#define PDRead_iterator_begin(r, it, keyName, parentIt) r->read_iterator_begin(r, it, keyName, parentIt)
#define PDRead_iterator_Next(r, keyName, it) r->read_iterator_next(r, keyName, it)
#define PDRead_find_s8(r, res, id, it) r->read_find_s8(r, res, id, it)
#define PDRead_find_u8(r, res, id, it) r->read_find_u8(r, res, id, it)
#define PDRead_find_s16(r, res, id, it) r->read_find_s16(r, res, id, it)
#define PDRead_find_u16(r, res, id, it) r->read_find_u16(r, res, id, it)
#define PDRead_find_s32(r, res, id, it) r->read_find_s32(r, res, id, it)
#define PDRead_find_u32(r, res, id, it) r->read_find_u32(r, res, id, it)
#define PDRead_find_s64(r, res, id, it) r->read_find_s64(r, res, id, it)
#define PDRead_find_u64(r, res, id, it) r->read_find_u64(r, res, id, it)
#define PDRead_find_float(r, res, id, it) r->read_find_float(r, res, id, it)
#define PDRead_find_double(r, res, id, it) r->read_find_double(r, res, id, it)
#define PDRead_find_string(r, res, id, it) r->read_find_string(r, res, id, it)
#define PDRead_find_data(r, res, size, id, it) r->read_find_data(r, res, size, id, it)
#define PDRead_find_array(r, arrayIt, id, it) r->read_find_array(r, arrayIt, id, it)
#define PDRead_dump_data(r) r->read_dump_data(r)

#ifdef __cplusplus
}
#endif

#endif

