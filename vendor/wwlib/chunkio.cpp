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

/* $Header: /Commando/Code/wwlib/chunkio.cpp 12    3/14/02 1:25p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Tiberian Sun / Commando / G Library                          * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwlib/chunkio.cpp                            $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 3/04/02 4:00p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 12                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   ChunkSaveClass::ChunkSaveClass -- Constructor                                             * 
 *   ChunkSaveClass::begin_chunk -- Begin a new chunk in the file                              *
 *   ChunkSaveClass::end_chunk -- Close a chunk, computes the size and adds to the header      *
 *   ChunkSaveClass::begin_micro_chunk -- begins a new "micro-chunk"                           *
 *   ChunkSaveClass::end_micro_chunk -- close a micro-chunk                                    *
 *   ChunkSaveClass::write -- write data into the current chunk                                *
 *   ChunkSaveClass::write -- write an IOVector2Struct                                         *
 *   ChunkSaveClass::write -- write an IOVector3Struct                                         *
 *   ChunkSaveClass::write -- write an IOVector4Struct                                         *
 *   ChunkSaveClass::write -- write an IOQuaternionStruct                                      *
 *   ChunkSaveClass::current_chunk_depth -- returns the current chunk recursion depth (debugging)  *
 *   ChunkLoadClass::ChunkLoadClass -- Constructor                                             * 
 *   ChunkLoadClass::open_chunk -- Open a chunk in the file, reads in the chunk header         *
 *   ChunkLoadClass::peek_next_chunk -- sneak peek into the next chunk that will be opened     *
 *   ChunkLoadClass::close_chunk -- Close a chunk, seeks to the end if needed                  *
 *   ChunkLoadClass::current_chunk_id -- Returns the ID of the current chunk                       *
 *   ChunkLoadClass::current_chunk_length -- Returns the current length of the current chunk       *
 *   ChunkLoadClass::current_chunk_depth -- returns the current chunk recursion depth              *
 *   ChunkLoadClass::contains_chunks -- Test whether the current chunk contains chunks (or dat *
 *   ChunkLoadClass::open_micro_chunk -- reads in a micro-chunk header                         *
 *   ChunkLoadClass::close_micro_chunk -- closes a micro-chunk                                 *
 *   ChunkLoadClass::current_micro_chunk_id -- returns the ID of the current micro-chunk (asserts  *
 *   ChunkLoadClass::current_micro_chunk_length -- returns the size of the current micro chunk     *
 *   ChunkLoadClass::read -- read data from the file                                           *
 *   ChunkLoadClass::read -- read an IOVector2Struct                                           *
 *   ChunkLoadClass::read -- read an IOVector3Struct                                           *
 *   ChunkLoadClass::read -- read an IOVector4Struct                                           *
 *   ChunkLoadClass::read -- read an IOQuaternionStruct                                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <cstring>
#include <cassert>
#include <string>
#include "chunkio.h"

/*********************************************************************************************** 
 * ChunkSaveClass::ChunkSaveClass -- Constructor                                               * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *  file - pointer to a FileClass object to write to														  * 
 * 																														  * 
 * OUTPUT:																												  * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
ChunkSaveClass::ChunkSaveClass(SDL_IOStream *stream) :
        m_file(stream),
        m_stack_index(0),
        m_in_micro_chunk(false),
        m_micro_chunk_position(0) {
    memset(m_position_stack, 0, sizeof(m_position_stack));
    memset(m_header_stack, 0, sizeof(m_header_stack));
    memset(&m_micro_chunk_header, 0, sizeof(m_micro_chunk_header));
}


/*********************************************************************************************** 
 * ChunkSaveClass::begin_chunk -- Begin a new chunk in the file                                *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *  id - id of the chunk																							  * 
 * 																														  * 
 * OUTPUT:																												  * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
bool ChunkSaveClass::begin_chunk(uint32_t id) {
    ChunkHeader chunkh;
    int filepos;

    // If we have a parent chunk, set its 'contains_chunks' flag
    if (m_stack_index > 0) {
        m_header_stack[m_stack_index - 1].set_sub_chunk_flag(true);
    }

    // Save the current file position and chunk header
    // for the call to end_chunk.
    chunkh.set_type(id);
    chunkh.set_size(0);
    filepos = SDL_SeekIO(m_file, 0, SDL_IO_SEEK_CUR);

    m_position_stack[m_stack_index] = filepos;
    m_header_stack[m_stack_index] = chunkh;
    m_stack_index++;

    // write a temporary chunk header (size = 0)
    if (SDL_WriteIO(m_file, &chunkh, sizeof(chunkh)) != sizeof(chunkh)) {
        return false;
    }
    return true;
}


/*********************************************************************************************** 
 * ChunkSaveClass::end_chunk -- Close a chunk, computes the size and adds to the header        *
 *  																														  * 
 * INPUT:																												  * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
bool ChunkSaveClass::end_chunk(void) {
    // If the user didn't close his micro chunks bad things are gonna happen
    assert(!m_in_micro_chunk);

    // Save the current position
    int curpos = SDL_SeekIO(m_file, 0, SDL_IO_SEEK_CUR);

    // Pop the position and chunk header off the stacks
    m_stack_index--;
    int chunkpos = m_position_stack[m_stack_index];
    ChunkHeader chunkh = m_header_stack[m_stack_index];

    // write the completed header
    SDL_SeekIO(m_file, chunkpos, SDL_IO_SEEK_SET);
    if (SDL_WriteIO(m_file, &chunkh, sizeof(chunkh)) != sizeof(chunkh)) {
        return false;
    }

    // Add the total bytes written to any encompasing chunk
    if (m_stack_index != 0) {
        m_header_stack[m_stack_index - 1].add_size(chunkh.get_size() + sizeof(chunkh));
    }

    // Go back to the end of the file
    SDL_SeekIO(m_file, curpos, SDL_IO_SEEK_SET);

    return true;
}


/***********************************************************************************************
 * ChunkSaveClass::begin_micro_chunk -- begins a new "micro-chunk"                             *
 *                                                                                             *
 * Micro chunks are used to wrap individual variables.  They aren't hierarchical so if you     *
 * attempt to open a micro chunk while already in one, an assert will occur.                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 * id - 8bit id                                                                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * id is asserted to be between 0 and 255                                                      *
 * cannot nest micro chunks so it asserts that you are currently not in another micro-chunk    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool ChunkSaveClass::begin_micro_chunk(uint32_t id) {
    assert(id < 256);
    assert(!m_in_micro_chunk);

    // Save the current file position and chunk header
    // for the call to end_micro_chunk.
    m_micro_chunk_header.set_type(id);
    m_micro_chunk_header.set_size(0);
    m_micro_chunk_position = SDL_SeekIO(m_file, 0, SDL_IO_SEEK_CUR);

    // write a temporary chunk header
    // NOTE: I'm calling the ChunkSaveClass::write method so that the bytes for
    // this header are tracked in the wrapping chunk.  This is because micro-chunks
    // are simply data inside the normal chunks...
    if (write(&m_micro_chunk_header, sizeof(m_micro_chunk_header)) != sizeof(m_micro_chunk_header)) {
        return false;
    }

    m_in_micro_chunk = true;
    return true;
}


/***********************************************************************************************
 * ChunkSaveClass::end_micro_chunk -- close a micro-chunk                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool ChunkSaveClass::end_micro_chunk(void) {
    assert(m_in_micro_chunk);

    // Save the current position
    int curpos = SDL_SeekIO(m_file, 0, SDL_IO_SEEK_CUR);

    // seek back and write the micro chunk header
    SDL_SeekIO(m_file, m_micro_chunk_position, SDL_IO_SEEK_SET);

    if (SDL_WriteIO(m_file, &m_micro_chunk_header, sizeof(m_micro_chunk_header)) != sizeof(m_micro_chunk_header)) {
        return false;
    }

    // Go back to the end of the file
    SDL_SeekIO(m_file, curpos, SDL_IO_SEEK_SET);
    m_in_micro_chunk = false;
    return true;
}

/*********************************************************************************************** 
 * ChunkSaveClass::write -- write data into the current chunk                                  *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
uint32_t ChunkSaveClass::write(const void *buf, uint32_t nbytes) {
    // If this assert hits, you mixed data and chunks within the same chunk NO NO!
    assert(m_header_stack[m_stack_index - 1].get_sub_chunk_flag() == 0);

    // If this assert hits, you didnt open any chunks yet
    assert(m_stack_index > 0);

    // write the bytes into the file
    if (SDL_WriteIO(m_file, buf, nbytes) != (int) nbytes) return 0;

    // track them in the wrapping chunk
    m_header_stack[m_stack_index - 1].add_size(nbytes);

    // track them if you are using a micro-chunk too.
    if (m_in_micro_chunk) {
        assert(m_micro_chunk_header.get_size() < 255 - nbytes);    // micro chunks can only be 255 bytes
        m_micro_chunk_header.add_size(nbytes);
    }

    return nbytes;
}


/***********************************************************************************************
 * ChunkSaveClass::write -- write an IOVector2Struct                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkSaveClass::write(const IOVector2Struct &v) {
    return write(&v, sizeof(v));
}


/***********************************************************************************************
 * ChunkSaveClass::write -- write an IOVector3Struct                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkSaveClass::write(const IOVector3Struct &v) {
    return write(&v, sizeof(v));
}


/***********************************************************************************************
 * ChunkSaveClass::write -- write an IOVector4Struct                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkSaveClass::write(const IOVector4Struct &v) {
    return write(&v, sizeof(v));
}

/***********************************************************************************************
 * ChunkSaveClass::write -- write an IOQuaternionStruct                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkSaveClass::write(const IOQuaternionStruct &q) {
    return write(&q, sizeof(q));
}

/*********************************************************************************************** 
 * ChunkSaveClass::current_chunk_depth -- returns the current chunk recursion depth (debugging)    *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
int ChunkSaveClass::current_chunk_depth(void) {
    return m_stack_index;
}


/*********************************************************************************************** 
 * ChunkLoadClass::ChunkLoadClass -- Constructor                                               * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
ChunkLoadClass::ChunkLoadClass(SDL_IOStream *stream) :
        m_file(stream),
        m_stack_index(0),
        m_in_micro_chunk(false),
        m_micro_chunk_position(0) {
    memset(m_position_stack, 0, sizeof(m_position_stack));
    memset(m_header_stack, 0, sizeof(m_header_stack));
    memset(&m_micro_chunk_header, 0, sizeof(m_micro_chunk_header));
}


/*********************************************************************************************** 
 * ChunkLoadClass::open_chunk -- Open a chunk in the file, reads in the chunk header           *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
bool ChunkLoadClass::open_chunk() {
    // if user didn't close any micro chunks that he opened, bad things could happen
    assert(m_in_micro_chunk == false);

    // check for stack overflow
    assert(m_stack_index < MAX_STACK_DEPTH - 1);

    // if the parent chunk has been completely eaten, return false
    if ((m_stack_index > 0) && (m_position_stack[m_stack_index - 1] == m_header_stack[m_stack_index - 1].get_size())) {
        return false;
    }

    // read the chunk header
    if (SDL_ReadIO(m_file, &m_header_stack[m_stack_index], sizeof(ChunkHeader)) != sizeof(ChunkHeader)) {
        return false;
    }

    m_position_stack[m_stack_index] = 0;
    m_stack_index++;
    return true;
}


/***********************************************************************************************
 * ChunkLoadClass::peek_next_chunk -- sneak peek into the next chunk that will be opened       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/2002   gth : Created.                                                                 *
 *=============================================================================================*/
bool ChunkLoadClass::peek_next_chunk(uint32_t *set_id, uint32_t *set_size) {
    // if user didn't close any micro chunks that he opened, bad things could happen
    assert(m_in_micro_chunk == false);

    // check for stack overflow
    assert(m_stack_index < MAX_STACK_DEPTH - 1);

    // if the parent chunk has been completely eaten, return false
    if ((m_stack_index > 0) && (m_position_stack[m_stack_index - 1] == m_header_stack[m_stack_index - 1].get_size())) {
        return false;
    }

    // peek at the next chunk header, return false if the read fails
    ChunkHeader temp_header;
    if (SDL_ReadIO(m_file, &temp_header, sizeof(ChunkHeader)) != sizeof(ChunkHeader)) {
        return false;
    }

    int seek_offset = sizeof(ChunkHeader);
    SDL_SeekIO(m_file, -seek_offset, SDL_IO_SEEK_CUR);

    if (set_id != nullptr) {
        *set_id = temp_header.get_type();
    }
    if (set_size != nullptr) {
        *set_size = temp_header.get_size();
    }

    return true;
}

/*********************************************************************************************** 
 * ChunkLoadClass::close_chunk -- Close a chunk, seeks to the end if needed                    *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
bool ChunkLoadClass::close_chunk() {
    // if user didn't close any micro chunks that he opened, bad things could happen
    assert(m_in_micro_chunk == false);

    // check for stack overflow
    assert(m_stack_index > 0);

    int csize = m_header_stack[m_stack_index - 1].get_size();
    int pos = m_position_stack[m_stack_index - 1];

    if (pos < csize) {
        SDL_SeekIO(m_file, csize - pos, SDL_IO_SEEK_CUR);
    }

    m_stack_index--;
    if (m_stack_index > 0) {
        m_position_stack[m_stack_index - 1] += csize + sizeof(ChunkHeader);
    }

    return true;
}


/*********************************************************************************************** 
 * ChunkLoadClass::current_chunk_id -- Returns the ID of the current chunk                         *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
uint32_t ChunkLoadClass::current_chunk_id() {
    assert(m_stack_index >= 1);
    return m_header_stack[m_stack_index - 1].get_type();
}


/*********************************************************************************************** 
 * ChunkLoadClass::current_chunk_length -- Returns the current length of the current chunk         *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
uint32_t ChunkLoadClass::current_chunk_length() {
    assert(m_stack_index >= 1);
    return m_header_stack[m_stack_index - 1].get_size();
}


/*********************************************************************************************** 
 * ChunkLoadClass::current_chunk_depth -- returns the current chunk recursion depth                *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
int ChunkLoadClass::current_chunk_depth() {
    return m_stack_index;
}


/***********************************************************************************************
 * ChunkLoadClass::contains_chunks -- Test whether the current chunk contains chunks (or data) *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/99    GTH : Created.                                                                 *
 *=============================================================================================*/
int ChunkLoadClass::contains_chunks() {
    return m_header_stack[m_stack_index - 1].get_sub_chunk_flag();
}

/***********************************************************************************************
 * ChunkLoadClass::open_micro_chunk -- reads in a micro-chunk header                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool ChunkLoadClass::open_micro_chunk() {
    assert(!m_in_micro_chunk);

    // read the chunk header
    // calling the ChunkLoadClass::read fn so that if we exhaust the chunk, the read will fail
    if (read(&m_micro_chunk_header, sizeof(m_micro_chunk_header)) != sizeof(m_micro_chunk_header)) {
        return false;
    }

    m_in_micro_chunk = true;
    m_micro_chunk_position = 0;
    return true;
}


/***********************************************************************************************
 * ChunkLoadClass::close_micro_chunk -- closes a micro-chunk (seeks to end)                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool ChunkLoadClass::close_micro_chunk() {
    assert(m_in_micro_chunk);
    m_in_micro_chunk = false;

    int csize = m_micro_chunk_header.get_size();
    int pos = m_micro_chunk_position;

    // seek the file past this micro chunk
    if (pos < csize) {

        SDL_SeekIO(m_file, csize - pos, SDL_IO_SEEK_CUR);

        // update the tracking variables for where we are in the normal chunk.
        if (m_stack_index > 0) {
            m_position_stack[m_stack_index - 1] += csize - pos;
        }
    }

    return true;
}


/***********************************************************************************************
 * ChunkLoadClass::current_micro_chunk_id -- returns the ID of the current micro-chunk (asserts if *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Asserts if you are not currently inside a micro-chunk                                       *
 * Micro chunks have an id between 0 and 255                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkLoadClass::current_micro_chunk_id() {
    assert(m_in_micro_chunk);
    return m_micro_chunk_header.get_type();
}


/***********************************************************************************************
 * ChunkLoadClass::current_micro_chunk_length -- returns the size of the current micro chunk       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Asserts if you are not currently inside a micro-chunk                                       *
 * Micro chunks have a maximum size of 255 bytes                                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkLoadClass::current_micro_chunk_length() {
    assert(m_in_micro_chunk);
    return m_micro_chunk_header.get_size();
}

// seek over nbytes in the stream
uint32_t ChunkLoadClass::seek(uint32_t nbytes) {
    assert(m_stack_index >= 1);

    // Don't seek if we would go past the end of the current chunk
    if (m_position_stack[m_stack_index - 1] + nbytes > (int) m_header_stack[m_stack_index - 1].get_size()) {
        return 0;
    }

    // Don't read if we are in a micro chunk and would go past the end of it
    if (m_in_micro_chunk && m_micro_chunk_position + nbytes > m_micro_chunk_header.get_size()) {
        return 0;
    }

    uint32_t curpos = SDL_SeekIO(m_file, 0, SDL_IO_SEEK_CUR);

    if (SDL_SeekIO(m_file, nbytes, SDL_IO_SEEK_CUR) - curpos != (int) nbytes) {
        return 0;
    }

    // Update our position in the chunk
    m_position_stack[m_stack_index - 1] += nbytes;

    // Update our position in the micro chunk if we are in one
    if (m_in_micro_chunk) {
        m_micro_chunk_position += nbytes;
    }

    return nbytes;
}

/*********************************************************************************************** 
 * ChunkLoadClass::read -- read data from the file                                             *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/17/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
uint32_t ChunkLoadClass::read(void *buf, uint32_t nbytes) {
    assert(m_stack_index >= 1);

    // Don't read if we would go past the end of the current chunk
    if (m_position_stack[m_stack_index - 1] + nbytes > (int) m_header_stack[m_stack_index - 1].get_size()) {
        return 0;
    }

    // Don't read if we are in a micro chunk and would go past the end of it
    if (m_in_micro_chunk && m_micro_chunk_position + nbytes > m_micro_chunk_header.get_size()) {
        return 0;
    }

    if (SDL_ReadIO(m_file, buf, nbytes) != (int) nbytes) {
        return 0;
    }

    // Update our position in the chunk
    m_position_stack[m_stack_index - 1] += nbytes;

    // Update our position in the micro chunk if we are in one
    if (m_in_micro_chunk) {
        m_micro_chunk_position += nbytes;
    }

    return nbytes;
}


/***********************************************************************************************
 * ChunkLoadClass::read -- read an IOVector2Struct                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkLoadClass::read(IOVector2Struct *v) {
    assert(v != nullptr);
    return read(v, sizeof(IOVector2Struct));
}


/***********************************************************************************************
 * ChunkLoadClass::read -- read an IOVector3Struct                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkLoadClass::read(IOVector3Struct *v) {
    assert(v != nullptr);
    return read(v, sizeof(IOVector3Struct));
}


/***********************************************************************************************
 * ChunkLoadClass::read -- read an IOVector4Struct                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkLoadClass::read(IOVector4Struct *v) {
    assert(v != nullptr);
    return read(v, sizeof(IOVector4Struct));
}


/***********************************************************************************************
 * ChunkLoadClass::read -- read an IOQuaternionStruct                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
uint32_t ChunkLoadClass::read(IOQuaternionStruct *q) {
    assert(q != nullptr);
    return read(q, sizeof(IOQuaternionStruct));
}

