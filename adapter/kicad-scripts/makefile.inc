# -*- Makefile -*-

%-fab.zip : %.kicad_pcb
	python ../kicad-scripts/kicad-fab.py $<
	zip -r $@ plot/
