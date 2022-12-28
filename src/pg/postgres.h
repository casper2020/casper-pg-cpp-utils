/**
 * @file postgresql.h
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

#include "cc/pragmas.h"

CC_DIAGNOSTIC_PUSH()

CC_DIAGNOSTIC_IGNORED("-Wdeprecated-register")
CC_DIAGNOSTIC_IGNORED("-Wshorten-64-to-32")
CC_DIAGNOSTIC_IGNORED("-Wsign-conversion")
CC_DIAGNOSTIC_IGNORED("-Wunused-parameter")
extern "C" {
    #include "server/postgres.h"
    #include "catalog/pg_type.h"
    #include <utils/array.h>
    #include "executor/spi.h"
    #include "lib/stringinfo.h"
    #include <utils/jsonb.h>
    #include <funcapi.h>
}
CC_DIAGNOSTIC_POP()
