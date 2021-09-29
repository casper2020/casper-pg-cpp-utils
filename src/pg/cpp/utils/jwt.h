/**
 * @file jwt.h
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
#ifndef PG_CPP_UTILS_JWT_H_
#define PG_CPP_UTILS_JWT_H_

#include "pg/cpp/utils/utility.h"

#include "cc/auth/jwt.h"

#include <string>     // std::string
#include <sstream>    // std::stringstream
#include <functional> // std::function

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class JWT final : public Utility
            {

            public: // Data Type(s)

                class Record final : public Utility::Record
                {

                public: // Const Data

                    const std::string payload_;

                private: // Data

                    char** p_string_values_;

                public: // Constructor / Destructor

                    /**
                     * @brief Default constructor.
                     *
                     * @param a_url
                     */
                    Record (const std::string& a_url)
                        : payload_(a_url)
                    {
                        p_string_values_ = nullptr;
                    }

                    /**
                     * @brief Destructor.
                     */
                    virtual ~Record ()
                    {
                        if ( nullptr != p_string_values_ ) {
                            if ( nullptr != p_string_values_[0] ) {
                                pfree(p_string_values_[0]);
                            }
                            pfree(p_string_values_);
                        }
                    }

                public: // Inherited Pure Virtual Method(s) / Function(s) - implementation

                    /**
                     * @brief Allocate memory from postgres pool and copy a record data to it.
                     *
                     * @return An array of string representing a record.
                     */
                    virtual char** PStringValues ()
                    {
                        char** tmp = (char**)palloc(sizeof(char*)*1);
                        if ( nullptr == tmp ) {
                            return nullptr;
                        }

                        tmp[0] = PCopyString(payload_.c_str());

                        return tmp;
                    }

                }; // end of 'Record' class

            private: // Const Data

                const std::string    pkey_uri_;

            private: // Helper(s)

                ::cc::auth::JWT     jwt_;

            private: // Data

                std::string          payload_;

            public: // Constructor / Destructor.

                JWT (const std::string& a_pkey_uri);
                virtual ~JWT();

            public: // Inherited Pure Virtual Method(s) / Function(s)

                virtual void FillOutputAtUserFuncContext (FuncCallContext* a_context);

            public: // Method(s) / Function(s)

                void Encode (const uint64_t& a_duration, const std::string& a_payload);

            }; // end of class 'JWT'

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace pg

#endif // PG_CPP_UTILS_JWT_H_
