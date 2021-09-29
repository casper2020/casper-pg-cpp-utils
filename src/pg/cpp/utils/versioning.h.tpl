/**
 * @file versioning.h.tpl
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
#ifndef PG_CPP_UTILS_VERSIONING_H_
#define PG_CPP_UTILS_VERSIONING_H_

#ifndef PG_CPP_UTILS_NAME
#define PG_CPP_UTILS_NAME "pg-cpp-utils@b.n.s@"
#endif

#ifndef PG_CPP_UTILS_TARGET
#define PG_CPP_UTILS_TARGET "t.t.t"
#endif

#ifndef PG_CPP_UTILS_VERSION
#define PG_CPP_UTILS_VERSION "x.x.x"
#endif

#ifndef PG_CPP_UTILS_REL_DATE
#define PG_CPP_UTILS_REL_DATE "r.r.d"
#endif

#ifndef PG_CPP_UTILS_REL_BRANCH
#define PG_CPP_UTILS_REL_BRANCH "r.r.b"
#endif

#ifndef PG_CPP_UTILS_REL_HASH
#define PG_CPP_UTILS_REL_HASH "r.r.h"
#endif

#ifndef PG_CPP_UTILS_INFO
#define PG_CPP_UTILS_INFO PG_CPP_UTILS_NAME " v" PG_CPP_UTILS_VERSION
#endif

#endif // PG_CPP_UTILS_VERSIONING_H_
