/**
 * @file exception.h
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

#pragma once
#ifndef PG_CPP_UTILS_EXCEPTION_H_
#define PG_CPP_UTILS_EXCEPTION_H_

#include <stdexcept>
#include <exception>
#include <vector>
#include <string>
#include <cstdarg>
#include <sstream>

#ifdef __APPLE__
    #define PG_CPP_UTILS_EXCEPTION_TRACE_CPP_GENERIC_EXCEPTION() \
        [&] () -> std::string { \
            std::stringstream ss; \
            std::exception_ptr p = std::current_exception(); \
            ss << "C++ Generic Exception @" << __PRETTY_FUNCTION__ << ":" << __LINE__; \
            try { \
                std::rethrow_exception(p);\
            } catch(const std::exception& e) { \
                ss << "what() =" << e.what(); \
            } \
            return ss.str(); \
        }()
#else
    #define PG_CPP_UTILS_EXCEPTION_TRACE_CPP_GENERIC_EXCEPTION() \
        [&] () -> std::string { \
            std::stringstream ss; \
            std::exception_ptr p = std::current_exception(); \
            ss << "C++ Generic Exception @" << __PRETTY_FUNCTION__ << ":" << __LINE__; \
            if ( p ) { \
                ss << "name() =" << p.__cxa_exception_type()->name(); \
                ss << "what() =" << p.__cxa_exception_type()->name(); \
            } \
            return ss.str(); \
        }()
#endif

#define PG_CPP_UTILS_EXCEPTION(a_format, ...) \
    pg::cpp::utils::Exception(__FUNCTION__, __LINE__, a_format, __VA_ARGS__);

#define PG_CPP_UTILS_EXCEPTION_NA(a_what) \
    pg::cpp::utils::Exception(__FUNCTION__, __LINE__, a_what);

namespace pg
{

    namespace cpp
    {

        namespace utils
        {

            class Exception final : public std::exception
            {

            public: // Data

                const std::string function_;
                const int         line_;

            private: // Data

                std::string what_;

            public: // Constructor(s) / Destructor

                /**
                 * @brief A constructor that provides the reason of the fault origin.
                 *
                 * @param a_function
                 * @param a_line
                 * @param a_format printf like format followed by a variable number of arguments.
                 * @param ...
                 */
                Exception (const char* const a_function, int a_line, const char* const a_format, ...) __attribute__((format(printf, 4, 5)))
                : function_(a_function), line_(a_line)
                {

                    auto temp   = std::vector<char> {};
                    auto length = std::size_t { 512 };
                    std::va_list args;
                    while ( temp.size() <= length ) {
                        temp.resize(length + 1);
                        va_start(args, a_format);
                        const auto status = std::vsnprintf(temp.data(), temp.size(), a_format, args);
                        va_end(args);
                        if ( status < 0 ) {
                            throw std::runtime_error {"string formatting error"};
                        }
                        length = static_cast<std::size_t>(status);
                    }
                    what_ = std::string { temp.data(), length };
                }

                /**
                 * @brief std::string Constructor.
                 *
                 * @param a_function
                 * @param a_line
                 * @param a_what
                 */
                Exception (const char* const a_function, int a_line, const std::string& a_what)
                : function_(a_function), line_(a_line)
                {
                    what_     = a_what;
                }

                /**
                 * @brief Destructor.
                 */
                virtual ~Exception() throw ()
                {
                    /* empty */
                }

            public:

                virtual const char* what() const throw()
                {
                    return what_.c_str();
                }

            }; // end of class 'Exception'

        } // end of namespace 'utils'

    } // end of namespace 'cpp'

} // end of namespace 'cce'

#endif // PG_CPP_UTILS_EXCEPTION_H_
