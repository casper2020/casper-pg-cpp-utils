/**
 * @file locales.h
 *
 * Copyright (c) 2011-2022 Cloudware S.A. All rights reserved.
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
#ifndef PG_CPP_UTILS_LOCALES_H_
#define PG_CPP_UTILS_LOCALES_H_

#include "pg/cpp/utils/utility.h"

#include <string>
#include <vector>
#include <sstream>

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class Locales final : public Utility
            {

            public: // Data Type(s)

                 class Record final : public Utility::Record
                {

                public: // Const Data

                    const std::string icu_;
                    const std::string locales_;

                private: // Data

                    char**            p_string_values_;
                    size_t            n_strings_;

                public: // Constructor / Destructor

                    /**
                     * @brief Default constructor.
                     *
                     * @param a_version
                     * @param a_target
                     * @param a_date
                     * @param a_repo
                     */
                    Record (const std::string& a_icu, const std::string& a_locales)
                    : icu_(a_icu), locales_(a_locales)
                    {
                        p_string_values_ = nullptr;
                        n_strings_       = 0;
                    }

                    /**
                     * @brief Destructor.
                     */
                    virtual ~Record ()
                    {
                        if ( nullptr != p_string_values_ ) {
                            for ( size_t idx = 0 ; idx < n_strings_ ; ++idx ) {
                                if ( nullptr != p_string_values_[idx] ) {
                                    pfree(p_string_values_[idx]);
                                }
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
                        n_strings_ = 2;
                        char** tmp = (char**)palloc(sizeof(char*)*n_strings_);
                        if ( nullptr == tmp ) {
                            return nullptr;
                        }
                        tmp[0] = PCopyString(icu_.c_str());
                        tmp[1] = PCopyString(locales_.c_str());
                        return tmp;
                    }

                }; // end of 'Record' class

            public: // Constructor / Destructor.

                Locales ();
                virtual ~Locales();

            public: // Inherited Pure Virtual Method(s) / Function(s)

                virtual void FillOutputAtUserFuncContext (FuncCallContext* a_context);

            public: // Method(s) / Function(s)

                void Calculate (const std::string& a_payload);

            }; // end of class 'InvoiceHash'

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace 'pg'

#endif // PG_CPP_UTILS_LOCALES_H_
