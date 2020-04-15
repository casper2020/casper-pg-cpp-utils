#
# @file Makefile
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

.PHONY: debug

include Settings.mk

PLATFORM    := $(shell uname -s)
PLATFORM_LC := $(shell echo $(PLATFORM) | tr A-Z a-z)

####################
# Set target type
####################
ifeq (Darwin, $(PLATFORM))
 ifndef TARGET
   TARGET:=Debug
 else
   override TARGET:=$(shell echo $(TARGET) | tr A-Z a-z)
   $(eval TARGET_E:=$$$(TARGET))
   TARGET_E:=$(shell echo $(TARGET_E) | tr A-Z a-z )
   TARGET_S:=$(subst $(TARGET_E),,$(TARGET))
   TARGET_S:=$(shell echo $(TARGET_S) | tr a-z A-Z )
   TMP_TARGET:=$(TARGET_S)$(TARGET_E)
   override TARGET:=$(TMP_TARGET)
 endif
else
 ifndef TARGET
   TARGET:=debug
  else
   override TARGET:=$(shell echo $(TARGET) | tr A-Z a-z)
  endif
endif
TARGET_LC:=$(shell echo $(TARGET) | tr A-Z a-z)

# validate target
ifeq (release, $(TARGET_LC))
  #
else
  ifeq (debug, $(TARGET_LC))
    #
  else
    $(error "Don't know how to build for target $(TARGET_LC) ")
  endif
endif

#####################
# SET EXTERNAL TOOLS
#####################

RAGEL    := $(shell which ragel)
ifeq (Darwin, $(PLATFORM))
  YACC := /usr/local/Cellar/bison/3.5/bin/bison
  READLINK_CMD := greadlink
else
  YACC := $(shell which bison)
  READLINK_CMD := readlink
endif
PG_CONFIG                      ?= $(shell which pg_config)
POSTGRESQL_HEADERS_DIR 	       := $(shell $(PG_CONFIG) --includedir-server)
POSTGRESQL_HEADERS_OTHER_C_DIR := $(POSTGRESQL_HEADERS_DIR:server=)

USE_CUSTOM_COMPILED_LIBS?=true

CC_PACKAGER_DIR?=$(shell $(READLINK_CMD) -m ../casper-packager)

# ICU
CC_ICU_DIR?=$(CC_PACKAGER_DIR)/icu/$(PLATFORM_LC)/pkg/$(TARGET)/icu/usr/local/casper/icu
CC_ICU_VERSION?=$(shell cat  $(CC_PACKAGER_DIR)/icu/version | cut -d'-' -f1)
CC_ICU_INCLUDE_DIR?=$(CC_ICU_DIR)/include
CC_ICU_LIB_DIR?=$(CC_ICU_DIR)/lib

CC_ICU_LIBS_FN=
ifeq (true, $(USE_CUSTOM_COMPILED_LIBS))
  ifeq (Darwin, $(PLATFORM))
    CC_ICU_LIBS_FN=libicuuc.$(CC_ICU_VERSION).dylib libicui18n.$(CC_ICU_VERSION).dylib libicudata.$(CC_ICU_VERSION).dylib libicuio.$(CC_ICU_VERSION).dylib libicutu.$(CC_ICU_VERSION).dylib
  endif
endif

# OPENSSL
CC_OPENSSL_DIR?=$(CC_PACKAGER_DIR)/openssl/$(PLATFORM_LC)/pkg/$(TARGET)/openssl/usr/local/casper/openssl
CC_OPENSSL_VERSION?=$(shell cat  $(CC_PACKAGER_DIR)/openssl/version | tr -dc '0-9.' | cut -d'.' -f1-2)

CC_OPENSS_INCLUDE_DIR?=$(CC_OPENSSL_DIR)/include
CC_OPENSS_LIB_DIR?=$(CC_OPENSSL_DIR)/lib

CC_OPENSSL_LIBS_FN=
ifeq (true, $(USE_CUSTOM_COMPILED_LIBS))
  ifeq (Darwin, $(PLATFORM))
    CC_OPENSSL_LIBS_FN=libcrypto.$(CC_OPENSSL_VERSION).dylib libssl.$(CC_OPENSSL_VERSION).dylib
  endif
endif

#
ifeq (true, $(USE_CUSTOM_COMPILED_LIBS))
  ICU_INCLUDE_DIR?=$(CC_ICU_INCLUDE_DIR)
  ICU_LIB_DIR?=$(CC_ICU_LIB_DIR)
  OPENSSL_INCLUDE_DIR?=$(CC_OPENSS_INCLUDE_DIR)
  OPENSSL_LIB_DIR?=$(CC_OPENSS_LIB_DIR)
endif

ifeq (Darwin, $(PLATFORM))
  ICU_INCLUDE_DIR?=/usr/local/opt/icu4c/include
  ICU_LIB_DIR?=/usr/local/opt/icu4c/lib
  OPENSSL_INCLUDE_DIR?=/usr/local/opt/openssl/include
  OPENSSL_LIB_DIR?=/usr/local/opt/openssl/lib
else
  ICU_INCLUDE_DIR?=/usr/include/unicode
  ICU_LIB_DIR?=usr/lib/x86_64-linux-gnu
  OPENSSL_INCLUDE_DIR?=/usr/include
  OPENSSL_LIB_DIR?=/usr/lib/x86_64-linux-gnu/
endif

#####################
# SOURCE & INCLUDES
#####################

SRC := src/pg-cpp-utils.cc                    \
	src/pg/cpp/utils/version.cc           \
	src/pg/cpp/utils/utility.cc           \
	src/pg/cpp/utils/b64.cc               \
	src/pg/cpp/utils/invoice_hash.cc      \
	src/pg/cpp/utils/public_link.cc       \
	src/pg/cpp/utils/number_spellout.cc   \
	src/pg/cpp/utils/number_formatter.cc  \
	src/pg/cpp/utils/message_formatter.cc

JSONCPP_SRC := src/jsoncpp/jsoncpp.cpp
OSAL_SRC    := ../casper-osal/src/osal/posix/posix_time.cc

OBJS := $(SRC:.cc=.o) $(JSONCPP_SRC:.cpp=.o) $(OSAL_SRC:.cc=.o)

### HEADERS SEARCH PATHS ###
FPG_HEADERS_SEARCH_PATH := \
	-I $(POSTGRESQL_HEADERS_DIR) 	     \
	-I $(POSTGRESQL_HEADERS_OTHER_C_DIR) \
	-I src

FPG_HEADERS_SEARCH_PATH += -I $(ICU_INCLUDE_DIR)
FPG_HEADERS_SEARCH_PATH += -I ../casper-osal/src -I ../cppcodec
FPG_HEADERS_SEARCH_PATH += -I $(OPENSSL_INCLUDE_DIR)

PG_CPPFLAGS := $(FPG_HEADERS_SEARCH_PATH)

######################
# Set compiler flags
######################
CXX      = g++
CXXFLAGS = -std=c++11 $(FPG_HEADERS_SEARCH_PATH) -c -Wall -fPIC
PG_CXXFLAGS = -std=c++11 $(FPG_HEADERS_SEARCH_PATH) -c -Wall -fPIC
ifeq ($(TARGET_LC),release)
  CXXFLAGS += -g -O2 -DNDEBUG
  PG_CXXFLAGS += -g -O2 -DNDEBUG
else
  CXXFLAGS += -g -O0 -DDEBUG
  PG_CXXFLAGS += -g -O0 -DDEBUG
endif
ifeq (Darwin, $(PLATFORM))
  CLANG_CXX_LANGUAGE_STANDARD = c++11
  CLANG_CXX_LIBRARY = libc++
endif

############################
# Set lib name and version
############################
LIB_NAME := pg-cpp-utils
ifndef LIB_VERSION
  LIB_VERSION := $(shell cat $(CC_PACKAGER_DIR)/pg-cpp-utils/version)
endif
LINKER_FLAGS =
ifeq (Darwin, $(PLATFORM))
  SO_NAME := $(LIB_NAME).dylib.$(LIB_VERSION)
else
  SO_NAME := $(LIB_NAME).so.$(LIB_VERSION)
  LINKER_FLAGS += -Wl,-soname,$(SO_NAME) -Wl,-z,relro -Bsymbolic
endif

LINKER_FLAGS += -lstdc++
LINKER_FLAGS += -Wl,-rpath,$(ICU_LIB_DIR),-rpath,$(OPENSSL_LIB_DIR)
LINKER_FLAGS += -L$(OPENSSL_LIB_DIR) -lcrypto -lssl
LINKER_FLAGS += -L$(ICU_LIB_DIR) -licuuc -licui18n -licudata -licuio -licutu

$(shell sed -e s#@VERSION@#${LIB_VERSION}#g pg-cpp-utils.control.tpl > pg-cpp-utils.control)
$(shell sed -e s#x\.x\.xx#${LIB_VERSION}#g src/pg/cpp/utils/versioning.h.tpl > src/pg/cpp/utils/versioning.h)

################
# Set PG flags
################
EXTENSION   := $(LIB_NAME)
EXTVERSION  := $(LIB_VERSION)
PG_LDFLAGS  := -lstdc++ 
SHLIB_LINK  := $(LINKER_FLAGS)
PG_LIBS     := 
MODULE_big  := $(LIB_NAME)
EXTRA_CLEAN := $(LIB_NAME).so* $(LIB_NAME).dylib*
PGXS        := $(shell $(PG_CONFIG) --pgxs)

include $(PGXS)

override LDFLAGS :=
override LIBS:=$(LIBS:-lcrypto=)
override LIBS:=$(LIBS:-lssl=)

##############################
# Set rules for .so / .dylib
##############################

# shared object
shared_object: $(OBJS)
	@echo "* $(LIB_NAME) ~> done"
	otool -L $(LIB_NAME)

# PostgreSQL bit code
%.bc:%.cc
	@echo "* bc  [$(TARGET)] $< ..."
	@$(COMPILE.cxx.bc) $(FPG_HEADERS_SEARCH_PATH) $(CCFLAGS) $(CPPFLAGS) -fPIC -c -o $@ $<

# c++
.cc.o:
	@echo "* cc  [$(TARGET)] $< ..."
	@$(CXX) $(CXXFLAGS) $< -o $@

# c++
.cpp.o:
	@echo "* cpp [$(TARGET)] $< ..."
	@$(CXX) $(CXXFLAGS) $< -o $@

# clean
clean_all: clean_bison clean_ragel
	@echo "* clean..."
	@find .. -name "*.o" -delete
	@find .. -name "*~" -delete
	@find . -name "$(LIB_NAME).so*" -delete
	@find . -name "$(LIB_NAME).dylib*" -delete

# so
so:
	@make -f Makefile clean clean_all
ifeq (Darwin, $(PLATFORM))
	@echo "* macOS $(TARGET) build..."
	@xcodebuild -target pg-cpp-utils -configuration $(TARGET) clean
	@xcodebuild -target pg-cpp-utils -configuration $(TARGET)
	@make install
else
	@echo "* linux $(TARGET) v$(EXTVERSION) build..."
	@make clean
	@make
endif

# release
release:
	@make -f Makefile so TARGET=release

# debug
debug:
	@echo "* macOS $(TARGET) build..."
ifeq (Darwin, $(PLATFORM))
	@xcodebuild -target pg-cpp-utils -configuration $(TARGET)
	@make install
else
	@make
endif

#
rpath:
	@echo "* macOS [$(TARGET)] fix rpath..."
ifeq (true, $(USE_CUSTOM_COMPILED_LIBS))
	@echo "* CC_ICU_LIB_DIR=$(CC_ICU_LIB_DIR): $(CC_ICU_LIBS_FN)"
	@echo "* CC_OPENSS_LIB_DIR=$(CC_OPENSS_LIB_DIR): $(CC_OPENSSL_LIBS_FN)"
ifeq (Darwin, $(PLATFORM))
	@otool -L $(LIB_NAME).so
	@$(foreach lib,$(CC_ICU_LIBS_FN), install_name_tool -change $(lib) $(CC_ICU_LIB_DIR)/$(lib) $(LIB_NAME).so ;)
	@$(foreach lib,$(CC_OPENSSL_LIBS_FN), install_name_tool -change $(lib) $(CC_OPENSS_LIB_DIR)/$(lib) $(LIB_NAME).so ;)
	@otool -L $(LIB_NAME).so
else
	@ldd $(LIB_NAME).so
	@patchelf --set-rpath '/usr/local/casper/icu/lib/:/usr/local/casper/openssl/lib' $(LIB_NAME).so
	@ldd $(LIB_NAME).so
endif
endif

# info
info:
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "LIB_VERSION=$(LIB_VERSION)"	
	@echo "_ICU_LIBS=$(_ICU_LIBS)"
	@echo "ICU_INCLUDE_DIR=$(ICU_INCLUDE_DIR)"
	@echo "ICU_LIB_DIR=$(ICU_LIB_DIR)"
	@echo "OPENSSL_INCLUDE_DIR=$(OPENSSL_INCLUDE_DIR)"
	@echo "OPENSSL_LIB_DIR=$(OPENSSL_LIB_DIR)"
	@echo "LINKER_FLAGS=$(LINKER_FLAGS)"
	@echo "LIBS=$(LIBS)"

# symbols dump
dump_dyn_symb_table:
ifeq (Darwin, $(PLATFORM))
	@echo "No Info"
else
	@readelf -d pg-cpp-utils.so
endif
