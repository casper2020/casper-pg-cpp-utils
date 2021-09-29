/**
 * @file jwt.cc
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

#include "pg/cpp/utils/jwt.h"

#include "pg/cpp/utils/exception.h"

#include "cc/exception.h"
#include "cc/easy/json.h"

/**
 * @brief Default constructor.
 *
 * @param a_pkey_uri
 */
pg::cpp::utils::JWT::JWT (const std::string& a_pkey_uri)
    : pkey_uri_(a_pkey_uri), jwt_("pg-cpp-utils")
{
    /* empty */
}

/**
 * @brief Destructor.
 */
pg::cpp::utils::JWT::~JWT ()
{
    /* empty */
}

/**
 * @brief Fill user provided context information.
 *
 * @param a_context
 */
void pg::cpp::utils::JWT::FillOutputAtUserFuncContext (FuncCallContext* a_context)
{
    Utility::Records* records = static_cast<Utility::Records*>(a_context->user_fctx);
    records->Append(new pg::cpp::utils::JWT::Record(payload_));
    a_context->max_calls += 1;
}

/**
 * @brief Calculate a payload hash.
 *
 * @param a_duration
 * @param a_payload
 *
 * @throw
 */
void pg::cpp::utils::JWT::Encode (const uint64_t& a_duration, const std::string& a_payload)
{    
    const cc::easy::JSON<::cc::Exception> json;
    try {
        // ... parse payload ...
        ::Json::Value payload;
        json.Parse(a_payload, payload);
        // ... prepare JWT ...
        for ( auto member : payload.getMemberNames() ) {
            jwt_.SetUnregisteredClaim(member, payload[member]);
        }
        payload_ = jwt_.Encode(a_duration, pkey_uri_);
    } catch (...) {
        try {
            ::cc::Exception::Rethrow(/* a_unhandled */ false, __FILE__, __LINE__, __FUNCTION__);
        } catch (const ::cc::Exception& a_cc_exception) {
            throw PG_CPP_UTILS_EXCEPTION("%s", a_cc_exception.what());
        }
    }
}
