/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Header: /Commando/Code/wwlib/chunkio.h 23    3/14/02 1:25p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Tiberian Sun / Commando / G Library                          *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/chunkio.h                              $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/04/02 3:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 23                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#pragma once

#include <cinttypes>
#include <SDL3/SDL_iostream.h>
#include "iostruct.h"


/************************************************************************************

	ChunkIO

	(gth) This module provides classes for reading and writing chunk-based files.
	For example, all of the w3d files are stored in a hierarchical-chunk format.
	Basically the format is similar to IFF.  All data in the file has chunk headers
	wrapped around it.  A chunk header contains an ID, and a Size.  The size
	is the number of bytes in the chunk (not including the header).  The
	contents of a chunk may be either: more "sub-chunks" or raw data.  These classes
	will automatically keep track of your positions within all of the sub and parent
	chunks (to some maximum recursion depth).

	Sept 3, 1999
	(gth) Adding the new concept of "micro-chunks".  Instead of filling the contents of a
	chunk with data, you can fill it with "micro-chunks" which contain a single byte
	id and a single byte size.  Micro-chunks are used for storing simple variables
	in a form that can survive revisions to the file format without paying the price
	for a full chunk header.  You CANNOT recursively embed micro-chunks due to their
	size limitations....

	Sept 24, 1999
	(gth) Using the MSB of the chunksize to indicate whether a chunk contains other
	chunks or pure data.  If the MSB is 0, the chunk contains data (so that the reader
	I'm going to write doesn't break on older files) and if it is 1 then it is
	assumed to contain other chunks.  This does not apply to micro-chunks as they
	are considered data.

**************************************************************************************/

struct ChunkHeader {
    // Functions.
    ChunkHeader() : m_chunk_type(0), m_chunk_size(0) {}

    ChunkHeader(uint32_t type, uint32_t size) {
        m_chunk_type = type;
        m_chunk_size = size;
    }

    // Use these accessors to ensure you correctly deal with the data in the chunk header
    void set_type(uint32_t type) { m_chunk_type = type; }

    uint32_t get_type(void) { return m_chunk_type; }

    void set_size(uint32_t size) {
        m_chunk_size &= 0x80000000;
        m_chunk_size |= (size & 0x7FFFFFFF);
    }

    void add_size(uint32_t add) { set_size(get_size() + add); }

    uint32_t get_size(void) { return (m_chunk_size & 0x7FFFFFFF); }

    void set_sub_chunk_flag(bool onoff) { if (onoff) { m_chunk_size |= 0x80000000; } else { m_chunk_size &= 0x7FFFFFFF; }}

    int get_sub_chunk_flag(void) { return (m_chunk_size & 0x80000000); }

    // Chunk type and size.
    // Note: MSB of m_chunk_size is used to indicate whether this chunk
    // contains other chunks or data.
    uint32_t m_chunk_type;
    uint32_t m_chunk_size;
};

struct MicroChunkHeader {
    MicroChunkHeader() : m_chunk_type(0), m_chunk_size(0) {}

    MicroChunkHeader(uint8_t type, uint8_t size) { m_chunk_type = type, m_chunk_size = size; }

    void set_type(uint8_t type) { m_chunk_type = type; }

    uint8_t get_type(void) { return m_chunk_type; }

    void set_size(uint8_t size) { m_chunk_size = size; }

    void add_size(uint8_t add) { set_size(get_size() + add); }

    uint8_t get_size(void) { return m_chunk_size; }

    uint8_t m_chunk_type;
    uint8_t m_chunk_size;
};


/**************************************************************************************
**
** ChunkSaveClass
** Wrap an instance of this class around an opened file for easy chunk
** creation.
**
**************************************************************************************/
class ChunkSaveClass {
public:
    explicit ChunkSaveClass(SDL_IOStream *stream);

    // Chunk methods
    bool begin_chunk(uint32_t id);

    bool end_chunk();

    int current_chunk_depth();

    // Micro chunk methods
    bool begin_micro_chunk(uint32_t id);

    bool end_micro_chunk();

    // write data into the file
    uint32_t write(const void *buf, uint32_t nbytes);

    uint32_t write(const IOVector2Struct &v);

    uint32_t write(const IOVector3Struct &v);

    uint32_t write(const IOVector4Struct &v);

    uint32_t write(const IOQuaternionStruct &q);

private:

    enum {
        MAX_STACK_DEPTH = 256
    };

    SDL_IOStream *m_file;

    // Chunk building support
    int m_stack_index;
    int m_position_stack[MAX_STACK_DEPTH];
    ChunkHeader m_header_stack[MAX_STACK_DEPTH];

    // MicroChunk building support
    bool m_in_micro_chunk;
    int m_micro_chunk_position;
    MicroChunkHeader m_micro_chunk_header;
};


/**************************************************************************************
**
** ChunkLoadClass
** wrap an instance of one of these objects around an opened file
** to easily parse the chunks in the file
**
**************************************************************************************/
class ChunkLoadClass {
public:

    explicit ChunkLoadClass(SDL_IOStream *stream);

    // Chunk methods
    bool open_chunk();

    bool close_chunk();

    uint32_t current_chunk_id();

    uint32_t current_chunk_length();

    int current_chunk_depth();

    int contains_chunks();

    // Micro Chunk methods
    bool open_micro_chunk();

    bool close_micro_chunk();

    uint32_t current_micro_chunk_id();

    uint32_t current_micro_chunk_length();

    // read a block of bytes from the output stream.
    uint32_t read(void *buf, uint32_t num_of_bytes);

    uint32_t read(IOVector2Struct *v);

    uint32_t read(IOVector3Struct *v);

    uint32_t read(IOVector4Struct *v);

    uint32_t read(IOQuaternionStruct *q);

    // seek over a block of bytes in the stream (same as read but don't copy the data to a buffer)
    uint32_t seek(uint32_t num_of_bytes);

    // Sneak peek at the next chunk that will be opened.  Beware, if you need
    // this, then you are probably hacking so be careful!
    bool peek_next_chunk(uint32_t *set_id, uint32_t *set_size);

private:

    enum {
        MAX_STACK_DEPTH = 256
    };

    SDL_IOStream *m_file;

    // Chunk reading support
    int m_stack_index;
    uint32_t m_position_stack[MAX_STACK_DEPTH];
    ChunkHeader m_header_stack[MAX_STACK_DEPTH];

    // Micro-chunk reading support
    bool m_in_micro_chunk;
    int m_micro_chunk_position;
    MicroChunkHeader m_micro_chunk_header;

};

/*
** WRITE_WWSTRING_CHUNK	- use this one-line macro to easily create a chunk to save a potentially
** long string.  Note:  This macro does NOT create a micro chunk...
** Example:
**
**	csave.Begin_Chunk(CHUNKID_PARENT);
**		ParentClass::Save (csave);
**	csave.End_Chunk();
**
**	WRITE_WWSTRING_CHUNK(csave, CHUNKID_NAME, string);
**	WRITE_WIDESTRING_CHUNK(csave, CHUNKID_WIDE_NAME, wide_string);
**
**	csave.begin_chunk(PHYSGRID_CHUNK_VARIABLES);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_VERSION,version);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_DUMMYVISID,DummyVisId);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_BASEVISID,BaseVisId);
**	csave.end_chunk();
**
*/
#define WRITE_WWSTRING_CHUNK(csave, id, var) { \
    csave.Begin_Chunk(id); \
    csave.Write((const TCHAR *)var, var.Get_Length () + 1); \
    csave.End_Chunk(); }

#define WRITE_WIDESTRING_CHUNK(csave, id, var) { \
    csave.Begin_Chunk(id); \
    csave.Write((const WCHAR *)var, (var.Get_Length () + 1) * 2); \
    csave.End_Chunk(); }


/*
** READ_WWSTRING_CHUNK	- use this macro in a switch statement to read the contents
**	of a chunk into a string object.
** Example:
**
**	while (cload.open_chunk()) {
**
**		switch(cload.current_chunk_id()) {
**			READ_WWSTRING_CHUNK(cload,CHUNKID_NAME,string);
**			READ_WIDESTRING_CHUNK(cload,CHUNKID_WIDE_NAME,wide_string);
**		}
**		cload.close_chunk();
**	}
**
*/
#define READ_WWSTRING_CHUNK(cload, id, var)        \
    case (id):    cload.Read(var.Get_Buffer(cload.Cur_Chunk_Length()),cload.Cur_Chunk_Length()); break;    \

#define READ_WIDESTRING_CHUNK(cload, id, var)        \
    case (id):    cload.Read(var.Get_Buffer((cload.Cur_Chunk_Length()+1)/2),cload.Cur_Chunk_Length()); break;    \


/*
** WRITE_MICRO_CHUNK	- use this one-line macro to easily make a micro chunk for an individual variable.
** Note that you should always wrap your micro-chunks inside a normal chunk.
** Example:
**
**	csave.begin_chunk(PHYSGRID_CHUNK_VARIABLES);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_VERSION,version);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_DUMMYVISID,DummyVisId);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_BASEVISID,BaseVisId);
**	csave.end_chunk();
*/
#define WRITE_MICRO_CHUNK(csave, id, var) { \
    csave.Begin_Micro_Chunk(id); \
    csave.Write(&var,sizeof(var)); \
    csave.End_Micro_Chunk(); }

#define WRITE_SAFE_MICRO_CHUNK(csave, id, var, type) { \
    csave.Begin_Micro_Chunk(id);        \
    type data = (type)var;                \
    csave.Write(&data,sizeof(data)); \
    csave.End_Micro_Chunk(); }

#define WRITE_MICRO_CHUNK_STRING(csave, id, var) { \
    csave.Begin_Micro_Chunk(id); \
    csave.Write(var, strlen(var) + 1); \
    csave.End_Micro_Chunk(); }

#define WRITE_MICRO_CHUNK_WWSTRING(csave, id, var) { \
    csave.Begin_Micro_Chunk(id); \
    csave.Write((const TCHAR *)var, var.Get_Length () + 1); \
    csave.End_Micro_Chunk(); }

#define WRITE_MICRO_CHUNK_WIDESTRING(csave, id, var) { \
    csave.Begin_Micro_Chunk(id); \
    csave.Write((const WCHAR *)var, (var.Get_Length () + 1) * 2); \
    csave.End_Micro_Chunk(); }


/*
** READ_MICRO_CHUNK - use this macro in a switch statement to read a micro chunk into a variable
** Example:
**
**	while (cload.open_micro_chunk()) {
**
**		switch(cload.current_micro_chunk_id()) {
**			READ_MICRO_CHUNK(cload,PHYSGRID_VARIABLE_VERSION,version);
**			READ_MICRO_CHUNK(cload,PHYSGRID_VARIABLE_DUMMYVISID,DummyVisId);
**			READ_MICRO_CHUNK(cload,PHYSGRID_VARIABLE_BASEVISID,BaseVisId);
**		}
**		cload.close_micro_chunk();
**	}
*/
#define READ_MICRO_CHUNK(cload, id, var)                        \
    case (id):    cload.Read(&var,sizeof(var)); break;    \

/*
** Like READ_MICRO_CHUNK but reads items straight into the data safe.
*/
#define READ_SAFE_MICRO_CHUNK(cload, id, var, type)                                \
    case (id):    {                                                     \
        void *temp_read_buffer_on_the_stack = _alloca(sizeof(type));    \
        cload.Read(temp_read_buffer_on_the_stack, sizeof(type));       \
        var = *((type*)temp_read_buffer_on_the_stack);                 \
        break;                                                         \
    }

#define READ_MICRO_CHUNK_STRING(cload, id, var, size)        \
    case (id):    WWASSERT(cload.Cur_Micro_Chunk_Length() <= size); cload.Read(var,cload.Cur_Micro_Chunk_Length()); break;    \

#define READ_MICRO_CHUNK_WWSTRING(cload, id, var)        \
    case (id):    cload.Read(var.Get_Buffer(cload.Cur_Micro_Chunk_Length()),cload.Cur_Micro_Chunk_Length()); break;    \

#define READ_MICRO_CHUNK_WIDESTRING(cload, id, var)        \
    case (id):    cload.Read(var.Get_Buffer((cload.Cur_Micro_Chunk_Length()+1)/2),cload.Cur_Micro_Chunk_Length()); break;    \

/*
** These load macros make it easier to add extra code to a specifc case
*/
#define LOAD_MICRO_CHUNK(cload, var)                        \
    cload.Read(&var,sizeof(var)); \

#define LOAD_MICRO_CHUNK_WWSTRING(cload, var)        \
    cload.Read(var.Get_Buffer(cload.Cur_Micro_Chunk_Length()),cload.Cur_Micro_Chunk_Length());    \

#define LOAD_MICRO_CHUNK_WIDESTRING(cload, var)        \
    cload.Read(var.Get_Buffer((cload.Cur_Micro_Chunk_Length()+1)/2),cload.Cur_Micro_Chunk_Length());    \


/*
** OBSOLETE_MICRO_CHUNK - use this macro in a switch statement when you want your code
** to skip a given micro chunk but not fall through to your 'default' case statement which
** prints an "unrecognized chunk" warning message.
*/
#define OBSOLETE_MICRO_CHUNK(id) \
    case (id): break;
