.PHONY:         clean test depend doc

FLAGS           = -O3

.cc.o:
		g++ $(FLAGS) `root-config --cflags` -c $<

OBJS            = dcs.o dcs_cint.o db_write.o db_read.o ChannelStatus.o \
                  db_detectors_channels.o nominal_HV.o time-f.o draw.o MainCanvas.o

mysql_libs_dir  = $(patsubst -L%,%,$(word 1,$(shell mysql_config --libs)))

dcs:            $(OBJS)
		g++ $(FLAGS) -o $@ $^ -lpopt `root-config --cflags --glibs`

dcs_cint.cc:    dcs.h MainCanvas.h LinkDef.h
		rootcint -f dcs_cint.cc -c dcs.h MainCanvas.h LinkDef.h

doc:            Doxyfile *.h *.cc help.doc
		@rm -rf doc
		@grep PROJECT_NUMBER Doxyfile
		@doxygen Doxyfile

version:        
		@echo -n 'Before:' && grep PROJECT_NUMBER Doxyfile
		@echo -n 'Before:' && grep version= version.h
		@cp Doxyfile Doxyfile.tmp
		@sed 's/PROJECT_NUMBER         = .*/PROJECT_NUMBER         = $(VERSION)/' Doxyfile.tmp > Doxyfile
		@rm Doxyfile.tmp
		@cp version.h version.h.tmp
		@sed 's/const char \*version=.*/const char \*version="$(VERSION)";/' version.h.tmp > version.h
		@rm version.h.tmp
		@echo -n 'After:' && grep PROJECT_NUMBER Doxyfile
		@echo -n 'After:' && grep version= version.h

clean:
		@rm -rf .depend dcs dcs_cint.* *.o doc

test:           dcs
		./dcs --t1=20040625 --t2=20040629

install_dir     = /afs/cern.ch/compass/detector/straw
install_bin     = $(install_dir)/bin
install_doc     = $(install_dir)/doc/dcs
install:        dcs doc
		cp -a dcs $(install_bin)
		rm -rf $(install_doc)
		cp -a doc $(install_doc)

depend:
		@g++ -MM `root-config --cflags` *.cc > .depend

-include        .depend
