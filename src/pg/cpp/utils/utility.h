/**
 * @file utility.h
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
#ifndef PG_CPP_UTILS_UTILLITY_H_
#define PG_CPP_UTILS_UTILLITY_H_

#include "pg/postgres.h"

#include <string>
#include <vector>

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class Utility
            {

            public: // Data Type(s)

                class Record
                {

                public: // Constructor / Destructor

                    /**
                     * @brief Destructor.
                     */
                    virtual ~Record ()
                    {
                        /* empty */
                    }

                public: // Method(s) / Function(s)

                    /**
                     * @brief Allocate memory from postgres pool and copy a string.
                     *
                     * @param a_value.
                     */
                    inline char* PCopyString (const char* const a_value)
                    {
                        if ( nullptr == a_value ) {
                            return nullptr;
                        }
                        const size_t length = sizeof(char) * ( strlen(a_value) + sizeof(char) );
                        char* rv = (char*)palloc(length);
                        if ( nullptr != rv ) {
                            rv[0] = '\0';
                            snprintf(rv, length, "%s", a_value);
                        }
                        return rv;
                    }

                public: // Pure Virtual Method(s) / Function(s) - declaration

                    /**
                     * @brief Allocate memory from postgres pool and copy a record data to it.
                     *
                     * @return An array of string representing a record.
                     */
                    virtual char** PStringValues () = 0;

                };

                /**
                 * A class that holds a calculation results.
                 */
                class Records
                {

                protected: // Data

                    std::vector<Record*> items_;

                public: // Constructor / Destructor

                    /**
                     * @brief Default constructor.
                     */
                    Records ()
                    {
                        /* empty */
                    }

                    /**
                     * @brief Destructor.
                     */
                    virtual ~Records ()
                    {
                        for ( auto record : items_ ) {
                            if ( nullptr != record ) {
                                delete record;
                            }
                        }
                    }

                public:

                    /**
                     * @brief Push back a record ( it's memory will be owned by this object ).
                     *
                     * @param a_record
                     */
                    inline void Append (Record* a_record)
                    {
                        items_.push_back(a_record);
                    }

                    /**
                     * @brief Release a previously allocated object at a specific position.
                     *
                     * @param a_index Record position within the internal array.
                     * @param a_values
                     */
                    inline void Release (const size_t a_index, char** /* a_values */)
                    {
                        if ( nullptr != items_[a_index] ) {
                            delete items_[a_index];
                            items_[a_index] = nullptr;
                        }
                    }

                    /**
                     * @brief Allocate memory from postgres pool and copy a record data to it.
                     *
                     * @param a_index Record position within the internal array.
                     */
                    virtual char** GetPStringValues (const size_t a_index)
                    {
                        if ( a_index >= items_.size() ) {
                            return nullptr;
                        }
                        return items_[a_index]->PStringValues();
                    }

                };

            public: // Constructor / Destructor

                /**
                 * @brief Destructor.
                 */
                virtual ~Utility ()
                {
                    /* empty */
                }

            protected: // Data

                std::string error_;

            public: // Pure Virtual Method(s) / Function(s)

                virtual void FillOutputAtUserFuncContext (FuncCallContext* a_context) = 0;

            public: // Method(s) / Function(s)

                const std::string& LastError () const;

            public: // Static Method(s) / Function(s)

                static void AllocUserFuncContext   (FuncCallContext* a_context);
                static void DeallocUserFuncContext (FuncCallContext* a_context);

            }; // end of class Utility

            inline const std::string& Utility::LastError () const
            {
                return error_;
            }

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace 'pg'

#endif // PG_CPP_UTILS_UTILLITY_H_
