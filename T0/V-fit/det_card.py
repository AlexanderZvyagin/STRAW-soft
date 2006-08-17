import sys,math

__map_prog__ = '/afs/cern.ch/user/z/zvyagin/DDD.git/examples/how-to/maps --run=37059'

from dT_card import read_table
import draw,Vs_card
from ROOT import TH1F, TCanvas
from ROOT import gROOT

##  @addtogroup T0
##  @{

##  Set bin content and label for detector T0 histogram
#
#   @arg \c h Histogram which holds @f$T_0^u@f$ and @f$T_0^d@f$ values for all cards.
#   @arg \c detector Detector name.
#   @arg \c card_data This is a Card object
#   @arg \c T0_err Error for @f$T_0@f$ calculation. The histogram bin errors will be set to it.
#
def set_histogram_bin(h,detector,card_data,T0_err):

    ud      = 'ud'.index(detector[6])
    cards_b = (7,6)[detector[4]=='Y']   # number of  6mm cards
    cards_c = (3,2)[detector[4]=='Y']   # number of 10mm cards
    abc     = (0,cards_c,cards_b+1+cards_c)['abc'.index(detector[7])]
    ph      = False

    chf = card_data.chf         # Make a short name!
   
    if      chf==0:
                    card = 0
    elif    chf==32:
                    card = 1
    elif    chf==64:
                    card = 2
    elif    chf==80:                    # Y chamber PH
                    card = 3
                    ph   = True
    elif    chf==95:                    # X-chamber PH region
        if  card_data.pos==-1:
                    card = 3            # normal card
        elif card_data.pos==1:
                    card = 4
                    ph   = True         # PH card
        else:
            raise 'Bad "pos" card value!'
    elif    chf==96:
                    card = 4            # Y-chamber card after a PH
    elif    chf==127:
                    card = 5            # X-card
    elif    chf==128:
                    card = 5            # Y-card
    elif    chf==158:
                    card = 6            # X-card
    elif    chf==160:
                    card = 6            # Y-card
    elif    chf==190:
                    card = 7            # X-card
    else:
            raise 'Unknown chf=%d pos=%d' % (card_data.pos)

    bin = 1+(card+abc)*2 + ud

    #print 'bin(%s chf=%d pos=%d) = %d' % (detector,chf,card_data.pos,bin)

    h().SetBinContent(bin,card_data.T0)
    h().SetBinError(bin,T0_err)

    label = '%s%s-%c %s' % (('','PH ')[ph],("10mm","6mm")[detector[7]=='b'],detector[6],card_data.card)

    if h().GetXaxis().GetBinLabel(bin):
        raise 'Bin label name already set: %s' % h().GetXaxis().GetBinLabel(bin)
    h().GetXaxis().SetBinLabel(bin,label)
    
    return bin

##  STRAW drift chamber detector short names for a given year.
#   
#   A short name is without ending two characteds, ex: \a ST03X1.
#
def straw_short_names(year):
    if year in [2002,2003,2004]:
        names = \
        ['ST03X1','ST03Y1','ST03U1',    \
         'ST03V1','ST03Y2','ST03X2',    \
         'ST04V1','ST04Y1','ST04X1',    \
         'ST05X1','ST05Y1','ST05U1',    \
         'ST06V1','ST06Y1','ST06X1']
    elif year in [2006]:
        names = \
        ['ST02X1','ST02Y1','ST02U1',    \
         'ST03V1','ST03Y2','ST03X2',    \
         'ST03X1','ST03Y1','ST03U1',    \
         'ST03V1','ST03Y2','ST03X2']
    else:
        raise 'The straw detector names for the year %d are not known' % year

    return names

##  STRAW drift chamber detector full names for a given year.
#   
#   An example of a full name: \a ST03X1db.
#
def straw_full_names(year):
    for n in straw_short_names(year):
        for ud in 'ud':
            for abc in 'abc':
                yield n+ud+abc

##  Create a histogram for the detector-T0 values.
#   
#   @arg \c name Histogram name
#   @arg \c title Histogram title
#   @arg \c bins Number of histogram bins
#
def book_hist_det_card(name,title,bins):
    h = draw.DrawObj(TH1F(name,title,bins,0,bins))
    h().GetXaxis().SetTitle('Card')
    h().GetXaxis().SetTitleOffset(3)
    h().GetYaxis().SetTitle('T_{0} time [ns]')
    h().GetYaxis().SetTitleOffset(1.4)
    h().SetStats(False)
    h.call(0,h().LabelsOption,['v','X'])
    h['TitleW'          ] = 0.2
    h['TitleX'          ] = (1-h['TitleW'])/2.
    h['TitleY'          ] = 0.95
    h['TitleH'          ] = 0.07
    return h

##  Create a latex string in math mode with the number \c n
#   
#   @arg \c n Number to be converted to latex string
#
def latex_number(n):
    if not n:
        return ''
    
    magnitude = int(math.log10(n))
    mantissa  = n/math.pow(10,magnitude)
    
    return '$%.1f \cdot 10^{%d}$' % (mantissa,magnitude)

##  Convert number to string, like: 1->B, 0->A, 100->BAA, 23->CD
#
def aNUM(n):
    try:
        n = int(n)
    except:
        return ''

    s = ''
    
    while n>0:
        s = 'ABCDEFGHIJ'[n%10] + s
        n /= 10
    return s

##  Get card number for a given histogram bin
#   
def get_card_number(detector,bin):

    card = (bin-1)/2

    cards_b = (7,6)[detector[4]=='Y']   # number of  6mm cards
    cards_c = (3,2)[detector[4]=='Y']   # number of 10mm cards
    card_ph = 3

    cards_b += 1    # a PH card!

    if   card>=cards_b+cards_c:
        abc = 'c'
        card -= cards_c + cards_b
    elif card>=cards_c:
        abc = 'b'
        card -= cards_c
    else:
        abc = 'a'

    det = detector+'u'+abc

    global __map_prog__
    chan = card*33
    if card>card_ph:
        chan -= 33
    prog = '%s --det=%s --chan=%d' % (__map_prog__,det,chan)
    
    import os,re    
    
    for out in os.popen(prog).readlines():
        r = re.match('(?P<det>\w+.)\s.*\swire=(?P<wire>\d+)\s.*\spos=(?P<pos>-*\d+)\s+.*geoID/port=(?P<geoID>\d+).*mode=(?P<mode>\d).*',out.strip())
        if r==None:
            continue

        det   = r.group('det')
        wire  = int(r.group('wire'))
        pos   = int(r.group('pos'))
        geoID = int(r.group('geoID'))
        mode  = int(r.group('mode'))

        if mode==0:
            continue    # Skip debug mode

        if pos!=0:
            if (pos==1)!=(card==card_ph):     # PH,  it is not like this always!  FIXME
                continue

    return str(geoID)

##  Create a histogram with the @f$ T_0 @f$ values for the given detector name
#   
#   @arg \c cards       Dictionary with the Card objects
#   @arg \c detector    Name of the detector for which we create the histogram
#   @arg \c T0_error    These errors will be aplied on the histogram bins
#
#   Example of usage:
#@code
#import dT_card, det_card
#from ROOT import TCanvas
#T0=dT_card.read_table('T0_geoID_i','')
#h=det_card.det_card(T0,'ST03X1db',0.2)
#c = TCanvas(h.GetName(),h.GetName())
#c.SetBottomMargin(0.25)
#c.SetLeftMargin(0.12)
#h.Draw()
#@endcode
#
def det_card(cards,detector,T0_error,t_range=10,T0_file=None):

    draw.set_default()

    nbins = 2 * (14,11)[detector[4]=='Y']
    name = '%6.6s_card_T0' % detector
    h = book_hist_det_card(name,name[:6],nbins)
    
    t0s = []
    n_points = {}
    n=0
    for name,card in cards.items():
        if name[:6]!=detector[:6]:
            continue
        bin = set_histogram_bin(h,name,card,T0_error)
        n_points[bin] = card.n_points
        
        t0s.append(card.T0)
        n += 1

    if not t0s:
        return h    # The histogram is empty!

    t0s.sort()
    t0s_median = t0s[len(t0s)/2]

    h().SetMinimum(t0s_median-t_range/2.)
    h().SetMaximum(t0s_median+t_range/2.)

    if T0_file:

        T0_file.write('\\begin{figure}[t]\n')
        T0_file.write('\\centering\n')
        T0_file.write('\\caption{Distribution of $T_0^u$ and $T_0^d$ values for {\\bf %s} detector.}\n' % detector)
        T0_file.write('\\label{fig:T0-%s}\n' % detector)
        T0_file.write('\\epsfxsize=355pt \\epsfbox{%s_card_T0.eps}\n' % detector)
        T0_file.write('\\end{figure}\n\n')

        T0_file.write('\\begin{table}[b]\n')
        T0_file.write('\\centering\n')
        T0_file.write('\\tiny\n')
        T0_file.write('\\caption{List of $T_0^u$ and $T_0^d$ for {\\bf %s} detector.}\n' % detector)
        T0_file.write('\\label{tbl:T0-%s}\n' % detector)
        T0_file.write('\\begin{tabular}{|c|c|c|c|c|c|c|} \\hline\n')
        T0_file.write('card & layer & data & $T_0$ & $|T_0^u-T_0^d|$ & $T_0^c$ & comment \\\\ \\hline\\hline\n')

        for bin in range(1,h().GetXaxis().GetNbins()+1,2):

            # Number of V-plot points
            n_ud=['\\ ','\\ ']
            for ud in range(2):
                try:
                    n_ud[ud] = latex_number(n_points[bin+ud])
                except:
                    pass

            # T0
            t0_u = '\\ '
            t0_d = '\\ '

            card = '\\ '

            label = h().GetXaxis().GetBinLabel(bin)
            if label:
                t0_u  = '%.2f' % h().GetBinContent(bin)
                

            label2 = h().GetXaxis().GetBinLabel(bin+1)
            if label2:
                t0_d  = '%.2f' % h().GetBinContent(bin+1)

            if not label:
                label = label2
            if label:
                n = label.find('mm-')
                if n<0:
                    print label
                card  = label[n+4:]
                label = label[:n+2]
                comment = '\\card%scomment' % aNUM(card)
            else:
                comment = ''

            if card=='\\ ':
                card = get_card_number(detector,bin)

            if not '\\ ' in [t0_u,t0_d]:
                dT     = '%.2f' % abs(h().GetBinContent(bin)-h().GetBinContent(bin+1))
                t0_avr = '%.2f' % ((h().GetBinContent(bin)+h().GetBinContent(bin+1))/2)
            else:
                dT     = ''
                t0_avr = ''

            #T0_file.write('%4s & %6s & %8s & %8s & %8s &  \\\\ \\hline \n' %
            #              (card,label,t0_u,t0_d,dT) )
            T0_file.write('\\parbox{11ex}{\\vspace{.7ex}%s \\newline %s\\vspace{.7ex}} & \n' % (card,label) )
            T0_file.write('\\parbox{2ex}{u  \\newline  d} & \n')
            T0_file.write('\\parbox{11ex}{%s \\newline %s} & \n' % (n_ud[0],n_ud[1]) )
            T0_file.write('\\parbox{11ex}{%s \\newline %s} & \n' % (t0_u,t0_d) )
            T0_file.write('%s &' % dT)
            T0_file.write('\\card%ssoft & %% t0_avr %s for %s %s\n' % ( (aNUM(card),0)[not card] ,t0_avr,detector,card) )
            T0_file.write('\\parbox{40ex}{%s}  %% card %s \n' % (comment,card) )
            T0_file.write('%%\\newcommand{\\card%scomment}{} %% %s %s\n' % (aNUM(card),detector,card) )
            T0_file.write('%%\\newcommand{\\card%ssoft}{}  %% %s %s %s  \n' % (aNUM(card),t0_avr,detector,card) )
            T0_file.write('\\\\ \\hline\n')

        T0_file.write('\\end{tabular}\n')
        T0_file.write('\\end{table}\n\n')
        T0_file.write('\\clearpage\n\n')

    return h

##  Create histograms with the @f$ T_0 @f$ values for the all detector names
#   
#   @arg \c cards       Dictionary with Card objects
#   @arg \c T0_error    These errors will be aplied on the histogram bins
#
#   @return List of histograms
#
#   Example of usage:
#@code
#import dT_card, det_card
#from ROOT import TCanvas
#T0=dT_card.read_table('T0_geoID_i','')
#hists=det_card.all_det_card(T0,0.2)
#objs={}
#for h in hists:
#    c = TCanvas('c_'+h.GetName(),'c_'+h.GetName())
#    c.SetBottomMargin(0.25)
#    c.SetLeftMargin(0.12)
#    h.Draw()
#    objs[h.GetName()] = h
#    objs[c.GetName()] = c
#@endcode
#
def all_det_card(cards,T0_error,t_range=10,T0_file=None):
    hists = []
    for det in straw_short_names(2004):
        hists.append(det_card(cards,det,T0_error,t_range,T0_file))
    return hists

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
                      help='DB access options (ex: -hhost -ume -ppass)', type='string')
    parser.add_option('', '--dt',dest='t_range',default=10.,
                      help='Histogram time range in ns, [default: %default]', type='float')
    parser.add_option('', '--t0-tex',dest='t0_file_tex',default=None,
                      help='File name for a table of T0s, [default: %default]', type='string')
    parser.add_option('', '--map-prog',dest='map_prog',
                      default=__map_prog__,
                      help='Program (with options) to get the mapping, [default: %default]', type='string')

    parser.usage = '%prog [options] <session-name> <detector> <T0-error>\n' \
                   'use the name "all" to print all detectors.'

    (options, args) = parser.parse_args()

    if len(args)!=3:
        parser.print_help()
        sys.exit(1)

    __map_prog__ = options.map_prog

    table    = args[0]
    detector = args[1]
    T0_error = float(args[2])

    plot_print = draw.Plot(options.pic_ext,options.pic_size)
    T0 = read_table(table,options.dbaccess)
    
    if options.t0_file_tex:
        f_tex = file(options.t0_file_tex,'w')
    else:
        f_tex = None
    
    if args[1]!='all':
        hists = [det_card(T0,detector,T0_error,options.t_range,f_tex)]
    else:
        hists = all_det_card(T0,T0_error,options.t_range,f_tex)

    for h in hists:
        c = TCanvas(h().GetName(),h().GetName(),plot_print.size_x,plot_print.size_y)
        c.SetBottomMargin(0.25)
        c.SetLeftMargin(0.12)
        h.Draw()
        plot_print.make(c)

    if options.root_file:
        draw.save_to_root(hists,options.root_file,options.root_dir)

##  @}
