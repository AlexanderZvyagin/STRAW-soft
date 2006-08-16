top_dir=../../..
include $(top_dir)/Makefile.inc

INCLUDE  := -I.. -I$(top_dir)/src $(INCLUDE) `root-config --cflags`
LIBS     := -L$(top_dir)/src -lDetectors -Wl,-rpath,$(shell pwd)/$(top_dir)/src $(LIBS) -lshift

libVS.so:   VS.o
	$(CXX) -shared -o $@ $^

clean:
	@rm -rf $(clean_mask) *.gif *.xml
	@rm -rf V

distclean: clean
	@rm -rf .depend *.root *.log *.out *.ps config.log config.status Makefile.inc

depend:
	$(CXX) -MM $(INCLUDE) *.cc > .depend

-include .depend
