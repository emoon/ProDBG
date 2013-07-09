#ifndef _PDREADWRITE_H_
#define _PDREADWRITE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef _cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDReadWriteType
{
	PDReadWriteType_s8,			/// signed 8 bit value
	PDReadWriteType_u8,			/// unsigned 8 bit value
	PDReadWriteType_s16,		/// signed 16 bit value
	PDReadWriteType_u16,		/// unsigned 16 bit value
	PDReadWriteType_s32,		/// signed 32 bit value
	PDReadWriteType_u32,		/// unsigned 32 bit value			
	PDReadWriteType_s64,		/// signed 64 bit value,
	PDReadWriteType_u64,		/// unsigned 64 bit value
	PDReadWriteType_float,		/// 32 bit floating point value 
	PDReadWriteType_double,		/// 64 bit floating point value 
	PDReadWriteType_string,		/// const char* string (null terminated) 
	PDReadWriteType_data,		/// data array (void*) 
	PDReadWriteType_event		/// Event that usually matches PDEventType 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDWriter
{
	/**
	 * Le private  data
	 *
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

struct PDReaderIterator;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDReader
{
	void* data;

	void (*readIteratorGetEvent)(struct PDReader* reader, struct PDReaderIterator* it);
	void (*readIteratorNextEvent)(struct PDReader* reader, struct PDReaderIterator* it);

	PDReadWriteType (*readIteratorBegin)(struct PDReader* reader, struct PDReaderIterator* it);
	PDReadWriteType (*readIteratorNext)(struct PDReader* reader, struct PDReaderIterator* it);

	// find functions

	int8_t (*readFindS8)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	uint8_t (*readFindU8)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	int16_t (*readFindS16)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	uint16_t (*readFindU16)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	int32_t (*readFindS32)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	uint32_t (*readFindU32)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	int64_t (*readFindS64)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	uint64_t (*readFindU64)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	float (*readFindFloat)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	double (*readFindDouble)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	const char* (*readFindString)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	void* (*readFindData)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);

	struct PDReaderIterator* (*readFindArray)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	struct PDReaderIterator* (*readNextArray)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);

	int8_t (*readS8)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	uint8_t (*readU8)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	int16_t (*readS16)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	uint16_t (*readU16)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	int32_t (*readS32)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	uint32_t (*readU32)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	int64_t (*readS64)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	uint64_t (*readU64)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	float (*readFloat)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	double (*readDouble)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	const char* (*readString)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);
	void* (*readData)(struct PDReader* reader, const char* id, struct PDReaderIterator* it);

} PDReader;

#ifdef _cplusplus
}
#endif

#endif

