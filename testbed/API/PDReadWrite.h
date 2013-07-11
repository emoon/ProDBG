#ifndef _PDREADWRITE_H_
#define _PDREADWRITE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef _cplusplus
extern "C" {
#endif

struct PDReaderIterator;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDReadWriteType
{
	/// Default type that represnts no type 
	PDReadWriteType_none,			
	/// signed 8 bit value
	PDReadWriteType_s8,			
	/// unsigned 8 bit value
	PDReadWriteType_u8,			
	/// signed 16 bit value
	PDReadWriteType_s16,		
	/// unsigned 16 bit value
	PDReadWriteType_u16,		
	/// signed 32 bit value
	PDReadWriteType_s32,		
	/// unsigned 32 bit value			
	PDReadWriteType_u32,		
	/// signed 64 bit value,
	PDReadWriteType_s64,		
	/// unsigned 64 bit value
	PDReadWriteType_u64,		
	/// 32 bit floating point value 
	PDReadWriteType_float,		
	/// 64 bit floating point value 
	PDReadWriteType_double,		
	/// const char* string (null terminated) 
	PDReadWriteType_string,		
	/// data array (void*) 
	PDReadWriteType_data
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDReadStatus
{
	// Read operation completed without any problems
	PDReadStatus_ok = (1 << 8),
	// Read operation completed but value was converted (for example u16 -> u8 which may cause issues)
	PDReadStatus_converted,
	// Read operation falied due to illeal type (example when reading s8 and type is string) 
	PDReadStatus_illegalType,
	// Read operation failed because the ID supplied to search function couldn't be found 
	PDReadStatus_notFound,
	// Mask used to get the type
	PDReadWrite_typeMask = 0xff
};

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
	void (*writeEventBegin)(struct PDWriter* writer, int event);

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
	void (*writeEventEnd)(struct PDWriter* writer);

   /**
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
	void (*writeHeaderArrayBegin)(struct PDWriter* writer, const char** ids);

   /**
	*
	* Ends writing of a predefined structure. See PDWriter::writeHeaderArrayBegin for more info
	*
	* @param write writer object.
	*
	*/
	void (*writeHeaderArrayEnd)(struct PDWriter* writer);

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
	* [ 
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
	* ] 
	*
	* \endocde
	*
	*/
	void (*writeArrayBegin)(struct PDWriter* writer, const char* name);

   /**
	*
	* Ends an array. See PDWrite::writeArrayBegin for more info
	*
	* @param writer writer object.
	*
	*/
	void (*writeArrayEnd)(struct PDWriter* writer);

   /**
	*
	* Starts an entry when writing to a table. See example for PDWrite::writeArrayBegin how this works.
	*
	* @param writer writer object.
	*
	*/
	void (*writeArrayEntryBegin)(struct PDWriter* writer);

   /**
	*
	* Ends the writing of an entry. See PDWrite::writeArrayBegin for example 
	*
	* @param writer writer object.
	*
	*/
	void (*writeArrayEntryEnd)(struct PDWriter* writer);

   /**
	* 
	* Writes an signed 8 bit value to the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeS8)(struct PDWriter* writer, const char* id, int8_t v);

   /**
	* 
	* Writes an unsigned 8 bit value to the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeU8)(struct PDWriter* writer, const char* id, uint8_t v);

   /**
	* 
	* Writes an signed 16 bit value to the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeS16)(struct PDWriter* writer, const char* id, int16_t v);

   /**
	* 
	* Writes an unsigned 16 bit value to the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeU16)(struct PDWriter* writer, const char* id, uint16_t v);

   /**
	* 
	* Writes an signed 32 bit value to the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeS32)(struct PDWriter* writer, const char* id, int32_t v);

   /**
	* 
	* Writes an unsigned 32 bit value to the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeU32)(struct PDWriter* writer, const char* id, uint32_t v);

   /**
	* 
	* Writes an signed 64 bit value to the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeS64)(struct PDWriter* writer, const char* id, int64_t v);

   /**
	* 
	* Writes an unsigned 64 bit value to the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeU64)(struct PDWriter* writer, const char* id, uint64_t v);

   /**
	* 
	* Writes an 32 bit floating point value the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeFloat)(struct PDWriter* writer, const char* id, float v);

   /**
	* 
	* Writes an 64 bit floating point value the writer.
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeDouble)(struct PDWriter* writer, const char* id, double v);

   /**
	* 
	* Writes an null terminated string to the writer. 
	*
	* @param writer writer object
	* @param id key to associate the value with and must be non-NULL unless inside a writerHeaderScope. See PDWriter::writeHeaderArrayBegin
	* @param v value to write
	*
	*/
	void (*writeString)(struct PDWriter* writer, const char* id, const char* v);

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
	void (*writeData)(struct PDWriter* writer, const char* id, void* data, size_t len);

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
    * to skip the ones that the plugin may not support. Iterators are used for this and by calling GetEvent and then
    * inside a loop call NextEvent its possible to iterate over all the events
    *
    * @param reader The reader object.
    * @param it This is the iterator that will be used when iterating over the events. Notice
    *           that the iterator can not be changed by the user and is for internal use only by the system
    * @return The event type and is usually a PDEventType but pluins can use custom events as well
    *
    * \code
    * struct PDReadIterator* eventIt;
    *
    * int event = PDRead_iteratorGetEvent(reader, &eventIt);
    *
    * while (event)
    * {
    *    switch (event)
    *    {
    *       case PDEvent_setBreakpoint : ....
    *       case PDEvent_setExecutable : ....
    *    }
    *
    *    event = PDRead_iteratorGetNextEvent(reader, eventIt);
    * }
    * \endcode
    */
	int (*readIteratorBeginEvent)(struct PDReader* reader, struct PDReadIterator** it);

   /**
    *
    * Get the next event. See FDRead::readIteratorGetEvent for details and example usage
    *
    * @param reader The reader object.
    * @param it The iterator (initialize with PDRead_iteratorGetEvent)
    * @return The event type and is usually a PDEventType but pluins can use custom events as well
    *
    */
	int (*readIteratorNextEvent)(struct PDReader* reader, struct PDReadIterator* it);

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
    * PDReadWriteType type = PDRead_iteratorBegin(reader, &it, &keyName, eventIt);
    *
    * while (type)
    * {
    *    switch (type)
    *    {
    *       case PDReadWriteType_s8:
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
	PDReadWriteType (*readIteratorBegin)(struct PDReader* reader, struct PDReaderIterator** it, const char** keyName, struct PDReaderIterator* parentIt);

   /**
    *
    * Increase the iterator to the next value. See PDRead::readIteratorBegin for example usage. 
    *
    * @param reader The reader
    * @param keyName Name of the key for the current key/value
    * @param it The iterator
    * @return the type of the next value and returns PDReadWriteType_none if no more values
    *
    */
	PDReadWriteType (*readIteratorNext)(struct PDReader* reader, const char** keyName, struct PDReaderIterator* it);

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
    * @return Lower 8-bit is the type of the the value in the struture (meaning that if you do a readS8 the value
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
	*  	  if (type == PDReadWiteType_string)
	*  	  {
	*          PDRead_readString(reader, &stringVal, it);
	*     }
	* }
    * \endcode
    *
    */
   ///@{
	PDReadStatus (*readFindS8)(struct PDReader* reader, int8_t* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindU8)(struct PDReader* reader, uint8_t* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindS16)(struct PDReader* reader, int16_t* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindU16)(struct PDReader* reader, uint16_t* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindS32)(struct PDReader* reader, int32_t* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindU32)(struct PDReader* reader, uint32_t* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindS64)(struct PDReader* reader, int64_t* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindU64)(struct PDReader* reader, uint64_t* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindFloat)(struct PDReader* reader, float* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindDouble)(struct PDReader* reader, double* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindString)(struct PDReader* reader, const char* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindData)(struct PDReader* reader, void* data, uint64_t* size, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readFindArray)(struct PDReader* reader, struct PDReaderIterater** arrayIt, const char* id, struct PDReaderIterator* it);
   ///@}

   /** @name Read functions
    *
    * These functions will read the data at the iterator to the res and id varibles. Just as in the finder functions
    * the code will try to convert the value if it can but will otherwise return an error code
    *
    * @param reader The reader object.
    * @param res Result in the given type (s8/u16/etc) if the function completed successfully.
    * @param id string of the identifier to search for the current iterator postion (can be NULL if output is ignored)
    * @return Lower 8-bit is the type of the the value in the struture (meaning that if you do a readS8 the value
    *         in the reader may be of a different type) the upper 8-bit describes if the read operator was successful or not
    *         See PDReadStatus for a complete list of codes
    *
    * \code
    * cons char* key;
    * This code shows what happens when trying to read s8 value but the value is actually a
    * string and how you can handle the case be simply doing a readString instead
    *
 	* if ((status = PDRead_s8(reader, &value, &key, it) >> 8) == PDReadStatus_illegalType)
	* {
	*     type = status & PDReadWrite_TypeMask;
	* 
	*  	  if (type == PDReadWiteType_string)
	*  	  {
	*          PDRead_string(reader, &stringVal, 0, it);
	*     }
	* }
    * \endcode
    */
   ///@{
	PDReadStatus (*readS8)(struct PDReader* reader, int8_t* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readU8)(struct PDReader* reader, uint8_t* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readS16)(struct PDReader* reader, int16_t* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readU16)(struct PDReader* reader, uint16_t* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readS32)(struct PDReader* reader, int32_t* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readU32)(struct PDReader* reader, uint32_t* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readS64)(struct PDReader* reader, int64_t* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readU64)(struct PDReader* reader, uint64_t* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readFloat)(struct PDReader* reader, float* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readDouble)(struct PDReader* reader, double* res, const char* id, struct PDReaderIterator* it);
	PDReadStatus (*readString)(struct PDReader* reader, const char* res, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readData)(struct PDReader* reader, void* res, uint64_t* size, const char** id, struct PDReaderIterator* it);
	PDReadStatus (*readArray)(struct PDReader* reader, struct PDReaderIterater** arrayIt, const char* id, struct PDReaderIterator* it);
   ///@}

} PDReader;

#ifdef _cplusplus
}
#endif

#endif

