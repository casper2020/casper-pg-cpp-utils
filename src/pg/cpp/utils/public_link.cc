/**
 * @file public_link.cc
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

#include "pg/cpp/utils/public_link.h"

#include "pg/cpp/utils/exception.h"
#include "pg/cpp/utils/b64.h"

#include "osal/osal_time.h"

#include "cppcodec/base64_url_unpadded.hpp"
#include "cppcodec/base64_rfc4648.hpp"

#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>

#include <string.h> // memcpy

#include <map>

// https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption

/**
 * @brief Default constructor.
 *
 * @param a_key
 * @param a_iv
 */
pg::cpp::utils::PublicLink::PublicLink (const std::string& a_key, const std::string& a_iv)
    : key_(a_key), iv_(a_iv)
{
    /* emtpy */
}

/**
 * @brief Destructor.
 */
pg::cpp::utils::PublicLink::~PublicLink ()
{
    /* empty */
}

/**
 * @brief Fill user provided context information.
 *
 * @param a_context
 */
void pg::cpp::utils::PublicLink::FillOutputAtUserFuncContext (FuncCallContext* a_context)
{
    Utility::Records* records = static_cast<Utility::Records*>(a_context->user_fctx);
    records->Append(new pg::cpp::utils::PublicLink::Record(url_));
    a_context->max_calls += 1;
}

/**
 * @brief Calculate a payload hash.
 *
 * @param a_base_url
 * @param a_company_id
 * @param a_entity_type
 * @param a_entity_id
 *
 * @throw
 */
void pg::cpp::utils::PublicLink::Calculate (const std::string& a_base_url,
                                            const int64_t a_company_id, const std::string& a_entity_type, const int64_t a_entity_id)
{
    unsigned char* key  = nullptr;
    unsigned char* iv   = nullptr;
    unsigned char* out  = nullptr;
    int            outl = 0;

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
    EVP_CIPHER_CTX    _ctx;
    EVP_CIPHER_CTX*    ctx = &_ctx;
#else
    EVP_CIPHER_CTX*    ctx = nullptr;
#endif

    try {

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
        EVP_CIPHER_CTX_init(ctx);
#else
        ctx = EVP_CIPHER_CTX_new();
#endif

        //
        // RESET
        //
        tmp_ss_.str("");
        url_ = "";

        //
        // PREPARE PAYLOAD
        //
        osal::Time::HumanReadableTime hr_time;
        osal::Time::GetHumanReadableLocalTimeFrom(hr_time);

        Json::Value object    = Json::Value(Json::ValueType::objectValue);
        object["timestamp"]   = osal::Time::ToHumanReadableTimeISO8601WithTZ(hr_time);
        object["company_id"]  = a_company_id;
        object["entity_type"] = a_entity_type;
        object["entity_id"]   = a_entity_id;

        //
        // int EVP_CIPHER_CTX_set_padding(EVP_CIPHER_CTX *x, int padding);
        //
        // - enables or disables padding;
        // - always returns 1;
        //
        if ( 1 != EVP_CIPHER_CTX_set_padding(ctx, 1) ) {
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
            EVP_CIPHER_CTX_cleanup(ctx);
#else
            EVP_CIPHER_CTX_free(ctx);
#endif
            throw PG_CPP_UTILS_EXCEPTION_NA("Unable to set padding!");
        }

        const std::map<std::string, unsigned char**> map = {
            { key_, &key },
            { iv_ , &iv  }
        };
        for ( auto it : map ) {
            if ( it.first.length() > 0 ) {
                const size_t dl = cppcodec::base64_rfc4648::decoded_max_size(it.first.length());
                (*it.second) = new unsigned char[dl];
                cppcodec::base64_rfc4648::decode((*it.second), dl, it.first.c_str(), it.first.length());
            } else {
                (*it.second) = nullptr;
            }
        }

        const EVP_CIPHER* cipher = EVP_aes_256_cbc();

        //
        // int EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *type, ENGINE *impl, unsigned char *key, unsigned char *iv);
        //
        // - sets up cipher context ctx for encryption with cipher type from ENGINE impl;
        // - return 1 for success and 0 for failure;
        //
        if ( 1 != EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv) ) {
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
            EVP_CIPHER_CTX_cleanup(ctx);
#else
            EVP_CIPHER_CTX_free(ctx);
#endif
            throw PG_CPP_UTILS_EXCEPTION_NA("Unable to initialize cipher!");
        }

        const std::string    payload = fast_writer_.write(object);
        const unsigned char* in      = reinterpret_cast<const unsigned char*>(payload.c_str());
        int                  inl     = static_cast<int>(payload.length());

        out  = new unsigned char[ (inl + EVP_CIPHER_block_size(cipher) - 1) ];
        outl = 0;

        //
        // int EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, unsigned char *in, int inl);
        //
        // - encrypts inl bytes from the buffer in and writes the encrypted version to out;
        // - return 1 for success and 0 for failure;
        //
        if ( 1 != EVP_EncryptUpdate(ctx, out, &outl, in, inl) ) {
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
            EVP_CIPHER_CTX_cleanup(ctx);
#else
            EVP_CIPHER_CTX_free(ctx);
#endif
            throw PG_CPP_UTILS_EXCEPTION_NA("Unable to update encryption!");
        }

        int encrypted_length = outl;

        //
        // int EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl);
        //
        // - encrypts the "final" data, that is any data that remains in a partial block;
        // - return 1 for success and 0 for failure;
        //
        if ( 1 != EVP_EncryptFinal_ex(ctx, out + encrypted_length, &outl) ) {
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
            EVP_CIPHER_CTX_cleanup(ctx);
#else
            EVP_CIPHER_CTX_free(ctx);
#endif
            throw PG_CPP_UTILS_EXCEPTION_NA("Unable to finalize encryption!");
        }
        encrypted_length += outl;

        const std::string b64 = cppcodec::base64_url_unpadded::encode(out, static_cast<size_t>(encrypted_length));

        //
        //
        // int EVP_CIPHER_CTX_cleanup(EVP_CIPHER_CTX *a);
        //
        // - returns 1 for success and 0 for failure.
        //
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
            EVP_CIPHER_CTX_cleanup(ctx);
#else
            EVP_CIPHER_CTX_free(ctx);
#endif
        delete [] out;
        if ( nullptr != key ) {
            delete [] key;
        }
        if ( nullptr != iv ) {
            delete [] iv;
        }

        //
        // set URL
        //
        tmp_ss_ << a_base_url << "/" << b64;
        url_ = tmp_ss_.str();

    } catch (const pg::cpp::utils::Exception& a_pg_cpp_utils_exception) {
        if ( nullptr != out ) {
            delete [] out;
        }
        if ( nullptr != key ) {
            delete [] key;
        }
        if ( nullptr != iv ) {
            delete [] iv;
        }
        throw a_pg_cpp_utils_exception;
    } catch (const Json::Exception& a_json_exception) {
        if ( nullptr != out ) {
            delete [] out;
        }
        if ( nullptr != key ) {
            delete [] key;
        }
        if ( nullptr != iv ) {
            delete [] iv;
        }
        throw PG_CPP_UTILS_EXCEPTION("%s", a_json_exception.what());
    }

}
