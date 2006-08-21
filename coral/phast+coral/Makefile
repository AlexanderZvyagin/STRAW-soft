TARGET = coral.exe

ifndef CORAL 
    error_msg += "Missing CORAL variable"
    TARGET = error
endif

ifndef PHAST
    error_msg += "Missing PHAST variable"
    TARGET = error
endif

target:         $(TARGET)

top_srcdir = $(CORAL)

include $(CORAL)/makefiles/Include.mk
include $(CORAL)/makefiles/General.mk

SRCS            = $(wildcard *.cc) ${PHAST}/coral/main_for_CORAL.cc
OBJS	        = $(SRCS:.cc=.o)

coral.exe:      $(OBJS)
		$(CXX) -o $@ $^ $(LIBS) -lssl

error:
		@echo $(error_msg)

.cc.o:
		$(CXX) $(C_INCLUDES) ${C_FLAGS} -c $*.cc -o $*.o
