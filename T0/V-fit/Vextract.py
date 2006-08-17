import sys
from ROOT import TFile, TIter, TCanvas

##  @addtogroup T0
##  @{

## Get a V-plot (histogram) and its RT from the ROOT file.
#
#   @arg \c filename Name of the ROOT file
#   @return Dictionary with all found TPad objects.
#   @warning The code relays on the page structure from the V.cc file
#
#   Here is a code example:
#@code
#d=get_V('file.root')
#c = TCanvas()
#draw_V_RT(d['TH2F'],d['TGraph'])
#@endcode
#
def get_V(filename):
    f = TFile(filename)
    
    nextobj = TIter(f.GetListOfKeys())

    while True:
        obj = nextobj.Next()
        if not obj:
            return None

        c = f.Get(obj.GetName())
        pad = c.FindObject('pad_main')
        if not pad:
            continue
        
        # We found an object with the pad 'pad_main'
        
        
        pad = pad.GetPad(1)
        
        # Now 'pad' must have a V-plot
        
        d={}
        
        it = TIter(pad.GetListOfPrimitives())
        while True:
            obj = it.Next()
            if not obj: break
            d[obj.Class().GetName()] = obj

        return d

##  Draw a V-plot and its RT in the current TCanvas
#
#   @arg \c V      The V-plot (TH2F histogram)
#   @arg \c RT     RT-function (TGraph)
#
def draw_V_RT(V,RT):
    V.Draw()
    RT.SetMarkerStyle(4)
    RT.SetMarkerSize(0.4)
    RT.Draw('PL')

if __name__=='__main__':
    
    try:
        import optparse
    except:
        sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
        import optparse

    parser = optparse.OptionParser()
    parser.add_option('', '--name',dest='name',default='V',
                      help='Picture name [default: "%default"]', type='string',metavar='NAME')
    parser.add_option('', '--ext',dest='ext',default='.eps',
                      help='Picture type [default: "%default"]', type='string',metavar='NAME')

    parser.usage = '%prog [options] <V-plot-file.root>'

    (options, args) = parser.parse_args()
    
    if len(args)!=1:
        parser.print_help()
        sys.exit(1)


    d=get_V(args[0])

    c = TCanvas(options.name,options.name)
    draw_V_RT(d['TH2F'],d['TGraph'])
    c.Print('','.'+options.ext)

##  @}
