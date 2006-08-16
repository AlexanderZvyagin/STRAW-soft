top_dir = ../../..
include $(top_dir)/Makefile.inc

INCLUDE  := -I.. -I$(top_dir)/src $(INCLUDE) `root-config --cflags`
LIBS     := $(LIBS)

all:    libVK.so

##########################################
# For Klaus code
GEMINI_BASE_DIR  =/afs/cern.ch/sw/lhcxx/specific/redhat73/gcc-3.2/Gemini/1.3.0.3
CERNLIB =/cern/pro/lib
LHCXX = /afs/cern.ch/sw/lhcxx/specific/redhat73/gcc-3.2/5.0.6/

CXXFLAGS  = `root-config --cflags`
CXXFLAGS += -I$(GEMINI_BASE_DIR)/include

LDFLAGS = -L$(GEMINI_BASE_DIR)/lib -lGemini-nag -lGemini-minuit -L$(CERNLIB)
#LDFLAGS += `root-config --libs`

LDFLAGS += -lGemini-nag -lGemini-minuit -lpacklib -lnsl -lcrypt -ldl -lnagc -lmathlib -lkernlib -lm -lg2c

LDFLAGS += -Xlinker -rpath $(GEMINI_BASE_DIR)/lib

LDFLAGS += -L$(LHCXX)/lib -Xlinker -rpath $(LHCXX)/lib

INCLUDE += -I$(GEMINI_BASE_DIR)/include

.SUFFIXES:	.cxx .hxx .ixx


.cxx.o:
	g++ $(CXXFLAGS) -c $<


libVK.so:   VK.o v_chi2.o $(top_dir)/src/RTRelation.o $(top_dir)/src/RTRelationGrid.o $(top_dir)/src/RTRelationPoly.o
	$(CXX) -shared -o $@ $^ $(LIBS) $(LDFLAGS)

VK:     V.o $(top_dir)/src/RTRelation.o $(top_dir)/src/RTRelationGrid.o $(top_dir)/src/RTRelationPoly.o v_chi2.o v_plot.ixx v_plot.hxx abs_pol.hxx abs_pol.ixx
	$(CXX) -o $@ v_chi2.o V.o $(top_dir)/src/RTRelation.o $(top_dir)/src/RTRelationGrid.o $(top_dir)/src/RTRelationPoly.o $(LIBS) $(LDFLAGS) `root-config --libs` -lMinuit -lpopt

SV: 	SV.o v_chi2.o v_plot.ixx v_plot.hxx abs_pol.hxx abs_pol.ixx
	g++ -o SV v_chi2.o SV.o $(LDFLAGS)

##########################################

clean:
	@rm -rf $(clean_mask) *.gif *.xml
	@rm -rf fit_Vn fit_V1 real make_straws convert_detdat xray submodule

install: all
	cp         straw_times        $(bindir)
	cp         convert_detdat     $(bindir)
	cp         xray               $(bindir)

test:


distclean: clean
	@rm -rf .depend *.root *.log *.out config.log config.status Makefile.inc

depend:
	$(CXX) -MM $(INCLUDE) $(CXXFLAGS) *.cc *.cxx > .depend

-include .depend
