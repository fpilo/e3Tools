###########################################################################
#
#   Makefile.inc
#
#  
#
#
###########################################################################

###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################

#-------------------------------------------------
# specify OS target, which further differentiates
# the underlying OS; supported values are:
# macosx64, linux
#-------------------------------------------------

ifndef TARGETOS

UNAME = $(shell uname -a)

ifeq ($(firstword $(filter Linux,$(UNAME))),Linux)
TARGETOS = Linux
endif

# Autodetect PTR64
ifndef PTR64
ifeq ($(firstword $(filter x86_64,$(UNAME))),x86_64)
PTR64 = 1
endif
endif

endif #TARGETOS

#-------------------------------------------------
# specify program options; see each option below 
# for details
#-------------------------------------------------

# uncomment next line to build a debug version
DEBUG = 1

#-------------------------------------------------
# specify build options; see each option below 
# for details
#-------------------------------------------------
  
# uncomment next line if you want to force building for a 64-bit target
# PTR64 = 1

# uncomment next line to generate verbose build information
# VERBOSE = 1


###########################################################################
#################    END USER-CONFIGURABLE OPTIONS    #####################
###########################################################################

#-------------------------------------------------
# compile-time definitions
#-------------------------------------------------

DEL = rm -rf 

#-------------------------------------------------
# ROOT FLAGS
#-------------------------------------------------

ROOTCFLAGS       := $(shell root-config --cflags)
ROOTLDFLAGS     := $(shell root-config --ldflags)
allROOTLIBS 	    := $(shell root-config --libs) -lMinuit -lTreePlayer
misslibs 		   = -lPhysics
ROOTLIBS  		   = $(filter-out $(misslibs),$(allROOTLIBS))
#ROOTGRAPHICSLIBS := -lGui -lGeom -lGeomPainter -lHistPainter
ROOTGLIBS     	 := $(shell root-config --glibs) -lGpad -lGX11 -lX3d -lGeom -lGeomPainter -lHistPainter

#-------------------------------------------------
# platform-specific definitions
#-------------------------------------------------

# compiler, linker and utilities

CXX	:= g++ 
FC		:= gfortran 
LD 	        := g++
SHARED	:= -shared -fPIC

#-------------------------------------------------
# compile flags
#-------------------------------------------------

# no warning if when compiling C, give the address of one const char[length] 
# a non-const char * pointer
CXXFLAGS:= -O3 -fPIC -Wall -Wno-write-strings -fexceptions

# add -v if we need verbose build information
ifdef DEBUG
CXXFLAGS += -g
endif

# add -v if we need verbose build information
ifdef VERBOSE
CXXFLAGS += -v
endif

CXXFLAGS += $(ROOTCFLAGS)

# Fortran flags
FCFLAGS:= -O3 -fPIC -Wall

#-------------------------------------------------
# link flags
#-------------------------------------------------

LDFLAGS	:= -O3
ifeq ($(PTR64),1)
LDFLAGS += $(ROOTLDFLAGS)
endif

#-------------------------------------------------
#-------------------------------------------------
