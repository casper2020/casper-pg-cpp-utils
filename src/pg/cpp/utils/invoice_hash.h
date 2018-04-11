/**
 * @file invoice_hash.h
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
#ifndef PG_CPP_UTILS_INVOICE_HASH_H_
#define PG_CPP_UTILS_INVOICE_HASH_H_

#include "pg/cpp/utils/utility.h"

#include <string>     // std::string
#include <sstream>    // std::stringstream
#include <functional> // std::function

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class InvoiceHash final : public Utility
            {

            public: // Data Type(s)

                class Record final : public Utility::Record
                {

                public: // Const Data

                    const std::string long_;
                    const std::string short_;

                private: // Data

                    char** p_string_values_;

                public: // Constructor / Destructor

                    /**
                     * @brief Default constructor.
                     *
                     * @param a_long
                     * @param a_short
                     */
                    Record (const std::string& a_long, const std::string& a_short)
                    : long_(a_long), short_(a_short)
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
                            if ( nullptr != p_string_values_[1] ) {
                                pfree(p_string_values_[1]);
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
                        char** tmp = (char**)palloc(sizeof(char*)*2);
                        if ( nullptr == tmp ) {
                            return nullptr;
                        }

                        tmp[0] = PCopyString(long_.c_str());
                        tmp[1] = PCopyString(short_.c_str());

                        return tmp;
                    }

                }; // end of 'Record' class

            private: // Const Data

                const std::string pem_uri_;

            private: // Data

                std::string       long_;
                std::string       short_;
                std::stringstream tmp_ss_;

            public: // Constructor / Destructor.

                InvoiceHash (const std::string& a_pem_uri);
                virtual ~InvoiceHash();

            public: // Inherited Pure Virtual Method(s) / Function(s)

                virtual void FillOutputAtUserFuncContext (FuncCallContext* a_context);

            public: // Method(s) / Function(s)

                void Calculate (const std::string& a_payload);

            }; // end of class 'InvoiceHash'

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace pg

#endif // PG_CPP_UTILS_INVOICE_HASH_H_
