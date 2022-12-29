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
MACHINE_ARCH := $(shell uname -m)
PRJ_ARCH ?= $(MACHINE_ARCH)
MULTI_ARCH_BUILD_MACHINE ?= false

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

PACKAGER_DIR?=$(shell $(READLINK_CMD) -m ../casper-packager)
INSTALL_DIR:=/usr/local/casper

# ICU
ICU_FULL_VERSION?=$(shell cat  $(PACKAGER_DIR)/icu/version)
ifndef ICU_INSTALL_DIR
  ifeq (Darwin, $(PLATFORM))
    ICU_INSTALL_DIR:=$(INSTALL_DIR)/icu/$(ICU_FULL_VERSION)/$(TARGET)
    ifeq (true, $(MULTI_ARCH_BUILD_MACHINE))
       ICU_INSTALL_DIR:=$(INSTALL_DIR)/icu/$(PRJ_ARCH)/$(TARGET)/$(ICU_FULL_VERSION)
    endif
  else
      ICU_INSTALL_DIR:=$(INSTALL_DIR)/icu/$(ICU_FULL_VERSION)
  endif
endif
ICU_INCLUDE_DIR?=$(shell $(READLINK_CMD) -m $(ICU_INSTALL_DIR)/include)
ICU_LIB_DIR?=$(shell $(READLINK_CMD) -m $(ICU_INSTALL_DIR)/lib)
ICU_VERSION?=$(shell cat  $(PACKAGER_DIR)/icu/version | cut -d'-' -f1)
ICU_LIBS_FN=libicuuc.$(ICU_VERSION).dylib libicui18n.$(ICU_VERSION).dylib libicudata.$(ICU_VERSION).dylib libicuio.$(ICU_VERSION).dylib libicutu.$(ICU_VERSION).dylib

# OPENSSL
OPENSSL_FULL_VERSION?=$(shell cat  $(PACKAGER_DIR)/openssl/version)
OPENSSL_VERSION?=$(shell cat $(PACKAGER_DIR)/openssl/version | tr -dc '0-9.' | cut -d'.' -f1-2)

ifndef OPENSSL_INSTALL_DIR
  ifeq (Darwin, $(PLATFORM))
    OPENSSL_INSTALL_DIR:=$(INSTALL_DIR)/openssl/$(OPENSSL_FULL_VERSION)/$(TARGET)
    ifeq (true, $(MULTI_ARCH_BUILD_MACHINE))
       OPENSSL_INSTALL_DIR:=$(INSTALL_DIR)/openssl/$(PRJ_ARCH)/$(TARGET)/$(OPENSSL_FULL_VERSION)
    endif
  else
    OPENSSL_INSTALL_DIR:=$(INSTALL_DIR)/openssl/$(OPENSSL_FULL_VERSION)
  endif
endif
OPENSSL_INCLUDE_DIR?=$(shell $(READLINK_CMD) -m $(OPENSSL_INSTALL_DIR)/include)
OPENSSL_LIB_DIR?=$(shell $(READLINK_CMD) -m $(OPENSSL_INSTALL_DIR)/lib)
OPENSSL_LIBS_FN=libcrypto.$(OPENSSL_VERSION).dylib libssl.$(OPENSSL_VERSION).dylib

#####################
# SOURCE & INCLUDES
#####################

SRC := src/pg-cpp-utils.cc                \
	src/pg/cpp/utils/version.cc           \
	src/pg/cpp/utils/info.cc              \
	src/pg/cpp/utils/locales.cc           \
	src/pg/cpp/utils/utility.cc           \
	src/pg/cpp/utils/b64.cc               \
	src/pg/cpp/utils/jwt.cc               \
	src/pg/cpp/utils/invoice_hash.cc      \
	src/pg/cpp/utils/public_link.cc       \
	src/pg/cpp/utils/number_spellout.cc   \
	src/pg/cpp/utils/number_formatter.cc  \
	src/pg/cpp/utils/message_formatter.cc

############################
# DEPENDENCIES
############################

OSAL_DIR          := $(shell $(READLINK_CMD) -m ../casper-osal)
OSAL_SRC_DIR      := $(OSAL_DIR)/src
OSAL_LIB_DIR      ?= $(PACKAGER_DIR)/build/$(PLATFORM_LC)/pg-cpp-utils//$(TARGET)/casper-osal
OSAL_LIB          ?= osal-icu
OSAL_LINKER_FLAGS := -L $(OSAL_LIB_DIR) -l$(OSAL_LIB)

CONNECTORS_DIR          := $(shell $(READLINK_CMD) -m ../casper-connectors)
CONNECTORS_SRC_DIR      := $(CONNECTORS_DIR)/src
CONNECTORS_LIB_DIR      ?= $(PACKAGER_DIR)/build/$(PLATFORM_LC)/pg-cpp-utils//$(TARGET)/casper-connectors
CONNECTORS_LIB          ?= connectors-icu
CONNECTORS_LINKER_FLAGS := -L $(CONNECTORS_LIB_DIR) -l$(CONNECTORS_LIB)

# jsoncpp
JSONCPP_DIR          := $(PACKAGER_DIR)/jsoncpp
JSONCPP_SRC_DIR      := $(shell $(READLINK_CMD) -m ../jsoncpp/dist )
JSONCPP_LIB_DIR      ?= $(PACKAGER_DIR)/build/$(PLATFORM_LC)/pg-cpp-utils//$(TARGET)/jsoncpp
JSONCPP_LIB          ?= jsoncpp
JSONCPP_LINKER_FLAGS := -L $(JSONCPP_LIB_DIR) -l$(JSONCPP_LIB)

# cppcodec
CPPCODEC_SRC_DIR      := $(shell $(READLINK_CMD) -m ../cppcodec )

############################
# OTHER
############################

OBJS := $(SRC:.cc=.o)

### HEADERS SEARCH PATHS ###
FPG_HEADERS_SEARCH_PATH := \
	-I $(POSTGRESQL_HEADERS_DIR) 	      \
	-I $(POSTGRESQL_HEADERS_OTHER_C_DIR)  \
	-I $(shell $(READLINK_CMD) -m ./src )

FPG_HEADERS_SEARCH_PATH += -I $(ICU_INCLUDE_DIR)
FPG_HEADERS_SEARCH_PATH += -I $(OPENSSL_INCLUDE_DIR)
FPG_HEADERS_SEARCH_PATH += -I $(CONNECTORS_SRC_DIR) 
FPG_HEADERS_SEARCH_PATH += -I $(OSAL_SRC_DIR)
FPG_HEADERS_SEARCH_PATH += -I $(CPPCODEC_SRC_DIR)
FPG_HEADERS_SEARCH_PATH += -I $(JSONCPP_SRC_DIR)

PG_CPPFLAGS := $(FPG_HEADERS_SEARCH_PATH)

######################
# Set compiler flags
######################
CXX = clang++
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
  LIB_VERSION := $(shell cat $(PACKAGER_DIR)/pg-cpp-utils/version)
endif
LINKER_FLAGS =
ifeq (Darwin, $(PLATFORM))
  LINKER_SET_STATIC_LIB_PREFERENCE:=-Wl, -Bstatic
  LINKER_SET_SHARED_LIB_PREFERENCE:=-Wl, -Bdynamic
  SO_NAME := $(LIB_NAME).dylib.$(LIB_VERSION)
else
  LINKER_SET_STATIC_LIB_PREFERENCE:=-Wl,-Bstatic
  LINKER_SET_SHARED_LIB_PREFERENCE:=-Wl,-Bdynamic
  SO_NAME := $(LIB_NAME).so.$(LIB_VERSION)
  LINKER_FLAGS += -Wl,-soname,$(SO_NAME) -Wl,-z,relro -Bsymbolic  
endif

LINKER_FLAGS += -lstdc++
LINKER_FLAGS += -L$(OPENSSL_LIB_DIR) -lcrypto -lssl
LINKER_FLAGS += -L$(ICU_LIB_DIR) -licuuc -licui18n -licudata -licuio -licutu
LINKER_FLAGS += $(CONNECTORS_LINKER_FLAGS)
LINKER_FLAGS += $(JSONCPP_LINKER_FLAGS)
LINKER_FLAGS += $(OSAL_LINKER_FLAGS)

############################
# EXPAND version.h
############################
REL_NAME   := $(LIB_NAME)
REL_DATE   := $(shell date -u)
REL_HASH   := $(shell git rev-parse HEAD)
REL_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

$(shell sed -e s#@VERSION@#${LIB_VERSION}#g pg-cpp-utils.control.tpl > pg-cpp-utils.control)
$(shell cp -f src/pg/cpp/utils/versioning.h.tpl src/pg/cpp/utils/versioning.h )
$(shell sed -i.bak s#"n.n.n"#$(REL_NAME)#g src/pg/cpp/utils/versioning.h )
$(shell sed -i.bak s#"x.x.x"#$(LIB_VERSION)#g src/pg/cpp/utils/versioning.h)
$(shell sed -i.bak s#"r.r.d"#"$(REL_DATE)"#g src/pg/cpp/utils/versioning.h )
$(shell sed -i.bak s#"r.r.b"#"$(REL_BRANCH)"#g src/pg/cpp/utils/versioning.h )
$(shell sed -i.bak s#"r.r.h"#"$(REL_HASH)"#g src/pg/cpp/utils/versioning.h )
$(shell sed -i.bak s#"v.v.v"#"$(REL_VARIANT)"#g src/pg/cpp/utils/versioning.h )
$(shell sed -i.bak s#"r.r.t"#"$(TARGET)"#g src/pg/cpp/utils/versioning.h )
$(shell rm -f src/pg/cpp/utils/versioning.h.bak)

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

override LDFLAGS := -Wl,-headerpad_max_install_names
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
	$(CXX) $(CXXFLAGS) $< -o $@

# c++
.cpp.o:
	@echo "* cpp [$(TARGET)] $< ..."
	@$(CXX) $(CXXFLAGS) $< -o $@

# clean
clean_all:
	@echo "* clean..."
	@find $(OSAL_SRC_DIR) -name "*.o" -or -name "*~" -delete
	@find $(CONNECTORS_SRC_DIR)  -name "*.o" -or -name "*~" -delete
	@find . -name "$(LIB_NAME).so*" -delete
	@find . -name "$(LIB_NAME).dylib*" -delete

common-deps:
	@$(eval OSAL_DEPS=ICU_STAND_ALONE_DEP_ON=true)
	@$(eval CONNECTORS_DEPS=JSONCPP_DEP_ON=true CASPER_OSAL_ICU_DEP_ON=true ICU_STAND_ALONE_DEP_ON=true CPPCODEC_DEP_ON=true ZLIB_DEP_ON=true)

# dependencies
deps-debug: common-deps
	@make -C $(PACKAGER_DIR)/casper-osal -f static-lib-makefile.mk PROJECT_ORIGIN=$(LIB_NAME) PRJ_ARCH=$(PRJ_ARCH) TARGET=debug $(OSAL_DEPS) clean lib
	make -C $(PACKAGER_DIR)/casper-connectors -f static-lib-makefile.mk PROJECT_ORIGIN=$(LIB_NAME) PRJ_ARCH=$(PRJ_ARCH) TARGET=debug $(CONNECTORS_DEPS) clean lib
	@make -C $(JSONCPP_DIR) -f static-lib-makefile.mk PROJECT_ORIGIN=$(LIB_NAME) PRJ_ARCH=$(PRJ_ARCH) TARGET=debug clean lib

deps-release: common-deps
	@make -C $(PACKAGER_DIR)/casper-osal -f static-lib-makefile.mk PROJECT_ORIGIN=$(LIB_NAME) PRJ_ARCH=$(PRJ_ARCH) TARGET=release $(OSAL_DEPS) clean lib
	@make -C $(PACKAGER_DIR)/casper-connectors -f static-lib-makefile.mk PROJECT_ORIGIN=$(LIB_NAME) PRJ_ARCH=$(PRJ_ARCH) TARGET=release $(CONNECTORS_DEPS) clean lib
	@make -C $(JSONCPP_DIR) -f static-lib-makefile.mk PROJECT_ORIGIN=$(LIB_NAME) PRJ_ARCH=$(PRJ_ARCH) TARGET=release clean lib

# release
release: deps-release
	@echo "* $(PLATFORM) $(TARGET) rebuild..."
	@make -f Makefile TARGET=release clean clean_all all

# debug
debug: deps-debug
	@echo "* $(PLATFORM) $(TARGET) rebuild..."
	@make -f Makefile TARGET=debug clean clean_all all

# development
dev:
	@make -f Makefile TARGET=$(TARGET) clean clean_all all rpath install

# rpath
rpath:
	@echo "* $(PLATFORM) [$(TARGET)] fix rpath ..."
	@echo "+ ICU_LIB_DIR=$(ICU_LIB_DIR)"
	@echo "+ OPENSSL_LIB_DIR=$(OPENSSL_LIB_DIR)"
ifeq (Darwin, $(PLATFORM))
	@otool -L $(LIB_NAME).so
	@$(foreach lib,$(ICU_LIBS_FN), install_name_tool -change $(lib) $(ICU_LIB_DIR)/$(lib) $(LIB_NAME).so ;)
	@$(foreach lib,$(ICU_LIBS_FN), \
		declare -a ICU_LIBS_FN=($(ICU_LIBS_FN)) ;\
		for l2 in $${ICU_LIBS_FN[@]} ; do \
			echo "\t\t⌥ $${l2} vs $(lib)" ; \
			install_name_tool -change $${l2} $(ICU_LIB_DIR)/$${l2} $(ICU_LIB_DIR)/$(lib) ; \
		done \
	;)
	@$(foreach lib,$(OPENSSL_LIBS_FN), install_name_tool -change $(lib) $(OPENSSL_LIB_DIR)/$(lib) $(LIB_NAME).so ;)
	@otool -L $(LIB_NAME).so
else
	@ldd $(LIB_NAME).so
	@patchelf --set-rpath '$(ICU_LIB_DIR)/:$(OPENSSL_LIB_DIR)' $(LIB_NAME).so
	@ldd $(LIB_NAME).so
endif

# info
info:
	@echo "CC=$(CC)"
	@echo "CXX=$(CXX)"
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
