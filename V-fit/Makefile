top_dir=../..
include $(top_dir)/Makefile.inc

INCLUDE  := -I$(top_dir)/src $(INCLUDE) `root-config --cflags`
LIBS     := $(LIBS)

.PHONY: VS VK VKS
all:    V

plugins_list = VS

V:      V.o Vmain.o VS/VS.o $(top_dir)/src/RTRelation.o $(top_dir)/src/RTRelationGrid.o $(top_dir)/src/RTRelationPoly.o 
	$(CXX) `root-config --libs` -o $@ $^ $(LIBS) $(LDFLAGS) `root-config --libs` -lMinuit -lpopt -lssl

plugins:
	@for dir in $(plugins_list); do $(MAKE) -C $$dir; done;

vcmp:   vcmp.o
	$(CXX) -o $@ $^ `root-config --libs` -lMinuit -lssl -lpopt

clean:
	@rm -rf $(clean_mask) *.gif *.xml
	@rm -rf V
	@for dir in $(plugins_list); do $(MAKE) -C $$dir clean; done;

test:

distclean: clean
	@rm -rf .depend *.root *.log *.out *.ps config.log config.status Makefile.inc
	@for dir in $(plugins_list); do $(MAKE) -C $$dir distclean; done;

depend:
	$(CXX) -MM $(INCLUDE) *.cc > .depend
	@for dir in $(plugins_list); do $(MAKE) -C $$dir depend; done;

-include .depend
