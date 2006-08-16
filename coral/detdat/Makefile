top_dir         = ../../..
include         $(top_dir)/Makefile.inc

DDD             = /afs/cern.ch/user/z/zvyagin/coral/src/DaqDataDecoding

INCLUDE         := -I$(top_dir)/src $(INCLUDE) `root-config --cflags`
#LIBS            := $(LIBS) `root-config --libs` -lpopt

all:            detdat


LIB_short       = $(top_dir)/src/libDetectors.a -L$(DDD)/src -lDaqDataDecoding -L$(DDD)/expat/xmlparse -lexpat
detdat:         detdat.o $(top_dir)/src/libDetectors.a
		$(CXX) `root-config --libs` -o $@ $^ -lMatrix $(LIB_short)  `root-config --libs` -lMinuit -lpopt

clean:
	@rm -rf $(clean_mask)
	@rm -rf detdat

distclean: clean
	@rm -rf .depend *.root *.log *.out *.ps config.log config.status Makefile.inc
	@for dir in $(plugins_list); do $(MAKE) -C $$dir distclean; done;

depend:
	$(CXX) -MM $(INCLUDE) *.cc > .depend
	@for dir in $(plugins_list); do $(MAKE) -C $$dir depend; done;

-include .depend
