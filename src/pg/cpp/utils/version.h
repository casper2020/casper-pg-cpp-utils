/**
 * @file version.h
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
#ifndef PG_CPP_UTILS_VERSION_H_
#define PG_CPP_UTILS_VERSION_H_

#include "pg/cpp/utils/utility.h"

#include <string>

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class Version final : public Utility
            {

            public: // Data Type(s)

                class Record final : public Utility::Record
                {

                public: // Const Data

                    const std::string short_;

                private: // Data

                    char** p_string_values_;

                public: // Constructor / Destructor

                    /**
                     * @brief Default constructor.
                     *
                     * @param a_short
                     */
                    Record (const std::string& a_short)
                        : short_(a_short)
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

                        tmp[0] = PCopyString(short_.c_str());

                        return tmp;
                    }

                }; // end of 'Record' class

            private: // Data

                std::string short_;

            public: // Constructor / Destructor.

                Version ();
                virtual ~Version();

            public: // Inherited Pure Virtual Method(s) / Function(s)

                virtual void FillOutputAtUserFuncContext (FuncCallContext* a_context);

            public: // Method(s) / Function(s)

                void Calculate (const std::string& a_payload);

            }; // end of class 'InvoiceHash'

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace 'pg'

#endif // PG_CPP_UTILS_VERSION_H_
