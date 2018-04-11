#
# @file Settings.mk
#
# Copyright (c) 2011-2018 Cloudware S.A. All rights reserved.
#
# This file is part of casper-pg-cpp-utils.
#
# casper-pg-cpp-utils is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# casper-pg-cpp-utils is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with casper.  If not, see <http://www.gnu.org/licenses/>.
#

ifdef GCC_OPTIMIZATION_LEVEL
	CFLAGS+= -O$(GCC_OPTIMIZATION_LEVEL)
endif

ifdef GCC_GENERATE_DEBUGGING_SYMBOLS
	CFLAGS+= -g
endif

## warning level in paranoid mode
CFLAGS += -Wall -W -Wextra -Wunused -Wpointer-arith -Wmissing-declarations -Wmissing-noreturn -Winline
CFLAGS += -Wno-unused-parameter
ifeq ($(shell uname -s),Darwin)
	CFLAGS += -Wbad-function-cast -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wcast-qual -Wredundant-decls -Wno-deprecated-register
endif

ifeq ($(GCC_WARN_SHADOW),YES)
	CFLAGS+= -Wshadow
endif
