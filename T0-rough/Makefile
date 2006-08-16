.PHONY:         ddd

DDD             = $(HOME)/DDD.git
include         $(DDD)/Makefile.inc

#DATA = /home/zvyagin/cdr21019-37059.raw
DATA            = /castor/cern.ch/compass/data/2004/raw/W27/cdr09001-37059.raw
OPTS            = --maps=/afs/cern.ch/compass/detector/maps/2004.xml

CXXFLAGS        += -I$(DDD)/src -I`root-config --incdir`

all:            straw_times.so ddd

ddd:
		@ln -sf $(DDD)/examples/how-to/ddd

LIBS            = -L$(DDD)/src -lDaqDataDecoding -L$(DDD)/expat/xmlparse -lexpat

straw_times.so: straw_times.cc
		$(CXX) $(CXXFLAGS) -shared -o $@ $+ `root-config --libs` $(LIBS)

run:            all
		$(DDD)/examples/how-to/ddd --plugin=$(PWD)/straw_times.so $(OPTS) $(DATA)
clean:
		@rm -rf *.so *.a *.o *.root *.ps *.log *.so

distclean:
		make clean
		@rm -fr .depend Makefile.inc config.h config.status config.cache

depend:
		$(CXX) $(CXXFLAGS) -I. -MM *.cc > .depend

-include        .depend
