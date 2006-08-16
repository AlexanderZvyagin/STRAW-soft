import os,sys,re
import Vextract, draw

import ROOT
from   ROOT import TCanvas
from   ROOT import gROOT

##  @addtogroup T0
##  @{

def setV_opts(V,detector,card):
    ROOT.gStyle.SetOptStat('e')
    ROOT.gStyle.SetStatX(0.9)
    ROOT.gStyle.SetStatY(0.9)
    Vdraw = draw.DrawObj(V)
    #Vdraw().SetStats(False)
    Vdraw().SetLineColor(ROOT.kRed)
    Vdraw().SetFillColor(ROOT.kRed)
    Vdraw().GetYaxis().SetTitleOffset(1.9)
    Vdraw().GetXaxis().SetTitle('Distance [cm]')
    Vdraw().GetYaxis().SetTitle('Time [ns]')
    #Vdraw().SetTitle('card %d of %s' % (card,detector) )
    Vdraw().SetMarkerColor(ROOT.kRed)
    Vdraw['TitleW'          ] = 0.4
    Vdraw['TitleX'          ] = (1-Vdraw['TitleW'])/2. + 0.05
    Vdraw['TitleY'          ] = 0.95
    Vdraw['TitleH'          ] = 0.06
    Vdraw.draw_opts = 'BOX'
    return Vdraw

def setRT_opts(RT):
    RTdraw = draw.DrawObj(RT)
    RTdraw.draw_opts = 'PL'
    RTdraw().SetMarkerStyle(4)
    RTdraw().SetMarkerSize(0.7)
    return RTdraw

##  Create a picture with the two V-plots from u/d layers of a card
#
#   @arg \c geo Card number
#   @arg \c d           Directory name with the ROOT files
#   @arg \c pic_name    Name for the picture, use '' to skip the picture creation.
#   @arg \c pic_ext     Picture extension, it defines the picture type
#   @arg \c pic_size    Picture size
#
#   Example of usage:
#@code
#import Vs_card
#from ROOT import TCanvas
#d=Vs_card.Vs_plot(145,'T0-p')
#@endcode
#
def Vs_plot(geo,d='./',plot_print=draw.Plot()):

    draw.set_default()

    Vud = {}

    files = {}

    for f in os.listdir(d):
        r = re.match('T0_(?P<det>........)_geoID(?P<geo>\d+)\.root',f)
        
        if not r:
            continue
        if int(r.group('geo'))!=geo:
            continue

        # OK, file matches
        detector = r.group('det')
        ud = detector[6]    # either 'u' od 'd'
        if ud not in 'ud':
            print 'Bad detector name: %s' % detector

        if Vud.has_key(ud):
            raise 'Attempt to add a second "%s" detector="%s"!' % (ud,detector)

        Vud[ud] = {}
        Vud[ud]['det' ] = detector[:6]+detector[7:]     # Detector name without 'u' or 'd'
        Vud[ud]['file'] = os.path.join(d,f)

    if len(Vud)!=2:
        print 'Not all files (must be 2, found %d) are available for card=%d!' % (len(Vud),geo)
        return
        
    if Vud['u']['det']!=Vud['d']['det']:
        print 'Print detector names are different!! "%s"!="%s"' % (Vud['u']['det'],Vud['d']['det'])
        return
    else:
        detector = Vud['u']['det']
    
    canvas_name = 'Vs_%s_card%d' % (detector,geo)
    ROOT.gStyle.SetPadLeftMargin(0.2)
    c = TCanvas(canvas_name,canvas_name,plot_print.size_x,plot_print.size_y)
    c.Divide(2,1)

    objs = {}
    
    for i in range(2):
        ud = 'ud'[i]
        V = Vextract.get_V(Vud[ud]['file'])
        objs['V'+ud] = V['TH2F']
        objs['RT'+ud] = V['TGraph']
        c.cd(i+1)
        #Vextract.draw_V_RT(V['TH2F'],V['TGraph'])
        
        V_draw  = setV_opts(V['TH2F'],Vud[ud]['det'],geo)
        RT_draw = setRT_opts(V['TGraph'])
        
        V_draw.Draw()
        RT_draw.Draw()

    c.cd(0)
    plot_print.make(c)
    objs[c.GetName()] = c
    
    return objs


if __name__=='__main__':

    gROOT.SetBatch(1)
    
    try:
        import optparse
    except:
        sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
        import optparse

    parser = optparse.OptionParser()
    draw.optparse_parameters(parser,'eps','400x300')
    parser.add_option('', '--dir',dest='dir',default='./',
                      help='Directory with the root files [default: "%default"]', type='string',metavar='PATH')

    parser.usage = '%prog [options] <geoID>'
    parser.description = 'Create a ROOT plot of two Vs from a given card geoghraphical address. ' + \
                         'The program will look for a files "T0_ST*_geoID*.root" and take histograms from them.'

    (options, args) = parser.parse_args()
    
    if len(args)!=1:
        parser.print_help()
        sys.exit(1)
    
    geo = int(args[0])

    print_plot = draw.Plot(options.pic_ext,options.pic_size)
    objs = Vs_plot(geo,options.dir,print_plot)

    if options.root_file:
        draw.save_to_root(objs.values(),options.root_file,options.root_dir)

##  @}
