TARGET = coral.exe

ifndef CORAL 
    error_msg += "Missing CORAL variable"
    TARGET = error
endif

ifndef PHAST
    error_msg += "Missing PHAST variable"
    TARGET = error
endif

ifndef Detectors
    error_msg += "Missing Detectors variable"
    TARGET = error
endif

target:         $(TARGET)

top_srcdir = $(CORAL)

include $(CORAL)/makefiles/Include.mk
include $(CORAL)/makefiles/General.mk

C_INCLUDES += -I${Detectors}/include
LIBS       += -L${Detectors}/lib -lDetectors

SRCS            = $(wildcard *.cc) ${PHAST}/coral/main_for_CORAL.cc
#SRCS            = CoralDet.cc CoralUser.cc ${PHAST}/coral/main_for_CORAL.cc
OBJS	        = $(SRCS:.cc=.o)

coral.exe:      $(OBJS)
		$(CXX) -o $@ $^ $(LIBS) -lssl

clean:
		@rm -rf *.o coral.exe

error:
		@echo $(error_msg)

.cc.o:
		$(CXX) $(C_INCLUDES) ${C_FLAGS} -c $*.cc -o $*.o
