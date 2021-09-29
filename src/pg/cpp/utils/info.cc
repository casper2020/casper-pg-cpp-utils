/**
 * @file info.cc
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

#include "pg/cpp/utils/info.h"

#include "pg/cpp/utils/versioning.h"

#include <openssl/crypto.h> // SSLeay_version // SSLEAY_VERSION
#include "cc/icu/includes.h" // U_ICU_VERSION

/**
 * @brief Default constructor.
 */
pg::cpp::utils::Info::Info ()
{
    /* emtpy */
}

/**
 * @brief Destructor.
 */
pg::cpp::utils::Info::~Info ()
{
    /* empty */
}

/**
 * @brief Fill user provided context information.
 *
 * @param a_context
 */
void pg::cpp::utils::Info::FillOutputAtUserFuncContext (FuncCallContext* a_context)
{
    const std::string openssl = std::string(SSLeay_version(SSLEAY_VERSION));
    const std::string icu     = "ICU " + std::string(U_ICU_VERSION);
    
    Utility::Records* records = static_cast<Utility::Records*>(a_context->user_fctx);
    records->Append(new pg::cpp::utils::Info::Record(
        PG_CPP_UTILS_VERSION,
        PG_CPP_UTILS_TARGET,
        PG_CPP_UTILS_REL_DATE,
        PG_CPP_UTILS_REL_BRANCH "@" PG_CPP_UTILS_REL_HASH,
        openssl + "; " + icu
    ));
    a_context->max_calls += 1;
}
