#
# Makefile to compile and link
# PHAST with CORAL.
#
#
# Dependent on CORAL Makefiles (to get CORAL "make" variables)
#


# by default "make" produce "coral.exe" - normal CORAL executable 
# but with module to write PHAST mDST
#
# possible parameters of "make" command:
#
# NO_FORTRAN=1 and/or NO_RFIO=1 - if your system do not have 
# Fortran compiler and/or RFIO library
#
# USR_PACK=1  -  to be able to call PHAST user functions 
#
# CORAL_IN_USERFUNC=1   -  create "pcol.exe" - normal PHAST executable
# but linked against CORAL libraries, so that user may use
# CORAL classes and functions in his ./user/User* code
#
# CORAL_DATE = YYYYMMDD - checkout date of CORAL
# (could be needed for backward compatibility with old CORALs)
# Try specify it if you have compilation problems	
# Default: if not specified, current "official" "PROD" version date is assumed.
# (See CORAL_DATE definition below)
#
#	   Sergei.Gerassimov@cern.ch
#
# some definitions
THIS=$(shell pwd)/////
LIBDIR=$(subst coral/////,lib,$(THIS))
USRDIR=$(subst coral/////,user,$(THIS))
SRCDIR=$(subst coral/////,src,$(THIS))
PHASTDIR=$(subst coral/////,,$(THIS))
export PHASTDIR
PREPROC = "-D PHAST_HOME="\"$(PHASTDIR)\"

# CORAL checkout/update date in format YYYYMMDD.
# Put real date here if there is a problem of compatiblility with older CORAL versions
#(hopefuly it will be automatized some time :-)
ifndef CORAL_DATE
  CORAL_DATE=20070522
endif
PREPROC += "-D CORAL_DATE = "$(CORAL_DATE)   

ALL_C   = $(wildcard *.cc)
ALL_O	= $(ALL_C:.cc=.o)
EXE  = coral.exe
PCOL = pcol.exe

ifdef CORAL_IN_USERFUNC
  USR_PACK=1
  PREPROC += "-D CORAL_IN_USERFUNC"
  default:	$(PCOL)          
else
  default:	$(EXE)
endif

ifeq ($(shell ls SLC4_64 2>/dev/null), SLC4_64)
  SLC4_64 = 1
endif


top_srcdir = $(CORAL)
include $(top_srcdir)/makefiles/Include.mk
include $(top_srcdir)/makefiles/General.mk

#
# Warning:
# Many variables 
# (such as C_INCLUDES, C_FLAGS, LIBS, etc)
# and some targets (depend, clean) are
# inherited from CORAL Include.mk and General.mk
#

PLIB = $(LIBDIR)/libPhast.so
MAKEPLIB = make -s -C $(LIBDIR) lib

ULIB = $(USRDIR)/libUser.so
MAKEULIB = make -s -C $(USRDIR) lib

MLIB = $(SRCDIR)/libMisc.so
MAKEMLIB = make -s -C $(SRCDIR) lib


#FORTRAN stuff switch
ifndef NO_FORTRAN
  ifdef NO_RFIO # remove -lshift
    export CERNLIB =`cernlib graflib grafX11 kernlib mathlib | sed -e 's/-lshift//'`
    ifdef SLC4_64
      export CERNLIB = -L/afs/cern.ch/sw/lcg/external/cernlib/2005/slc4_amd64_gcc34/lib/ \
	-L/usr/local/lib -lnsl -lcrypt -ldl -lkernlib -lpacklib -lmathlib
    endif
  else
    export CERNLIB =`cernlib graflib grafX11 kernlib mathlib`
    ifdef SLC4_64
      export CERNLIB = -L/afs/cern.ch/sw/lcg/external/cernlib/2005/slc4_amd64_gcc34/lib/ \
	-L/usr/local/lib -lshift -lnsl -lcrypt -ldl -lkernlib -lpacklib -lmathlib
    endif
  endif
else
  PREPROC += "-DNO_FORTRAN"
endif

# is used in ./user/Makefile ./lib/Makefile
export NO_FORTRAN 

#RFIO libraries switch
ifndef NO_RFIO
  RFIOLIB = "-lRFIO"
else
  PREPROC += "-DNO_RFIO"
endif

export ROOTLIB=`root-config --glibs` $(RFIOLIB)

#if users' will be called
ifdef USR_PACK
  PREPROC += "-D USR_PACK"
  MORE_LIBS =  $(ULIB) $(MLIB)
  MORE_LIBS_LINK = $(MLIB)
endif

# flags to be used in external Makefiles
export INCLUDE += $(C_INCLUDES) -I$(LIBDIR) -I$(USRDIR) -I$(SRCDIR) 
export FLAGS = -g -Wall -fPIC $(PREPROC) 
export F77FLAGS = -g -Wall -fPIC
export LD = $(CXX) -shared
export DLSUF = so

#az
Detectors = /afs/cern.ch/compass/detector/straw/Detectors/SLC4
export INCLUDE += -I$(Detectors)/include
LIBS           += $(Detectors)/lib/libDetectors.so -Xlinker -rpath -Xlinker /afs/cern.ch/compass/detector/straw/Detectors/SLC4/lib
#az

#Fortran compilation flAGS
export F77FLAGS= -g -Wall -fPIC

LOCAL_FLAGS =  -Wno-overloaded-virtual -fPIC -D PHASTinCORAL -DDEBUG 

ifndef SLC4_64
  export FLAGS    += -m32 
  export F77FLAGS += -m32
  LOCAL_FLAGS     += -m32 
endif

# make exe
$(EXE):	$(ALL_O) $(PLIB) $(MORE_LIBS) Depend.mk 
	@echo "Making " $(EXE)
	@$(CXX) -o $(EXE) $(FLAGS) $(C_FLAGS) $(ALL_O) $(MORE_LIBS_LINK) $(LIBS) $(PLIB) \
	$(CERNLIB) $(ROOTLIB) $(LIBS)

# make "phast with coral lib"
$(PCOL):	./main.o $(PLIB) $(MORE_LIBS) Depend.mk 
	@echo "Making " $(PCOL)
	@$(CXX) -o $(PCOL) $(FLAGS) $(C_FLAGS)./ main.o $(MORE_LIBS) $(LIBS) $(PLIB) \
	$(CERNLIB) $(ROOTLIB) $(LIBS)

Depend.mk:	
	@echo "Create dependencies file"
	@$(CXX) $(INCLUDE) $(ALL_C) -MM > Depend.mk

./%.o:	./%.cc
	@echo "Compilation: "$<
	@$(CXX) $(FLAGS) $(C_FLAGS) $(LOCAL_FLAGS) $(INCLUDE) -c $< -o $@

./main.o:	$(SRCDIR)/main.cc
	@echo "Compilation: "$<
	@$(CXX) $(FLAGS) $(C_FLAGS) $(LOCAL_FLAGS) $(INCLUDE) -c $< -o $@

# "clean" target already defined in included CORAL Makefiles
Clean:
	@echo "Cleaning ..."
	@rm -f  ./*.o ./*.mk ./Cint.* ./LinkDef.h ./*.exe ./coral/*~ 


$(PLIB):	$(LIBDIR)/*.h $(LIBDIR)/*.cc 
	@$(MAKEPLIB)

$(ULIB):	$(USRDIR)/*.h $(USRDIR)/*.cc 
	@$(MAKEULIB)

$(MLIB):	$(SRCDIR)/*.h $(SRCDIR)/*.cc 
	@$(MAKEMLIB)

show:	
	@echo "C_FLAGS  : "$(C_FLAGS)
	@echo "FLAGS    : "$(FLAGS)
	@echo "THIS     : "$(THIS)
	@echo "LIBDIR   : "$(LIBDIR)
	@echo "LIBS     : "$(LIBS)
	@echo "MAKELIB  : "$(MAKELIB)
	@echo "INCLUDE  : "$(INCLUDE)
	@echo "PHASTDIR : "$(PHASTDIR)

-include Depend.mk
















