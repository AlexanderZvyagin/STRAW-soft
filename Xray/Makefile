top_dir=../..
include $(top_dir)/Makefile.inc

INCLUDE  := -I$(top_dir)/src $(INCLUDE) `root-config --cflags`
LIBS     := -L$(top_dir)/src -lDetectors -Wl,-rpath,$(shell pwd)/$(top_dir)/src $(LIBS) -lshift
#DATELIB  =
#DATELIB  = /afs/cern.ch/compass/online/daq/dateV371/monitoring/Linux/libmonitor.a
#EXTRA_LIBS = /afs/cern.ch/compass/online/daq/dateV371/monitoring/Linux/libmonitor.a -lshift
#EXTRA_LIBS =


all:    fill_xray xray

fill_xray:     fill_xray.o
	$(CXX) `root-config --libs` -lMinuit $(CFLAGS) -o $@ $^ $(LIBS)

Xray_show:     Xray_show.C
	$(CXX) `root-config --libs --cflags` -o $@ $^

xray:   xray.o
	$(CXX) `root-config --libs` -o $@ $^ $(LIBS) `root-config --libs` -lMinuit -lssl -lpopt

install: all
	cp         xray               $(bindir)

test:

clean:
	@rm -rf $(clean_mask) *.gif *.xml
	@rm -rf fill_xray xray

distclean: clean
	@rm -rf .depend *.root *.log *.out config.log config.status Makefile.inc

depend:
	$(CXX) -MM $(INCLUDE) *.cc > .depend

-include .depend
