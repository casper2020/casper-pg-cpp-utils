/**
 * @file number_spellout.cc
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

#include "pg/cpp/utils/number_spellout.h"

#include "pg/cpp/utils/exception.h"

#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>

#include "unicode/msgfmt.h"

/**
 * @brief Default constructor.
 *
 * @param a_locale
 */
pg::cpp::utils::NumberSpellout::NumberSpellout (const std::string& a_locale, const std::string& a_spellout_override)
    : icu_locale_(U_ICU_NAMESPACE::Locale::createFromName(a_locale.c_str()))
{
    icu_error_code_ = UErrorCode::U_ZERO_ERROR;
    if ( 0 == a_spellout_override.length() ) {
        icu_number_format_ = new U_ICU_NAMESPACE::RuleBasedNumberFormat(U_ICU_NAMESPACE::URBNFRuleSetTag::URBNF_SPELLOUT, icu_locale_, icu_error_code_);
    } else {
        icu_number_format_ = new U_ICU_NAMESPACE::RuleBasedNumberFormat(a_spellout_override.c_str(), icu_locale_, icu_parse_error_, icu_error_code_);
    }
    if ( not ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) ) {
      error_ = "ICU version:" + std::string(U_ICU_VERSION) + " - an error occurred while initializing RuleBasedNumberFormat: " + std::to_string(icu_error_code_);
    }
}

/**
 * @brief Destructor.
 */
pg::cpp::utils::NumberSpellout::~NumberSpellout ()
{
    delete icu_number_format_;
}

/**
 * @brief Fill user provided context information.
 *
 * @param a_context
 */
void pg::cpp::utils::NumberSpellout::FillOutputAtUserFuncContext (FuncCallContext* a_context)
{
    Utility::Records* records = static_cast<Utility::Records*>(a_context->user_fctx);
    records->Append(new pg::cpp::utils::NumberSpellout::Record(string_));
    a_context->max_calls += 1;
}

/**
 * @brief Convert a number to words.
 *
 * @param a_number
 *
 * @throw
 */
void pg::cpp::utils::NumberSpellout::Spellout (double a_number)
{
    string_ = "";
    if ( not ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) ) {
	return;
    }
    if ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) {
        U_ICU_NAMESPACE::UnicodeString unicode_string;
        icu_number_format_->format(a_number, unicode_string);
        unicode_string.toUTF8String(string_);
    } else {
        error_ = "ICU version:" + std::string(U_ICU_VERSION) + " - an error occurred while calling icu number format function:" + std::to_string(icu_error_code_);
    }
}

/**
 * @brief Convert a currency number to words.
 *
 * @param a_number
 *
 * @throw
 */
void pg::cpp::utils::NumberSpellout::CurrencySpellout (double a_major, const std::string& a_major_singular, const std::string& a_major_plural,
                                            double a_minor, const std::string& a_minor_singular, const std::string& a_minor_plural,
                                            const std::string& a_format)
{
    Spellout(a_major);
    if ( not ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) ) {
      return;
    }
    const std::string major_str = string_;
    Spellout(a_minor);
    if ( not ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) ) {
      return;
    }
    const std::string minor_str = string_;

    const U_ICU_NAMESPACE::Formattable arguments[] = {
        a_major, a_major_singular.c_str(), a_major_plural.c_str(), major_str.c_str(),
        a_minor, a_minor_singular.c_str(), a_minor_plural.c_str(), minor_str.c_str()
    };

    U_ICU_NAMESPACE::UnicodeString unicode_string;
    U_ICU_NAMESPACE::MessageFormat::format(a_format.c_str(), arguments, 8, unicode_string, icu_error_code_);

    string_ = "";
    error_  = "";
    if ( U_ZERO_ERROR == icu_error_code_ || U_USING_DEFAULT_WARNING == icu_error_code_ ) {
        unicode_string.toUTF8String(string_);
    } else {
      error_ = "ICU version:" + std::string(U_ICU_VERSION) + " - an error occurred while calling icu message format: " + std::to_string(icu_error_code_);
    }
}
