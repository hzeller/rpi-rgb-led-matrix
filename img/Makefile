
coordinates.png:

%.png : %.fig
	fig2dev -L pdf $^ | pdftoppm | pnmtopng > $@
