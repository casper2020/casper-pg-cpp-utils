/**
 * @file pg-cpp-utils.cc
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

extern "C" {
    #include "pg/postgres.h"
    #include "access/tupmacs.h"
    #include "utils/builtins.h"
}

#include <inttypes.h>

// C++ headers
#include <string>

#include "pg/cpp/utils/version.h"
#include "pg/cpp/utils/info.h"
#include "pg/cpp/utils/exception.h"
#include "pg/cpp/utils/jwt.h"
#include "pg/cpp/utils/invoice_hash.h"
#include "pg/cpp/utils/public_link.h"
#include "pg/cpp/utils/number_spellout.h"
#include "pg/cpp/utils/number_formatter.h"
#include "pg/cpp/utils/message_formatter.h"

#include <unicode/utypes.h> // u_init
#include <unicode/uclean.h> // u_cleanup

#include <openssl/ssl.h>
#include <openssl/err.h>

extern "C" {
    PG_MODULE_MAGIC;    
    Datum pg_cpp_utils_make_jwt(PG_FUNCTION_ARGS);
    Datum pg_cpp_utils_invoice_hash(PG_FUNCTION_ARGS);
    Datum pg_cpp_utils_number_spellout(PG_FUNCTION_ARGS);
    Datum pg_cpp_utils_version(PG_FUNCTION_ARGS);
    Datum pg_cpp_utils_info(PG_FUNCTION_ARGS);
    PG_FUNCTION_INFO_V1(pg_cpp_utils_make_jwt);
    PG_FUNCTION_INFO_V1(pg_cpp_utils_invoice_hash);
    PG_FUNCTION_INFO_V1(pg_cpp_utils_public_link);
    PG_FUNCTION_INFO_V1(pg_cpp_utils_number_spellout);
    PG_FUNCTION_INFO_V1(pg_cpp_utils_currency_spellout);
    PG_FUNCTION_INFO_V1(pg_cpp_utils_format_number);
    PG_FUNCTION_INFO_V1(pg_cpp_utils_format_message);
    PG_FUNCTION_INFO_V1(pg_cpp_utils_version);
    PG_FUNCTION_INFO_V1(pg_cpp_utils_info);
} // extern "C"

#if defined(DEBUG)
    #ifndef PG_CPP_UTILS_TOKEN
        #define PG_CPP_UTILS_TOKEN "pg_cpp_utils"
    #endif
    #ifndef PG_CPP_UTILS_LOG_MSG
        #define PG_CPP_UTILS_LOG_MSG(a_format, ...) \
            ereport(DEBUG3, (errmsg_internal(PG_CPP_UTILS_TOKEN ": " a_format, __VA_ARGS__)));
    #endif
    #ifndef PG_CPP_UTILS_LOG_DEBUG
        #define PG_CPP_UTILS_LOG_DEBUG(a_format, ...) \
            ereport(DEBUG3, (errmsg_internal(PG_CPP_UTILS_TOKEN ": " a_format, __VA_ARGS__)));
    #endif
#else
    #undef PG_CPP_UTILS_TOKEN
    #undef PG_CPP_UTILS_LOG_MSG
    #define PG_CPP_UTILS_LOG_MSG(a_format, ...)
    #undef PG_CPP_UTILS_LOG_DEBUG
    #define PG_CPP_UTILS_LOG_DEBUG(a_format, ...)
#endif

extern "C" {

    /*
     * DEBUG INFO:
     *
     * - Get backend PID - SELECT pg_backend_pid();
     */

    /**
     * @brief SEE interface to PostreSQL
     */
    Datum pg_cpp_utils_utils_common (FunctionCallInfo fcinfo,
                                     const std::function<pg::cpp::utils::Utility*()> a_alloc_utility_func,
                                     const std::function<void(pg::cpp::utils::Utility*)> a_perform_func,
                                     const std::function<pg::cpp::utils::Utility*(pg::cpp::utils::Utility*)> a_dealloc_utility_func
                                     )
    {

        FuncCallContext* func_call_context;

        if ( SRF_IS_FIRSTCALL() ) {

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
            OPENSSL_config(NULL);
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();
#else
            if ( 0 == OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG, NULL) ) {
                // ... report error ....
                ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED), errmsg("%s", "Unable to initialize openssl!")));
            }
            /*  OPENSSL_init_ssl() may leave errors in the error queue while returning success */
            ERR_clear_error();
#endif

            // ... create a function context for cross-call persistence ...
            func_call_context = SRF_FIRSTCALL_INIT();

            // ... switch to memory context appropriate for multiple function calls ...
            MemoryContext old_context = MemoryContextSwitchTo(func_call_context->multi_call_memory_ctx);

            // ... build a tuple descriptor for our result type ...
            TupleDesc tupdesc;
            const TypeFuncClass return_type = get_call_result_type(fcinfo, NULL, &tupdesc);
            if ( TYPEFUNC_COMPOSITE != return_type ) {
                // ... restore context ...
                MemoryContextSwitchTo(old_context);
                // ... report error ....
                ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED), errmsg("Expecting result type %d ( TYPEFUNC_COMPOSITE ) not %d!", TYPEFUNC_COMPOSITE, return_type)));
            }
            func_call_context->attinmeta = TupleDescGetAttInMetadata(tupdesc);

            // ... create utility ...
            pg::cpp::utils::Utility* utility = nullptr;
            try {
                utility = a_alloc_utility_func();
                // ... perform ...
                a_perform_func(utility);
                const std::string& error = utility->LastError();
                if ( error.length() > 0 ) {
                    throw PG_CPP_UTILS_EXCEPTION("error code %s", error.c_str());
                }
                // ... allocate user func context ...
                pg::cpp::utils::Utility::AllocUserFuncContext(func_call_context);
                // ... set result data to function call context ...
                utility->FillOutputAtUserFuncContext(func_call_context);
            } catch (const pg::cpp::utils::Exception& a_pg_cpp_utils_exception) {
                // ... release utility ...
                a_dealloc_utility_func(utility);
                // ... dealloc user func context ...
                pg::cpp::utils::Utility::DeallocUserFuncContext(func_call_context);
                // ... report error ...
                ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED), errmsg("%s", a_pg_cpp_utils_exception.what())));
            } catch (...) {
                // ... release utility ...
                a_dealloc_utility_func(utility);
                // ... dealloc user func context ...
                pg::cpp::utils::Utility::DeallocUserFuncContext(func_call_context);
                // ... report error ...
                ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED), errmsg("Unexpected exception generic caught!")));
            }

            utility = a_dealloc_utility_func(utility);

            // ... restore context ...
            MemoryContextSwitchTo(old_context);
        }

        // ... stuff done on every call of the function ...
        func_call_context = SRF_PERCALL_SETUP();

#ifdef __APPLE__
    #define CALL_CNTR_FMT PRIu32
#else
    #define CALL_CNTR_FMT PRIu64
#endif

        // ... if still data rows to return ....
        if ( func_call_context->call_cntr < func_call_context->max_calls ) {

            // ... access user function call data ...
            pg::cpp::utils::Utility::Records* records = static_cast<pg::cpp::utils::Utility::Records*>(func_call_context->user_fctx);

            // ... pick current row values ...
            char** values = records->GetPStringValues(func_call_context->call_cntr);
            // ... no data?
            if ( nullptr == values ) {
                // ... for debug proposes only ...
                PG_CPP_UTILS_LOG_DEBUG("----- CALL #%" CALL_CNTR_FMT " ----- NO DATA", func_call_context->call_cntr);
                // ... next ...
                SRF_RETURN_NEXT_NULL(func_call_context);
            }

            // ... for debug proposes only ...
            PG_CPP_UTILS_LOG_DEBUG("----- CALL #%" CALL_CNTR_FMT " ----- [%zd]: %s", func_call_context->call_cntr, strlen(values[0]), values[0]);

            //... build a tuple ....
            HeapTuple tuple = BuildTupleFromCStrings(func_call_context->attinmeta, values);

            //... make the tuple into a datum ...
            Datum result = HeapTupleGetDatum(tuple);

            // ... release record memory now ...
            records->Release(func_call_context->call_cntr, values);
            // ... next ...
            SRF_RETURN_NEXT(func_call_context, result);
        } else {
            // ... dealloc user func context ...
            pg::cpp::utils::Utility::DeallocUserFuncContext(func_call_context);
            // ... for debug proposes only ...
            PG_CPP_UTILS_LOG_DEBUG("----- %s -----", "FINAL_CALL");
            // ... we're done ...
            SRF_RETURN_DONE(func_call_context);
        }

    }

    /**
     * @brief pg-cpp-utils JWT interface to PostreSQL
     */
    Datum pg_cpp_utils_make_jwt (PG_FUNCTION_ARGS)
    {
        // ... test the number of arguments ...
        const size_t args_count = PG_NARGS();
        if ( 3 != args_count ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_make_jwt(...) - received %zd argument(s), expected at least %d argument(s)!", args_count, 6)
                    )
            );
        }

        if ( PG_ARGISNULL(0) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_make_jwt(...) - payload argument can not be null!")
                     )
            );
        }

        if ( PG_ARGISNULL(1) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_make_jwt(...) - duration id argument can not be null!")
                     )
            );
        }

        if ( PG_ARGISNULL(2) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_make_jwt(...) - private key uri argument can not be null!")
                    )
            );
        }

        // ... collect param(s) ...
        text*   tmp_payload  = PG_GETARG_TEXT_P(0);
        int     tmp_duration = PG_GETARG_INT32(1);
        text*   tmp_pkey_uri = PG_GETARG_TEXT_P(2);

        PG_CPP_UTILS_LOG_DEBUG("%s,%d,%s", tmp_payload, tmp_duration, tmp_pkey_uri);

        if ( nullptr == tmp_payload ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_make_jwt(...) - b) payload argument can not be null!")
                    )
            );
        }

        if ( nullptr == tmp_pkey_uri ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_make_jwt(...) - b) private key uri argument can not be null!")
                    )
            );
        }    

        const std::string payload   = std::string(VARDATA(tmp_payload), VARSIZE(tmp_payload) - VARHDRSZ);
        const uint64_t     duration = static_cast<uint64_t>(tmp_duration);
        const std::string pkey_uri  = std::string(VARDATA(tmp_pkey_uri), VARSIZE(tmp_pkey_uri) - VARHDRSZ);

        // ... perform request ...
        return pg_cpp_utils_utils_common(fcinfo,
                                         /* allocation */
                                         [&pkey_uri] () -> pg::cpp::utils::Utility* {
                                             return new pg::cpp::utils::JWT(pkey_uri);
                                         },
                                         /* execute */
                                         [&payload, &duration] (pg::cpp::utils::Utility* a_utility) -> void {
                                             // ... perform ...
                                             static_cast<pg::cpp::utils::JWT*>(a_utility)->Encode(duration, payload);
                                         },
                                         /* dealloc */
                                         [] (pg::cpp::utils::Utility* a_utility) -> pg::cpp::utils::Utility* {
                                             delete a_utility;
                                             return nullptr;
                                         }
        );
    }

    /**
     * @brief pg-cpp-utils invoice hash interface to PostreSQL
     *
     * http://info.portaldasfinancas.gov.pt/NR/rdonlyres/89DB70CE-7BB5-417B-B13E-C72A912FF66E/0/Despacho_n_8632_2014_03_07.pdf
     */
    Datum pg_cpp_utils_invoice_hash (PG_FUNCTION_ARGS)
    {
        // ... test the number of arguments ...
        const size_t args_count = PG_NARGS();
        if ( 2 != args_count ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_invoice_hash(...) - received %zd argument(s), expected at least %d argument(s)!", args_count, 2)
                    )
            );
        }

        // ... collect param(s) ...
        text* tmp_pem_uri         = PG_GETARG_TEXT_P(0);
        text* tmp_payload         = PG_GETARG_TEXT_P(1);

        if ( nullptr == tmp_pem_uri ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_invoice_hash(...) - pem uri argument can not be null!")
                     )
            );
        }

        if ( nullptr == tmp_payload ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_invoice_hash(...) - payload argument can not be null!")
                    )
            );
        }

        const std::string pem_uri = std::string(VARDATA(tmp_pem_uri) , VARSIZE(tmp_pem_uri)  - VARHDRSZ);
        const std::string payload = std::string(VARDATA(tmp_payload) , VARSIZE(tmp_payload)  - VARHDRSZ);

        // ... perform request ...
        return pg_cpp_utils_utils_common(fcinfo,
                                /* allocation */
                                [&pem_uri] () -> pg::cpp::utils::Utility* {
                                    return new pg::cpp::utils::InvoiceHash(pem_uri);
                                },
                                /* execute */
                                [&payload] (pg::cpp::utils::Utility* a_utility) -> void {
                                    // ... perform ...
                                    static_cast<pg::cpp::utils::InvoiceHash*>(a_utility)->Calculate(payload);
                                },
                                /* dealloc */
                                [] (pg::cpp::utils::Utility* a_utility) -> pg::cpp::utils::Utility* {
                                    delete a_utility;
                                    return nullptr;
                                }
        );
    }

    /**
     * @brief pg-cpp-utils invoice hash interface to PostreSQL
     */
    Datum pg_cpp_utils_public_link (PG_FUNCTION_ARGS)
    {
        // ... test the number of arguments ...
        const size_t args_count = PG_NARGS();
        if ( 6 != args_count ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_public_link(...) - received %zd argument(s), expected at least %d argument(s)!", args_count, 6)
                    )
            );
        }

        if ( PG_ARGISNULL(1) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_public_link(...) - company id argument can not be null!")
                     )
            );
        }

        if ( PG_ARGISNULL(3) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_public_link(...) - entity id argument can not be null!")
                    )
            );
        }

        // ... collect param(s) ...
        text*  tmp_base_url        = PG_GETARG_TEXT_P(0);
        float8 tmp_company_id      = PG_GETARG_FLOAT8(1);
        text*  tmp_entity_type     = PG_GETARG_TEXT_P(2);
        float8 tmp_entity_id       = PG_GETARG_FLOAT8(3);
        text*  tmp_key             = PG_GETARG_TEXT_P(4);
        text*  tmp_iv              = PG_GETARG_TEXT_P(5);

        if ( nullptr == tmp_base_url ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_invoice_hash(...) - base url argument can not be null!")
                    )
            );
        }

        if ( nullptr == tmp_entity_type ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_public_link(...) - entity type argument can not be null!")
                    )
            );
        }

        if ( nullptr == tmp_key ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_public_link(...) - key argument can not be null!")
                    )
            );
        }

        if ( nullptr == tmp_iv ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_public_link(...) - iv argument can not be null!")
                    )
            );
        }

        const int64_t company_id      = static_cast<int64_t>(tmp_company_id);
        const int64_t entity_id       = static_cast<int64_t>(tmp_entity_id);

        const std::string base_url    = std::string(VARDATA(tmp_base_url)   , VARSIZE(tmp_base_url)    - VARHDRSZ);
        const std::string entity_type = std::string(VARDATA(tmp_entity_type), VARSIZE(tmp_entity_type) - VARHDRSZ);
        const std::string key         = std::string(VARDATA(tmp_key)        , VARSIZE(tmp_key)         - VARHDRSZ);
        const std::string iv          = std::string(VARDATA(tmp_iv)         , VARSIZE(tmp_iv)          - VARHDRSZ);

        // ... perform request ...
        return pg_cpp_utils_utils_common(fcinfo,
                                         /* allocation */
                                         [&key, &iv] () -> pg::cpp::utils::Utility* {
                                             return new pg::cpp::utils::PublicLink(key, iv);
                                         },
                                         /* execute */
                                         [&base_url, &company_id, &entity_type, &entity_id] (pg::cpp::utils::Utility* a_utility) -> void {
                                             // ... perform ...
                                             static_cast<pg::cpp::utils::PublicLink*>(a_utility)->Calculate(base_url, company_id, entity_type, entity_id);
                                         },
                                         /* dealloc */
                                         [] (pg::cpp::utils::Utility* a_utility) -> pg::cpp::utils::Utility* {
                                             delete a_utility;
                                             return nullptr;
                                         }
        );
    }

    /**
     * @brief pg-cpp-utils number to words interface to PostreSQL
     */
    Datum pg_cpp_utils_number_spellout (PG_FUNCTION_ARGS)
    {
        // ... test the number of arguments ...
        const size_t args_count = PG_NARGS();
        if ( args_count < 2 ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_number_spellout(...) - received %zd argument(s), expected at least %d argument(s)!", args_count, 2)
                    )
            );
        }

        // ... collect param(s) ...
        text*  tmp_locale = PG_GETARG_TEXT_P(0);
        float8 tmp_number = PG_GETARG_FLOAT8(1);

        const std::string locale            = tmp_locale ? std::string(VARDATA(tmp_locale) , VARSIZE(tmp_locale)  - VARHDRSZ) : "en_US";
        const double      number            = PG_ARGISNULL(1) ?     0.0 : tmp_number;

        std::string spellout_override;
        if ( args_count >= 2 && 0 == PG_ARGISNULL(2) ) {
            text*  tmp_override = PG_GETARG_TEXT_P(2);
            if ( ( VARSIZE(tmp_override) - VARHDRSZ ) > 0 ) {
                spellout_override = std::string(VARDATA(tmp_override), VARSIZE(tmp_override) - VARHDRSZ);
            }
        }

        // ... perform request ...
        return pg_cpp_utils_utils_common(fcinfo,
                                         /* allocation */
                                         [&locale, &spellout_override] () -> pg::cpp::utils::Utility* {
                                             UErrorCode icu_error_code = UErrorCode::U_ZERO_ERROR;
                                             u_init(&icu_error_code);
                                             if ( UErrorCode::U_ZERO_ERROR != icu_error_code ) {
                                                 throw PG_CPP_UTILS_EXCEPTION("ICU initialization error code %d", icu_error_code);
                                             }
                                             return new pg::cpp::utils::NumberSpellout(locale, spellout_override);
                                         },
                                         /* execute */
                                         [&number] (pg::cpp::utils::Utility* a_utility) -> void {
                                             // ... perform ...
                                             static_cast<pg::cpp::utils::NumberSpellout*>(a_utility)->Spellout(number);
                                         },
                                         /* dealloc */
                                         [] (pg::cpp::utils::Utility* a_utility) -> pg::cpp::utils::Utility* {
                                             delete a_utility;
                                             u_cleanup();
                                             return nullptr;
                                         }
        );

    }

    /**
     * @brief pg-cpp-utils currency to words interface to PostreSQL
     */
    Datum pg_cpp_utils_currency_spellout (PG_FUNCTION_ARGS)
    {
        // ... test the number of arguments ...
        const size_t args_count = PG_NARGS();
        if ( args_count < 8 ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_currency_spellout(...) - received %zd argument(s), expected at least %d argument(s)!", args_count, 8)
                    )
            );
        }

        // ... collect param(s) ...

        text*  tmp_locale         = PG_GETARG_TEXT_P(0);
        float8 tmp_major          = PG_ARGISNULL(1) ? 0.0 : PG_GETARG_FLOAT8(1);
        text*  tmp_major_singular = PG_GETARG_TEXT_P(2);
        text*  tmp_major_plural   = PG_GETARG_TEXT_P(3);
        float8 tmp_minor          = PG_ARGISNULL(4) ? 0.0 : PG_GETARG_FLOAT8(4);
        text*  tmp_minor_singular = PG_GETARG_TEXT_P(5);
        text*  tmp_minor_plural   = PG_GETARG_TEXT_P(6);
        text*  tmp_format         = PG_GETARG_TEXT_P(7);

        const std::string locale            = PG_ARGISNULL(0) ? "pt_PT" : std::string(VARDATA(tmp_locale)        , VARSIZE(tmp_locale)         - VARHDRSZ);
        const std::string major_singular    = PG_ARGISNULL(2) ? ""      : std::string(VARDATA(tmp_major_singular), VARSIZE(tmp_major_singular) - VARHDRSZ);
        const std::string major_plural      = PG_ARGISNULL(3) ? ""      : std::string(VARDATA(tmp_major_plural)  , VARSIZE(tmp_major_plural)   - VARHDRSZ);
        const std::string minor_singular    = PG_ARGISNULL(5) ? ""      : std::string(VARDATA(tmp_minor_singular), VARSIZE(tmp_minor_singular) - VARHDRSZ);
        const std::string minor_plural      = PG_ARGISNULL(6) ? ""      : std::string(VARDATA(tmp_minor_plural)  , VARSIZE(tmp_minor_plural)   - VARHDRSZ);
        const std::string format            = PG_ARGISNULL(7) ? ""      : std::string(VARDATA(tmp_format)        , VARSIZE(tmp_format)         - VARHDRSZ);

        std::string spellout_override;
        if ( args_count >= 9 && 0 == PG_ARGISNULL(8) ) {
            text*  tmp_override = PG_GETARG_TEXT_P(8);
            if ( ( VARSIZE(tmp_override) - VARHDRSZ ) > 0 ) {
                spellout_override = std::string(VARDATA(tmp_override), VARSIZE(tmp_override) - VARHDRSZ);
            }
        }

        const double major = tmp_major;
        const double minor = tmp_minor;

        // ... perform request ...
        return pg_cpp_utils_utils_common(fcinfo,
                                         /* allocation */
                                         [&locale, &spellout_override] () -> pg::cpp::utils::Utility* {
                                             UErrorCode icu_error_code = UErrorCode::U_ZERO_ERROR;
                                             u_init(&icu_error_code);
                                             if ( UErrorCode::U_ZERO_ERROR != icu_error_code ) {
                                                 throw PG_CPP_UTILS_EXCEPTION("ICU initialization error code %d", icu_error_code);
                                             }
                                             return new pg::cpp::utils::NumberSpellout(locale, spellout_override);
                                         },
                                         /* execute */
                                         [&major, &major_singular, &major_plural, &minor, &minor_singular, &minor_plural, &format] (pg::cpp::utils::Utility* a_utility) -> void {
                                             // ... perform ...
                                             static_cast<pg::cpp::utils::NumberSpellout*>(a_utility)->CurrencySpellout(major, major_singular, major_plural,
                                                                                                                       minor, minor_singular, minor_plural,
                                                                                                                       format
                                            );
                                         },
                                         /* dealloc */
                                         [] (pg::cpp::utils::Utility* a_utility) -> pg::cpp::utils::Utility* {
                                             delete a_utility;
                                             u_cleanup();
                                             return nullptr;
                                         }
        );
    }

    /**
     * @brief pg-cpp-utils currency to words interface to PostreSQL
     */
    Datum pg_cpp_utils_format_number (PG_FUNCTION_ARGS)
    {
        // ... test the number of arguments ...
        const size_t args_count = PG_NARGS();
        if ( args_count < 3 ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_number(...) - received %zd argument(s), expected at least %d argument(s)!", args_count, 3)
                     )
            );
        }

        if ( 1 == PG_ARGISNULL(0) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_number(...) - value argument can not be null!")
                     )
            );
        }

        if ( 1 == PG_ARGISNULL(1) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_number(...) - pattern argument can not be null!")
                     )
            );
        }

        if ( 1 == PG_ARGISNULL(2) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_number(...) - locale argument can not be null!")
                     )
            );
        }

        // ... collect param(s) ...

        text*  tmp_locale  = PG_GETARG_TEXT_P(0);
        float8 tmp_value   = PG_GETARG_FLOAT8(1);
        text*  tmp_pattern = PG_GETARG_TEXT_P(2);

        const double      value   = tmp_value;
        const std::string pattern = std::string(VARDATA(tmp_pattern), VARSIZE(tmp_pattern) - VARHDRSZ);
        const std::string locale  = std::string(VARDATA(tmp_locale), VARSIZE(tmp_locale) - VARHDRSZ);

        // ... perform request ...
        return pg_cpp_utils_utils_common(fcinfo,
                                         /* allocation */
                                         [&locale] () -> pg::cpp::utils::Utility* {
                                             UErrorCode icu_error_code = UErrorCode::U_ZERO_ERROR;
                                             u_init(&icu_error_code);
                                             if ( UErrorCode::U_ZERO_ERROR != icu_error_code ) {
                                                 throw PG_CPP_UTILS_EXCEPTION("ICU initialization error code %d", icu_error_code);
                                             }
                                             return new pg::cpp::utils::NumberFormatter(locale);
                                         },
                                         /* execute */
                                         [&value, &pattern] (pg::cpp::utils::Utility* a_utility) -> void {
                                             // ... perform ...
                                             static_cast<pg::cpp::utils::NumberFormatter*>(a_utility)->Format(value, pattern);
                                         },
                                         /* dealloc */
                                         [] (pg::cpp::utils::Utility* a_utility) -> pg::cpp::utils::Utility* {
                                             delete a_utility;
                                             u_cleanup();
                                             return nullptr;
                                         }
        );
    }

    /**
     * @brief pg-cpp-utils format a message
     */
    Datum pg_cpp_utils_format_message (PG_FUNCTION_ARGS)
    {
        // ... test the number of arguments ...
        const size_t args_count = PG_NARGS();
        if ( args_count < 3 ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_message(...) - received %zd argument(s), expected at least %d argument(s)!", args_count, 3)
                    )
            );
        }

        if ( 1 == PG_ARGISNULL(0) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_message(...) - locale argument can not be null!")
                )
            );
        }

        if ( 1 == PG_ARGISNULL(1) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_message(...) - key argument can not be null!")
                )
            );
        }

        if ( 1 == PG_ARGISNULL(2) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_message(...) - args arguments can not be null!")
                )
            );
        }

        // ... collect param(s) ...

        text*      tmp_locale = PG_GETARG_TEXT_P(0);
        text*      tmp_key    = PG_GETARG_TEXT_P(1);
        ArrayType* in_array   = PG_GETARG_ARRAYTYPE_P(2);

        if ( TEXTOID != ARR_ELEMTYPE(in_array) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_message(...) - arguments array must be type cstring[]!")
                    )
            );
        }

        if ( 1 != ARR_NDIM(in_array) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_message(...) - arguments array must be one-dimensional!")
                    )
            );
        }

        if ( array_contains_nulls(in_array) ) {
            ereport(ERROR,
                    (
                     errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("pg_cpp_utils_format_message(...) - arguments array must not contain nulls!")
                    )
            );
        }

        const std::string        locale = std::string(VARDATA(tmp_locale), VARSIZE(tmp_locale) - VARHDRSZ);
        const std::string        format = std::string(VARDATA(tmp_key), VARSIZE(tmp_key) - VARHDRSZ);
        std::vector<std::string> args;

        Datum* in_datums = nullptr;
        int    in_count  = 0;
        /* hardwired knowledge about cstring's representation details here */
        deconstruct_array(in_array, TEXTOID, -1, false, 'i', &in_datums, /* &in_nulls */ nullptr, &in_count);
        for ( int idx = 0; idx < in_count; ++idx ) {
            char* c_str = TextDatumGetCString(in_datums[idx]);
            args.push_back(c_str);
            pfree(c_str);
        }
        pfree(in_datums);

        // ... perform request ...
        return pg_cpp_utils_utils_common(fcinfo,
                                         /* allocation */
                                         [&locale] () -> pg::cpp::utils::Utility* {
                                             UErrorCode icu_error_code = UErrorCode::U_ZERO_ERROR;
                                             u_init(&icu_error_code);
                                             if ( UErrorCode::U_ZERO_ERROR != icu_error_code ) {
                                                 throw PG_CPP_UTILS_EXCEPTION("ICU initialization error code %d", icu_error_code);
                                             }
                                             return new pg::cpp::utils::MessageFormatter(locale);
                                         },
                                         /* execute */
                                         [&format, &args] (pg::cpp::utils::Utility* a_utility) -> void {
                                             // ... perform ...
                                             static_cast<pg::cpp::utils::MessageFormatter*>(a_utility)->Format(format, args);
                                         },
                                         /* dealloc */
                                         [] (pg::cpp::utils::Utility* a_utility) -> pg::cpp::utils::Utility* {
                                             delete a_utility;
                                             u_cleanup();
                                             return nullptr;
                                         }
        );
    }

    /**
     * @brief pg-cpp-utils version output.
     */
    Datum pg_cpp_utils_version (PG_FUNCTION_ARGS)
    {
        // ... perform request ...
        return pg_cpp_utils_utils_common(fcinfo,
                                         /* allocation */
                                         [] () -> pg::cpp::utils::Utility* {
                                             return new pg::cpp::utils::Version();
                                         },
                                         /* execute */
                                         [] (pg::cpp::utils::Utility* a_utility) -> void {
                                             // ... nothing to do ...
                                         },
                                         /* dealloc */
                                         [] (pg::cpp::utils::Utility* a_utility) -> pg::cpp::utils::Utility* {
                                             delete a_utility;
                                             return nullptr;
                                         }
        );

    }

    /**
     * @brief pg-cpp-utils info output.
     */
    Datum pg_cpp_utils_info (PG_FUNCTION_ARGS)
    {
        // ... perform request ...
        return pg_cpp_utils_utils_common(fcinfo,
                                         /* allocation */
                                         [] () -> pg::cpp::utils::Utility* {
                                             return new pg::cpp::utils::Info();
                                         },
                                         /* execute */
                                         [] (pg::cpp::utils::Utility* a_utility) -> void {
                                             // ... nothing to do ...
                                         },
                                         /* dealloc */
                                         [] (pg::cpp::utils::Utility* a_utility) -> pg::cpp::utils::Utility* {
                                             delete a_utility;
                                             return nullptr;
                                         }
        );

    }
}
