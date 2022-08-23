/**
 * @file locales.cc
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

#include "pg/cpp/utils/locales.h"

#include "pg/cpp/utils/versioning.h"

#include <openssl/crypto.h> // SSLeay_version // SSLEAY_VERSION
#include "cc/icu/includes.h" // U_ICU_VERSION

#include "json/json.h"

/**
 * @brief Default constructor.
 */
pg::cpp::utils::Locales::Locales ()
{
    /* emtpy */
}

/**
 * @brief Destructor.
 */
pg::cpp::utils::Locales::~Locales ()
{
    /* empty */
}

/**
 * @brief Fill user provided context information.
 *
 * @param a_context
 */
void pg::cpp::utils::Locales::FillOutputAtUserFuncContext (FuncCallContext* a_context)
{
    int32_t count;
    const U_ICU_NAMESPACE::Locale* locales = U_ICU_NAMESPACE::Locale::getAvailableLocales(count);

    Json::Value      array = Json::Value(Json::ValueType::arrayValue);
    Json::FastWriter fw; fw.omitEndingLineFeed();

    for ( int32_t idx = 0 ; idx < count ; ++idx ) {
        const auto locale = locales[idx];
        array.append(Json::Value(locale.getName()));
    }

    Utility::Records* records = static_cast<Utility::Records*>(a_context->user_fctx);
    records->Append(new pg::cpp::utils::Locales::Record(std::string(U_ICU_VERSION), fw.write(array)));
    a_context->max_calls += 1;
}
