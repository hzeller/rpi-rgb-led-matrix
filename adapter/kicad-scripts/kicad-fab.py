'''
    Based on gen_gerber_and_drill_files_board.py in kicad/demos directory.
'''

import sys

from pcbnew import *
filename=sys.argv[1]

board = LoadBoard(filename)

plotDir = "plot/"

pctl = PLOT_CONTROLLER(board)

popt = pctl.GetPlotOptions()

popt.SetOutputDirectory(plotDir)

# Set some important plot options:
popt.SetPlotFrameRef(False)
popt.SetLineWidth(FromMM(0.35))

popt.SetAutoScale(False)
popt.SetScale(1)
popt.SetMirror(False)
popt.SetUseGerberAttributes(False)
popt.SetUseGerberProtelExtensions(True)
popt.SetExcludeEdgeLayer(True);
popt.SetScale(1)
popt.SetUseAuxOrigin(True)

# This by gerbers only (also the name is truly horrid!)
popt.SetSubtractMaskFromSilk(False)

# param 0 is the layer ID
# param 1 is a string added to the file base name to identify the drawing
# param 2 is a comment
# Create filenames in a way that if they are sorted alphabetically, they
# are shown in exactly the layering the board would look like. So
#   gerbv *
# just makes sense.
plot_plan = [
    ( Edge_Cuts, "0-Edge_Cuts",   "Edges" ),

    ( F_Paste,   "1-PasteTop",    "Paste top" ),
    ( F_SilkS,   "2-SilkTop",     "Silk top" ),
    ( F_Mask,    "3-MaskTop",     "Mask top" ),
    ( F_Cu,      "4-CuTop",       "Top layer" ),

    ( B_Cu,      "5-CuBottom",    "Bottom layer" ),
    ( B_Mask,    "6-MaskBottom",  "Mask bottom" ),
    ( B_SilkS,   "7-SilkBottom",  "Silk top" ),
    ( B_Paste,   "8-PasteBottom", "Paste Bottom" ),
]


for layer_info in plot_plan:
    pctl.SetLayer(layer_info[0])
    pctl.OpenPlotfile(layer_info[1], PLOT_FORMAT_GERBER, layer_info[2])
    pctl.PlotLayer()

# At the end you have to close the last plot, otherwise you don't know when
# the object will be recycled!
pctl.ClosePlot()

# Fabricators need drill files.
# sometimes a drill map file is asked (for verification purpose)
drlwriter = EXCELLON_WRITER( board )
drlwriter.SetMapFileFormat( PLOT_FORMAT_PDF )

mirror = False
minimalHeader = False
offset = wxPoint(0,0)
mergeNPTH = True
drlwriter.SetOptions( mirror, minimalHeader, offset, mergeNPTH )

metricFmt = True
drlwriter.SetFormat( metricFmt )

genDrl = True
genMap = True
drlwriter.CreateDrillandMapFilesSet( plotDir, genDrl, genMap );
