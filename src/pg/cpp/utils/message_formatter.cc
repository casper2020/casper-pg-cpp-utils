/**
 * @file message_formatter.cc
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

#include "pg/cpp/utils/message_formatter.h"

#include "pg/cpp/utils/exception.h"

#include "unicode/msgfmt.h"

/**
 * @brief Default constructor.
 *
 * @param a_locale
 */
pg::cpp::utils::MessageFormatter::MessageFormatter (const std::string& a_locale)
    : icu_locale_(U_ICU_NAMESPACE::Locale::createFromName(a_locale.c_str())),
      icu_error_code_(UErrorCode::U_ZERO_ERROR)
{
    /* empty */
}

/**
 * @brief Destructor.
 */
pg::cpp::utils::MessageFormatter::~MessageFormatter ()
{
    /* empty */
}

/**
 * @brief Fill user provided context information.
 *
 * @param a_context
 */
void pg::cpp::utils::MessageFormatter::FillOutputAtUserFuncContext (FuncCallContext* a_context)
{
    Utility::Records* records = static_cast<Utility::Records*>(a_context->user_fctx);
    records->Append(new pg::cpp::utils::MessageFormatter::Record(string_));
    a_context->max_calls += 1;
}

/**
 * @brief Convert a number to words.
 *
 * @param a_format
 * @param a_args
 *
 * @throw
 */
void pg::cpp::utils::MessageFormatter::Format (const std::string& a_format, const std::vector<std::string>& a_args)
{
    string_ = "";
    error_  = "";

    if ( 0 == a_args.size() ) {
        string_ = a_format;
        return;
    }

    if ( not ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) ) {
        error_ = std::to_string(icu_error_code_);
        return;
    }

    U_ICU_NAMESPACE::UnicodeString unicode_string;

    U_ICU_NAMESPACE::Formattable* args = new U_ICU_NAMESPACE::Formattable[a_args.size()];
    for ( size_t idx = 0 ; idx < a_args.size() ; ++idx ) {
        args[idx] = U_ICU_NAMESPACE::Formattable(a_args[idx].c_str());
    }

    U_ICU_NAMESPACE::MessageFormat::format(a_format.c_str(),
        args, static_cast<int32_t>(a_args.size()),
        unicode_string, icu_error_code_
    );

    delete [] args;

    if ( not ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) ) {
        error_ = std::to_string(icu_error_code_);
        return;
    }

    unicode_string.toUTF8String(string_);
}
