/**
 * @file b64.cc
 *
 * Copyright (c) 2011-2018 Cloudware S.A. All rights reserved.
 *
 * This file is part of casper-pg-cpp-utils.
 *
 * casper-pg-cpp-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper-pg-cpp-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with casper.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pg/cpp/utils/b64.h"

const char pg::cpp::utils::B64::k_table64_[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * @brief Default constructor.
 */
pg::cpp::utils::B64::B64 ()
{
    encode_buffer_ = nullptr;
}

/**
 * @brief Destructor.
 */
pg::cpp::utils::B64::~B64 ()
{
    if ( nullptr != encode_buffer_ ) {
        delete [] encode_buffer_;
    }
}

/**
 * @brief Encode an unsigned int buffer to base 64.
 *
 * @param a_payload Pointer to an allocated area holding the data to be encoded.
 * @param a_size    The data size.
 *
 * @return          Read only access to the encoded buffer.
 */
const char* const pg::cpp::utils::B64::Encode (const unsigned char* a_payload, unsigned int a_size)
{
    const unsigned char* in_data  = a_payload;
    unsigned int         in_size  = a_size;
    unsigned int         out_size = 0;
    unsigned char        i_buf[3];
    unsigned char        o_buf[4];

    // ... release previousoly encode buffer ...
    if ( nullptr != encode_buffer_ ) {
        delete [] encode_buffer_;
        encode_buffer_ = nullptr;
    }

    // ... ensure valid params ...
    if ( a_payload == nullptr || 0 == a_size ) {
        return nullptr;
    }

    // ... calculate the length of a base64-encoded string ...
    out_size = ( in_size / 3) << 2;
    if ( ( in_size % 3 ) > 0 ) {
        out_size += 4;
    }

    // ... terminating null for the encoded string ...
    out_size += 1;

    // ... allocate memory for the encoded buffer ...
    encode_buffer_ = new char[out_size];

    // ... encode ...
    unsigned int input_parts = 0;
    while ( in_size > 0 ) {

        // ... extract 3 ...
        input_parts = 0;
        for ( unsigned char i = 0; i < 3; i++) {
            if ( in_size > 0 ) {
                input_parts++;
                i_buf[i] = *in_data;
                in_data++;
                in_size--;
            } else {
                i_buf[i] = 0;
            }
        }

        // ... encode to 4 ...
        o_buf[0] = (unsigned char) ((i_buf[0] & 0xFC) >> 2);
        o_buf[1] = (unsigned char) (((i_buf[0] & 0x03) << 4) | ((i_buf[1] & 0xF0) >> 4));
        o_buf[2] = (unsigned char) (((i_buf[1] & 0x0F) << 2) | ((i_buf[2] & 0xC0) >> 6));
        o_buf[3] = (unsigned char) (i_buf[2] & 0x3F);

        switch ( input_parts ) {
            case 1: // ... only one byte read ...
            {
                /*
                 * (2) the final quantum of encoding input is exactly 8 bits; here, the
                 * final unit of encoded output will be two characters followed by two
                 * "=" padding characters.
                 */
                encode_buffer_[0] = pg::cpp::utils::B64::k_table64_[o_buf[0]];
                encode_buffer_[1] = pg::cpp::utils::B64::k_table64_[o_buf[1]];
                encode_buffer_[2] = '=';
                encode_buffer_[3] = '=';
            }
                break;
            case 2: // ... two bytes read ...
            {
                /*
                 * (3) the final quantum of encoding input is exactly 16 bits; here, the
                 *     final unit of encoded output will be three characters followed by one
                 *     "=" padding character.
                 */
                encode_buffer_[0] = pg::cpp::utils::B64::k_table64_[o_buf[0]];
                encode_buffer_[1] = pg::cpp::utils::B64::k_table64_[o_buf[1]];
                encode_buffer_[2] = pg::cpp::utils::B64::k_table64_[o_buf[2]];
                encode_buffer_[3] = '=';
            }
                break;
            default: // ... tree bytes read ...
            {
                /*
                 * (1) the final quantum of encoding input is an integral multiple of 24
                 *	   bits; here, the final unit of encoded output will be an integral
                 *    multiple of 4 characters with no "=" padding.
                 */
                encode_buffer_[0] = pg::cpp::utils::B64::k_table64_[o_buf[0]];
                encode_buffer_[1] = pg::cpp::utils::B64::k_table64_[o_buf[1]];
                encode_buffer_[2] = pg::cpp::utils::B64::k_table64_[o_buf[2]];
                encode_buffer_[3] = pg::cpp::utils::B64::k_table64_[o_buf[3]];
            }
                break;
        }

        encode_buffer_ += 4;

    }

    // ... terminate and rewind ...
    encode_buffer_[0] = 0;
    encode_buffer_   -= ( out_size - 1);
    // ... we're done ...
    return encode_buffer_;
}
