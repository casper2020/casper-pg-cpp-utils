/**
 * @file b64.h
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

#pragma once
#ifndef PG_CPP_UTILS_B64_H_
#define PG_CPP_UTILS_B64_H_

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class B64 final
            {

            private: // Static Const Data

                static const char k_table64_ [];

            private : // Data

                char* encode_buffer_;

            public: // Constructor(s) / Destructor

                B64();
                virtual ~B64();

            public: // Method(s) / Function(s)

                const char* const Encode (const unsigned char* a_payload, unsigned int a_size);

            }; // end of class 'B64'

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace pg

#endif // PG_CPP_UTILS_B64_H_
