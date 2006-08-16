import sys,os,math

import ROOT
from   ROOT import TH1F,TCanvas,TF1
from   ROOT import gStyle,gROOT

import draw

##  @addtogroup T0
##  @{

##  The class which holds a card T0 fit results
#
class Card:
    ##  Constructor
    #
    #   @arg See the class attribute descriptions.
    #
    def __init__(self,det,card,chf,chl,pos,T0):

        ##  Detector name
        self.detector = det
        
        ##  The card number
        self.card     = card
        
        ##  First detector channel of the card.
        self.chf      = chf
        
        ##  Last detector channel of the card.
        self.chl      = chl
        
        ##  Card position.
        self.pos      = pos
        
        ##  Card @f$ T_0 @f$
        self.T0       = T0
        
        ##  Number of points in the V-plot
        self.n_points = None
    
    ##  String presenation of the card name.
    #
    #   It is used in dictionary indexing.
    #
    def __str__(self):
        return '%s card=%s' % (self.detector,self.card)

##  Some initialization code
#
#   Set ROOT options (TStyle)
#
def init():
    draw.set_default()
    #gStyle.SetTitleX(0.4)
    gStyle.SetTitleY(0.93)
    gStyle.SetStatColor(ROOT.kWhite)
    #gStyle.SetOptStat(0)

##  Set histogram bin errors
#
#   @arg \c h histogram
#   @arg \c error Bin error.
#
def set_errors(h,error):
    #print 'Setting errors to %g\n' % error
    for b in range(1,h.GetXaxis().GetNbins()+1):
        h.SetBinError(b,error)

## Book the 'dT' histogram
def book_h_dT(bins,dT_max=1):
    h = draw.DrawObj( TH1F('dT','T_{0}^{u} - T_{0}^{d} versus card',bins,0,bins) )
    h().GetXaxis() . SetTitle('Card')
    h().GetYaxis() . SetTitle('Time difference [ns]')
    h().SetMinimum(-dT_max)
    h().SetMaximum( dT_max)
    h().GetYaxis().SetTitleOffset(1.2)
    h().SetStats(False)
    #h.call(0,h().LabelsOption,['v','X'])
    h['TitleW'          ] = 0.4
    h['TitleX'          ] = (1-h['TitleW'])/2.
    h['TitleY'          ] = 0.95
    h['TitleH'          ] = 0.1
    return h

## Book the 'dT_proj' histogram
def book_h_dT_proj():
    gStyle.SetOptStat('er')
    gStyle.SetStatX(0.85)
    gStyle.SetStatY(0.85)
    h = draw.DrawObj( TH1F('dT_proj','T_{0}^{u} - T_{0}^{d}',100,-5,5) )
    h().GetXaxis().SetTitle('Difference T_{0}^{u} - T_{0}^{d}  [ns]')
    h().GetYaxis().SetTitle('Entries')
    #h().GetYaxis().SetTitleOffset(1)
    h['TitleW'          ] = 0.18
    h['TitleX'          ] = (1-h['TitleW'])/2.
    h['TitleH'          ] = 0.1
    h['TitleY'          ] = 0.95
    h['StatW'           ] = 0.13
    h['StatH'           ] = 0.1
    #h['StatX'           ] = 0.95
    #h['StatY'   ] = 0.9    # This does not work!
    #h['OptStat' ] = 'e'    # This does not work!
    return h

##  Make histograms for the @f$ T_0^u-T_0^d @f$ differences.
#   
#   @arg \c T0      Dictionary with the Card objects
#
#   The function creates two histograms:
#   - @f$ T_0^u-T_0^d @f$ versus card
#   - @f$ T_0^u-T_0^d @f$ for all card (Y-projection of the previos hisogram)
#
#   @return Dictionary with histograms.
#
def make_h_dT(T0,fit_range=1.5):

    dT={}

    for name,card_u in T0.items():
        if name[6]=='d':
            continue
        if name[6]!='u':
            print 'Bad name: %s' % name
            continue
    
        cid = name[:6]+'d'+name[7:]
        try:
            card_d = T0[cid]
        except:
            print '%s was not found!' % cid
            continue
        
        cid = name[:6]+'d'+name[7:]
        dT[cid] = card_u.T0-card_d.T0
        #if math.abs(dT[cid])>1:
        #    print cid,card_u.T0,card_d.T0

    h_dT      = book_h_dT(len(dT))
    h_dT_proj = book_h_dT_proj()
    
    bin=1
    for card,diff in dT.items():
        h_dT() . SetBinContent(bin,diff)
        h_dT() . GetXaxis() . SetBinLabel(bin,card)
        bin += 1
        
        h_dT_proj().Fill(diff)

    fit = TF1('fit','gaus')
    fit.SetParameters(h_dT().GetEntries(),0,0.1)
    fit.FixParameter(1,0)
    h_dT_proj().Fit(fit,'q0','',-fit_range,fit_range)
    h_dT_proj().GetFunction('fit').ResetBit(TF1.kNotDraw)
    set_errors(h_dT(),fit.GetParameter(2)/math.sqrt(2))

    objs = {}
    objs[h_dT()     .GetName()] = h_dT
    objs[h_dT_proj().GetName()] = h_dT_proj
    #objs[fit      .GetName()] = fit
    
    return objs
    
##  Read table of T0 measurements from a DB.
#
#   DB is accessed via comman line, so ther is no need to have a python MySQL module.
#   @arg \c table name.
#   @arg \c dbaccess access options to the mysql DB, example: "-hna58pc052.cern.ch"
#   
#   @return Dictionary of Card objects
def read_table(table,dbaccess):
    T0_card = {}

    command = "mysql -s -B %s -e 'DESCRIBE STDC.%s'" % (dbaccess,table)
    table_info = os.popen(command).readlines()
    if not table_info:
        raise 'Can not access the table!'

    command = "mysql -s -B %s -e 'select detector,comment,chf,chl,pos,T0,data from STDC.%s'" % (dbaccess,table)
    for f in os.popen(command):
        det,geo_str,geo,chf,chl,pos,t0,data = f.strip().split()
        geo  = int(geo)
        chf  = int(chf)
        chl  = int(chl)
        pos  = int(pos)
        t0   = float(t0)
        data = int(data)

        if not data:
            continue

        if len(det)!=8:
            raise 'Bad detector length.'
        if not det[6] in 'ud':
            raise 'Bad detector name.'
        
        card = Card(det,geo,chf,chl,pos,t0)
        card.n_points = data
        T0_card[str(card)] = card
    
    return T0_card

##  Read table of T0 measurements from a DB and analyze them.
#   
#   @arg For the arguments description see read_table() function
#
#   @return dictionary with the Card objects. The dictionary key is Card.__str__()
#
#   Example of usage:
#@code
#import dT_card
#d=dT_card.read_and_analyze('T0_geoID_q','-hna58pc052.cern.ch')
#@endcode
#
def read_and_analyze(table,dbaccess,plot_print=draw.Plot()):
    T0 = read_table(table,dbaccess)
    init()
    objs = make_h_dT(T0)
    return objs

if __name__ == '__main__':

    gROOT.SetBatch(1)

    try:
        import optparse
    except:
        sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
        import optparse

    parser = optparse.OptionParser()
    draw.optparse_parameters(parser,'eps','400x300')
    parser.add_option('', '--db-access',dest='dbaccess',default='-hna58pc052.cern.ch',
                      help='DB access options [default: %default]', type='string')

    parser.usage = '%prog [options] <session-name>'

    (options, args) = parser.parse_args()

    if len(args)!=1:
        parser.print_help()
        sys.exit(1)

    plot_print = draw.Plot(options.pic_ext,options.pic_size)
    objs = read_and_analyze(args[0],options.dbaccess,plot_print)

    for s in ['dT','dT_proj']:
        c = TCanvas('T0_'+s,'T0_'+s,plot_print.size_x,plot_print.size_y)
        objs[s].draw()
        plot_print.make(c)

    if options.root_file:
        draw.save_to_root(objs.values(),options.root_file,options.root_dir)

##  @}
