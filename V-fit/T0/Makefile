all:    T0-geoID-report.ps

T0-geoID-report.ps: T0-geoID-report.dvi
	dvips  T0-geoID-report.dvi -o T0-geoID-report.ps

T0-geoID-report.pdf: T0-geoID-report.dvi
#	TEXINPUTS=:$(PWD)/texslides
	dvipdf  T0-geoID-report.dvi T0-geoID-report.pdf
#	mv et.pdf zvyagin_060601.pdf

T0-geoID-report.dvi: T0-geoID-report.tex t0-tables.tex cards.tex *.eps
	latex T0-geoID-report && latex T0-geoID-report

ps:     T0-geoID-report.dvi
	dvips -o T0-geoID-report.ps T0-geoID-report.dvi

pdf:   T0-geoID-report.pdf
	acroread T0-geoID-report.pdf

view:   T0-geoID-report.dvi
	xdvi -s 4 T0-geoID-report.dvi

clean:
	@rm -f *.aux *.dvi *.log *.out *.ps *.pdf *.eps *.gif
