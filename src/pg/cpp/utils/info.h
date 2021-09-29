/**
 * @file info.h
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
#ifndef PG_CPP_UTILS_INFO_H_
#define PG_CPP_UTILS_INFO_H_

#include "pg/cpp/utils/utility.h"

#include <string>

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class Info final : public Utility
            {

            public: // Data Type(s)

                 class Record final : public Utility::Record
                {

                public: // Const Data

                    const std::string version_;
                    const std::string target_;
                    const std::string date_;
                    const std::string repo_;
                    const std::string dependencies_;

                private: // Data

                    char** p_string_values_;
                    size_t n_strings_;

                public: // Constructor / Destructor

                    /**
                     * @brief Default constructor.
                     *
                     * @param a_version
                     * @param a_target
                     * @param a_date
                     * @param a_repo
                     */
                    Record (const std::string& a_version, const std::string& a_target, const std::string& a_date, const std::string a_repo,
                            const std::string& a_dependencies)
                    : version_(a_version), target_(a_target), date_(a_date), repo_(a_repo), dependencies_(a_dependencies)
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
                            for ( auto idx = 0 ; idx < n_strings_ ; ++idx ) {
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
                        n_strings_ = 5;
                        char** tmp = (char**)palloc(sizeof(char*)*n_strings_);
                        if ( nullptr == tmp ) {
                            return nullptr;
                        }

                        tmp[0] = PCopyString(version_.c_str());
                        tmp[1] = PCopyString(target_.c_str());
                        tmp[2] = PCopyString(date_.c_str());
                        tmp[3] = PCopyString(repo_.c_str());
                        tmp[4] = PCopyString(dependencies_.c_str());

                        return tmp;
                    }

                }; // end of 'Record' class

            public: // Constructor / Destructor.

                Info ();
                virtual ~Info();

            public: // Inherited Pure Virtual Method(s) / Function(s)

                virtual void FillOutputAtUserFuncContext (FuncCallContext* a_context);

            public: // Method(s) / Function(s)

                void Calculate (const std::string& a_payload);

            }; // end of class 'InvoiceHash'

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace 'pg'

#endif // PG_CPP_UTILS_INFO_H_
