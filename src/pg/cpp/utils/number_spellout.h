/**
 * @file number_spellout.h
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
#ifndef PG_CPP_UTILS_NUMBER_SPELLOUT_H_
#define PG_CPP_UTILS_NUMBER_SPELLOUT_H_

#include "pg/cpp/utils/utility.h"

#include <string>          // std::string

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"

#include "unicode/locid.h" // ICU Locale
#include "unicode/rbnf.h"  // ICU RuleBasedNumberFormat

#pragma clang diagnostic pop

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class NumberSpellout final : public Utility
            {

            public: // Data Type(s)

                class Record final : public Utility::Record
                {

                public: // Const Data

                    const std::string string_;

                private: // Data

                    char** p_string_values_;

                public: // Constructor / Destructor

                    /**
                     * @brief Default constructor.
                     *
                     * @param a_string
                     */
                    Record (const std::string& a_string)
                    : string_(a_string)
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

                        tmp[0] = PCopyString(string_.c_str());

                        return tmp;
                    }

                }; // end of 'Record' class

            private: // Data

                U_ICU_NAMESPACE::Locale                 icu_locale_;
                UErrorCode                              icu_error_code_;
                UParseError                             icu_parse_error_;
                U_ICU_NAMESPACE::RuleBasedNumberFormat* icu_number_format_;
                std::string                             string_;

            public: // Constructor / Destructor.

                NumberSpellout (const std::string& a_locale, const std::string& a_spellout_override);
                virtual ~NumberSpellout();

            public: // Inherited Pure Virtual Method(s) / Function(s)

                virtual void FillOutputAtUserFuncContext (FuncCallContext* a_context);

            public: // Method(s) / Function(s)

                void Spellout         (double a_number);
                void CurrencySpellout (double a_major, const std::string& a_major_singular, const std::string& a_major_plural,
                                       double a_minor, const std::string& a_minor_singular, const std::string& a_minor_plural,
                                       const std::string& a_format);

            }; // end of class 'NumberSpellout

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace 'cce'

#endif // PG_CPP_UTILS_NUMBER_SPELLOUT_H_
