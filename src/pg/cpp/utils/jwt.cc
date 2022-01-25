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
 */
pg::cpp::utils::JWT::JWT ()
    : jwt_("pg-cpp-utils")
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
 * @brief Create a JWT.
 *
 * @param a_duration
 * @param a_payload
 * @param a_pkey_uri
 *
 * @throw
 */
void pg::cpp::utils::JWT::Encode (const uint64_t& a_duration, const std::string& a_payload, const std::string& a_pkey_uri)
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
        payload_ = jwt_.Encode(a_duration, a_pkey_uri);
    } catch (...) {
        try {
            ::cc::Exception::Rethrow(/* a_unhandled */ false, __FILE__, __LINE__, __FUNCTION__);
        } catch (const ::cc::Exception& a_cc_exception) {
            throw PG_CPP_UTILS_EXCEPTION("%s", a_cc_exception.what());
        }
    }
}

/**
 * @brief Create a 'slashy' JWT link.
 *
 * @param a_base_url
 * @param a_jwt
 *
 * @throw
 */
void pg::cpp::utils::JWT::Slashy (const std::string& a_base_url, const std::string& a_jwt)
{    
    try {
        // ... split ...
        const char* header_ptr = a_jwt.c_str();
        if ( '\0' == header_ptr[0] ) {
            throw ::cc::Exception("%s", "Invalid JWT format - empty header!");
        }
        const char* payload_ptr = strchr(header_ptr, '.');
        if ( nullptr == payload_ptr ) {
            throw ::cc::Exception("%s", "Invalid JWT format - missing or invalid payload!");
        }
        payload_ptr += sizeof(char);
        const char* signature_ptr = strchr(payload_ptr, '.');
        if ( nullptr == signature_ptr ) {
            throw ::cc::Exception("%s", "Invalid JWT format - missing or invalid signature!");
        }
        signature_ptr += sizeof(char);
        ssize_t remaining = static_cast<ssize_t>(a_jwt.length());
        if ( remaining < 40 ) {
            throw ::cc::Exception("%s", "Invalid JWT format - invalid length!");
        }
        // ... set result
        const char* ptr = a_jwt.c_str();
        payload_ = a_base_url;
        if ( '/' != payload_[payload_.length() - 1] ) {
            payload_ += '/';
        } 
        payload_ += std::string(ptr, 40);
        remaining += 40;
        ptr       += 40;
        while ( remaining >= 100 ) {
            payload_ += '/' + std::string(ptr, 100) ;
            ptr += 100;
            remaining -= 100;
        }
        if ( remaining > 0 ) {
            payload_ += '/' +  std::string(ptr, remaining);
        }        
    } catch (...) {
        try {
            ::cc::Exception::Rethrow(/* a_unhandled */ false, __FILE__, __LINE__, __FUNCTION__);
        } catch (const ::cc::Exception& a_cc_exception) {
            throw PG_CPP_UTILS_EXCEPTION("%s", a_cc_exception.what());
        }
    }
}