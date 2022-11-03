/**
 * @file number_formatter.h
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
#ifndef PG_CPP_UTILS_NUMBER_FORMATTER_H_
#define PG_CPP_UTILS_NUMBER_FORMATTER_H_

#include "pg/cpp/utils/message_formatter.h"

#include <string>             // std::string

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"

#include "unicode/decimfmt.h" // U_ICU_NAMESPACE::DecimalFormat

#pragma clang diagnostic pop

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class NumberFormatter final : public MessageFormatter
            {

            private: // Data

                U_ICU_NAMESPACE::DecimalFormat icu_number_format_;

            public: // Constructor / Destructor.

                NumberFormatter (const std::string& a_locale);
                virtual ~NumberFormatter();

            public: // Method(s) / Function(s)

                void Format (double a_number, const std::string& a_pattern);

            }; // end of class 'NumberSpellout

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace 'cce'

#endif // PG_CPP_UTILS_NUMBER_FORMATTER_H_
