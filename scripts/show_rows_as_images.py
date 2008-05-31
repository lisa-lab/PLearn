#!/usr/bin/env python

import sys
from plearn.vmat.PMat import PMat
from plearn.plotting.netplot import showRowsAsImages

####################
### main program ###

if __name__ == "__main__":

    try:
        datapmatfile, imgheight, imgwidth, nrows, ncols = sys.argv[1:]
        imgheight = int(imgheight)
        imgwidth = int(imgwidth)
        nrows = int(nrows)
        ncols = int(ncols)
    except:
        print "Usage: "+sys.argv[0]+" <datafile.pmat> <imgheight> <imgwidth> <nrows> <ncols>"
        print """
        Will load a pmat in memory and consider the beginning of each row a imgheight x imgwidth imagette.
        These will be interactively displayed in a nrows x ncols grid of imagettes.
        """
        print "Ex: "+sys.argv[0]+" /home/fringant2/lisa/data/faces/olivetti/faces.pmat 64 64  5 7"
        print "Ex: "+sys.argv[0]+" /home/fringant2/lisa/data/icml07data/mnist_basic/plearn/mnist_basic2_train.pmat 28 28 5 7"
        raise
    # sys.exit()

    data = PMat(datapmatfile)
    showRowsAsImages(data, img_height=imgheight, img_width=imgwidth, nrows=nrows, ncols=ncols, figtitle=datapmatfile)



