/**
 * @file invoice_hash.cc
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

#include "pg/cpp/utils/invoice_hash.h"

#include "pg/cpp/utils/exception.h"
#include "pg/cpp/utils/b64.h"

#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>

#include <stdio.h> // strerror

/**
 * @brief Default constructor.
 *
 * @param a_pem_uri
 */
pg::cpp::utils::InvoiceHash::InvoiceHash (const std::string& a_pem_uri)
    : pem_uri_(a_pem_uri)
{
    /* emtpy */
}

/**
 * @brief Destructor.
 */
pg::cpp::utils::InvoiceHash::~InvoiceHash ()
{
    /* empty */
}

/**
 * @brief Fill user provided context information.
 *
 * @param a_context
 */
void pg::cpp::utils::InvoiceHash::FillOutputAtUserFuncContext (FuncCallContext* a_context)
{
    Utility::Records* records = static_cast<Utility::Records*>(a_context->user_fctx);
    records->Append(new pg::cpp::utils::InvoiceHash::Record(long_, short_));
    a_context->max_calls += 1;
}

/**
 * @brief Calculate a payload hash.
 *
 * @param a_payload
 *
 * @throw
 */
void pg::cpp::utils::InvoiceHash::Calculate (const std::string& a_payload)
{
    bool           ctx_initialized  = false;
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
    EVP_MD_CTX    _ctx;
    EVP_MD_CTX*    ctx = &_ctx;
#else
    EVP_MD_CTX*    ctx = nullptr;
#endif
    EVP_PKEY*      pkey             = nullptr;
    RSA*           rsa_pkey         = nullptr;
    FILE*          private_key_file = nullptr;
    unsigned char* signature_bytes  = nullptr;
    unsigned int   signature_len    = 0;
    pg::cpp::utils::B64       b64;

    const auto cleanup = [&pkey, &private_key_file, &signature_bytes, &ctx_initialized, ctx] () {

        if ( nullptr != signature_bytes ) {
            delete [] signature_bytes;
        }

        if ( nullptr != pkey ) {
            EVP_PKEY_free(pkey);
        }

        if ( nullptr != private_key_file ) {
            fclose(private_key_file);
        }

        if ( true == ctx_initialized ) {
#if OPENSSL_VERSION_NUMBER < 0x1010000fL
            EVP_MD_CTX_cleanup(ctx);
#else
            EVP_MD_CTX_free(ctx);
#endif
            ctx_initialized = false;
        }

    };

    long_  = "";
    short_ = "";
    tmp_ss_.str("");

    try {

        pkey             = EVP_PKEY_new();
        private_key_file = fopen(pem_uri_.c_str(), "r");
        if ( nullptr == private_key_file ) {
            int err = errno;
            throw PG_CPP_UTILS_EXCEPTION("Unable to open RSA private key file '%s' : %s!",
                                         pem_uri_.c_str(), strerror(err)
            );
        }

        if ( ! PEM_read_RSAPrivateKey(private_key_file, &rsa_pkey, NULL, NULL) ) {
            throw PG_CPP_UTILS_EXCEPTION_NA("Error while loading RSA private key File!");
        }

        if ( ! EVP_PKEY_assign_RSA(pkey, rsa_pkey) ) {
            throw PG_CPP_UTILS_EXCEPTION_NA("Error while assigning RSA!");
        }

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
        EVP_MD_CTX_init(ctx);
#else
        ctx = EVP_MD_CTX_new();
#endif
        ctx_initialized = true;

        if ( ! EVP_SignInit(ctx, EVP_sha1()) ) {
            throw PG_CPP_UTILS_EXCEPTION_NA("Error while setting up signing context!");
        }

        if ( ! EVP_SignUpdate(ctx, a_payload.c_str(), a_payload.length() ) ) {
            throw PG_CPP_UTILS_EXCEPTION_NA("Error while updating signing context");
        }

        signature_bytes = new unsigned char[static_cast<size_t>(EVP_PKEY_size(pkey))];
        if ( !EVP_SignFinal(ctx, signature_bytes, &signature_len, pkey) ) {
            throw PG_CPP_UTILS_EXCEPTION_NA("Error while finalizing signing context!");
        }

        long_ = b64.Encode(signature_bytes, signature_len);

        const size_t hash_length = long_.length();
        if ( hash_length != 172 ) {
            throw PG_CPP_UTILS_EXCEPTION("Error while encoding signature to B64 - got %zd (bytes), expected %d bytes!",
                                         hash_length,
                                         172
            );
        }

        const char* const long_c_str = long_.c_str();
        tmp_ss_ << long_c_str[0] << long_c_str[10] << long_c_str[20] << long_c_str[30];
        short_ = tmp_ss_.str();

        cleanup();

    } catch (const pg::cpp::utils::Exception& a_pg_cpp_utils_exception) {
        cleanup();
        throw a_pg_cpp_utils_exception;
    }  catch (const std::bad_alloc& a_bad_alloc) {
        cleanup();
        throw PG_CPP_UTILS_EXCEPTION("C++ Bad Alloc: %s", a_bad_alloc.what());
    } catch (const std::runtime_error& a_rte) {
        cleanup();
        throw PG_CPP_UTILS_EXCEPTION("C++ Runtime Error: %s", a_rte.what());
    } catch (const std::exception& a_std_exception) {
        cleanup();
        throw PG_CPP_UTILS_EXCEPTION("C++ Standard Exception: %s", a_std_exception.what());
    } catch (...) {
        cleanup();
        throw PG_CPP_UTILS_EXCEPTION("C++ Generic Exception: %s", PG_CPP_UTILS_EXCEPTION_TRACE_CPP_GENERIC_EXCEPTION().c_str());
    }

}
