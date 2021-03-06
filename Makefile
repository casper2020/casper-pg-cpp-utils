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

#####################
# SET EXTERNA TOOLS
#####################

PLATFORM := $(shell uname -s)
RAGEL    := $(shell which ragel)
ifeq (Darwin, $(PLATFORM))
	YACC := /usr/local/Cellar/bison/3.0.4/bin/bison
else
	YACC := $(shell which bison)
endif
PG_CONFIG                      ?= $(shell which pg_config)
POSTGRESQL_HEADERS_DIR 	       := $(shell $(PG_CONFIG) --includedir-server)
POSTGRESQL_HEADERS_OTHER_C_DIR := $(POSTGRESQL_HEADERS_DIR:server=)

#####################
# SOURCE & INCLUDES
#####################

SRC := src/pg-cpp-utils.cc                   \
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

FPG_HEADERS_SEARCH_PATH := \
	-I $(POSTGRESQL_HEADERS_DIR) 	     \
	-I $(POSTGRESQL_HEADERS_OTHER_C_DIR) \
	-I src

ifeq (Darwin, $(PLATFORM))
  FPG_HEADERS_SEARCH_PATH += -I /usr/local/opt/openssl/include -I /usr/local/opt/icu4c/include
else
  FPG_HEADERS_SEARCH_PATH += -I /usr/include/openssl
  FPG_HEADERS_SEARCH_PATH += -I ../libicu-dev_52.1-8+deb8u7_amd64/usr/include/unicode
endif
FPG_HEADERS_SEARCH_PATH += -I ../casper-osal/src -I ../cppcodec

PG_CPPFLAGS := $(FPG_HEADERS_SEARCH_PATH)
PG_CXXFLAGS := $(FPG_HEADERS_SEARCH_PATH)

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
LIB_NAME    := pg-cpp-utils
ifndef LIB_VERSION
	LIB_VERSION := "0.0.00"
endif
LINKER_FLAGS =
ifeq (Darwin, $(PLATFORM))
  SO_NAME := $(LIB_NAME).dylib.$(LIB_VERSION)
  LINKER_FLAGS += -L/usr/local/opt/openssl/lib
  LINKER_FLAGS += /usr/local/opt/openssl/lib/libcrypto.a /usr/local/opt/openssl/lib/libssl.a /usr/local/opt/icu4c/lib/libicudata.a /usr/local/opt/icu4c/lib/libicuio.a /usr/local/opt/icu4c/lib/libicutu.a /usr/local/opt/icu4c/lib/libicuuc.a /usr/local/opt/icu4c/lib/libicui18n.a
else
  SO_NAME := $(LIB_NAME).so.$(LIB_VERSION)
  LINKER_FLAGS += -Wl,-soname,$(SO_NAME) -Wl,-z,relro -Bsymbolic
  LINKER_FLAGS += -lcrypto -lssl
  LINKER_FLAGS += ../libicu-dev_52.1-8+deb8u7_amd64/usr/lib/x86_64-linux-gnu/libicudata.a
  LINKER_FLAGS += ../libicu-dev_52.1-8+deb8u7_amd64/usr/lib/x86_64-linux-gnu/libicuio.a
  LINKER_FLAGS += ../libicu-dev_52.1-8+deb8u7_amd64/usr/lib/x86_64-linux-gnu/libicutu.a
  LINKER_FLAGS += ../libicu-dev_52.1-8+deb8u7_amd64/usr/lib/x86_64-linux-gnu/libicuuc.a
  LINKER_FLAGS += ../libicu-dev_52.1-8+deb8u7_amd64/usr/lib/x86_64-linux-gnu/libicui18n.a
endif
$(shell sed -e s#@VERSION@#${LIB_VERSION}#g pg-cpp-utils.control.tpl > pg-cpp-utils.control)
$(shell sed -e s#x\.x\.xx#${LIB_VERSION}#g src/pg/cpp/utils/versioning.h.tpl > src/pg/cpp/utils/versioning.h)

################
# Set PG flags
################
EXTENSION   := $(LIB_NAME)
EXTVERSION  := $(LIB_VERSION)
SHLIB_LINK  := -lstdc++ $(LINKER_FLAGS)
MODULE_big  := $(LIB_NAME)
EXTRA_CLEAN :=
PGXS        := $(shell $(PG_CONFIG) --pgxs)

include $(PGXS)

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
	@echo "$(FPG_HEADERS_SEARCH_PATH)"
	@echo "* cc  [$(TARGET)] $< ..."
	$(CXX) $(CXXFLAGS) $< -o $@

# c++
.cpp.o:
	@echo "$(FPG_HEADERS_SEARCH_PATH)"
	@echo "* cpp [$(TARGET)] $< ..."
	@$(CXX) $(CXXFLAGS) $< -o $@

# clean
clean_all: clean_bison clean_ragel
	@echo "* clean..."
	@find .. -name "*.o" -delete
	@find .. -name "*~" -delete
	@find . -name "$(LIB_NAME).so*" -delete
	@find . -name "$(LIB_NAME).dylib*" -delete
	@rm -f $(LIB_NAME)

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

# symbols dump
dump_dyn_symb_table:
ifeq (Darwin, $(PLATFORM))
	@echo "No Info"
else
	@readelf -d pg-cpp-utils.so
endif
