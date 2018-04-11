/**
 * @file utility.cc
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

#include "pg/cpp/utils/utility.h"

/**
 * @brief Allocate user context information.
 *
 * @param a_context
 */
void pg::cpp::utils::Utility::AllocUserFuncContext (FuncCallContext* a_context)
{
    a_context->user_fctx = new pg::cpp::utils::Utility::Records();
    a_context->max_calls = 0;
}

/**
 * @brief Release user provided context information.
 *
 * @param a_context
 */
void pg::cpp::utils::Utility::DeallocUserFuncContext (FuncCallContext* a_context)
{
    if ( nullptr != a_context->user_fctx ) {
        pg::cpp::utils::Utility::Records* records = static_cast<pg::cpp::utils::Utility::Records*>(a_context->user_fctx);
        delete records;
        a_context->user_fctx = nullptr;
        a_context->max_calls = 0;
    }
}
