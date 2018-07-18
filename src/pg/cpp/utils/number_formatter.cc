/**
 * @file number_formatter.cc
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

#include "pg/cpp/utils/number_formatter.h"

#include "pg/cpp/utils/exception.h"

/**
 * @brief Default constructor.
 *
 * @param a_locale
 */
pg::cpp::utils::NumberFormatter::NumberFormatter (const std::string& a_locale)
    : pg::cpp::utils::MessageFormatter(a_locale.c_str()),
      icu_number_format_(icu_error_code_)
{
    icu_number_format_.setRoundingMode(U_ICU_NAMESPACE::DecimalFormat::kRoundUp); // Unnecessary);

    U_ICU_NAMESPACE::DecimalFormatSymbols symbols(icu_locale_, icu_error_code_);
    if ( U_ZERO_ERROR == icu_error_code_ || U_USING_FALLBACK_WARNING == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) {
        if ( a_locale == "pt_PT" || a_locale == "pt-PT" ) {
	  symbols.setSymbol(U_ICU_NAMESPACE::DecimalFormatSymbols::kGroupingSeparatorSymbol, ".", true);
        }
        icu_number_format_.setDecimalFormatSymbols(symbols);
        icu_error_code_ = U_ZERO_ERROR;
    } else {
        throw PG_CPP_UTILS_EXCEPTION("Locale '%s' is not supported", a_locale.c_str());
    }
}

/**
 * @brief Destructor.
 */
pg::cpp::utils::NumberFormatter::~NumberFormatter ()
{
    /* empty */
}

/**
 * @brief Convert a number to words.
 *
 * @param a_number
 * @param a_pattern
 *
 * @throw
 */
void pg::cpp::utils::NumberFormatter::Format (double a_number, const std::string& a_pattern)
{
    string_ = "";
    error_  = "";

    if ( not ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) ) {
        error_ = std::to_string(icu_error_code_);
        return;
    }

    icu_number_format_.applyPattern(U_ICU_NAMESPACE::UnicodeString::fromUTF8(a_pattern), icu_error_code_);
    if ( not ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) ) {
        error_ = std::to_string(icu_error_code_);
        return;
    }

    U_ICU_NAMESPACE::UnicodeString unicode_string;

    unicode_string = icu_number_format_.format(a_number, unicode_string);
    unicode_string.toUTF8String(string_);
}
