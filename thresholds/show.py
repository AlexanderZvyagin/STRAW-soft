import sys,re
import mapping
import ROOT

channel_axis_label = 'CATCH: card_channel+port*64'

def create_hists(file_thresholds):

    r = re.match('.*threshold_(?P<catch>\d+)\.dat.*',file_thresholds)
    if r==None:
        raise 'Bad file name: "%s"' % file_thresholds

    catch = int(r.group('catch'))
    name = 'catch_%d' % catch

    content = open(file_thresholds).readlines()

    if len(content)!=1024:
        raise 'Bad number of lines %d in the file "%s"' % (len(content),file_thresholds)

    rates_lim = 30
    global channel_axis_label
    
    h_rate = ROOT.TH1F('h_rate_%d' % catch,"Rates",1024,0,1024)
    h_rate.GetXaxis().SetTitle(channel_axis_label)
    h_rate.GetYaxis().SetTitle('Noise rate per channel, kHz')
    h_rate.SetDirectory(None)
    h_rate.SetMinimum(0)
    h_rate.SetMaximum(rates_lim)
    h_rate.SetStats(0)

    h_rate_proj = ROOT.TH1F('h_rate_prof_%d' % catch,'Rates projection',100,0,rates_lim);
    h_rate_proj.SetDirectory(None);
    h_rate_proj.GetXaxis().SetTitle('Rate, kHz')
    h_rate_proj.GetYaxis().SetTitle('Entries')

    h_thr = ROOT.TH1F('h_thr_%d' % catch,'Thresholds on catch %d' % catch,1024,0,1024);
    h_thr.GetXaxis().SetTitle(channel_axis_label)
    h_thr.GetYaxis().SetTitle('Threshold, mV')
    h_thr.SetDirectory(None)
    h_thr.SetMinimum(-2000)
    h_thr.SetMaximum(-300)
    h_thr.SetStats(0)

    # Read content of the file.

    for l in content:
        
        r = re.match('\s*(?P<port>\d+)\s+(?P<chan>\d+)\s+(?P<thr>\d+)(\s+(?P<rate>[\d\.]+).*)',l)
        
        if r==None:
            raise 'In file "%s", bad line: "%s"' % (file_thresholds,l)
        
        port = int(r.group('port'))
        chan = int(r.group('chan'))
        thr  = int(r.group('thr'))
        rate = r.group('rate')
        
        assert port>=0 and port<16
        assert chan>=0 and chan<64
        
        bin  = 1+chan+port*64
        h_thr.SetBinContent(bin,(thr-128)*2040/128.)
        
        if rate:
            rate = float(rate)

            h_rate.SetBinContent(bin,rate)

            if rate>0:
                h_rate_proj.Fill(rate)
        
    objs = (h_rate,h_rate_proj,h_thr)

    return (catch,objs)

def draw_mapping(h,h_objs,maps,catch,where=0):

    for port in range(16):
        pos_port = 0
        pos_det  = 0

        r = h.GetMaximum()-h.GetMinimum()
        if where==0:
            # histogram bottom
            pos_port = h.GetMinimum()+r*0.0
            pos_det  = h.GetMinimum()+r*0.07
        else:
            # histogram top
            pos_port = h.GetMaximum()-r*0.07
            pos_det  = h.GetMaximum()-r*0.5
    
        l = ROOT.TLine(port*64,h.GetMinimum(),port*64,h.GetMaximum())
        l.SetLineStyle(2)
        l.Draw()
        h_objs.append(l)
        
        # FIXME
        txt = ROOT.TText(10+port*64,pos_port,str(port))
        txt.Draw()
        h_objs.append(txt)

        try:
            card = maps['%d %d' % (catch,port)]
        except:
            card = ''
            continue

        offset = 40
        mm = ROOT.TText(offset+port*64,pos_det,str(card))
        mm.SetTextAngle(90)
        mm.SetTextSize(0.04)
        mm.Draw()
        h_objs.append(mm)

__h_objs__ = []
def fill_canvases(canvases,maps,files,color=None):
    colors = {'black':ROOT.kBlack, 'red':ROOT.kRed, 'green':ROOT.kGreen, 'blue':ROOT.kBlue}
    try:
        color = colors[color]
    except:
        pass

    for f in files:

        # Parse the file
        catch,objs = create_hists(f)

        # Set color for histograms
        if color:
            for o in objs:
                o.SetLineColor(color)

        # Create or change the canvas
        try:
            canvas = canvases[catch]
            opt = 'SAME'
        except:
            canvases[catch] = ROOT.TCanvas('catch_%d' % catch)
            canvas = canvases[catch]
            canvas.Divide(2,2)
            opt = ''

        if len(objs)>4:
            raise 'Too many histograms were created!'
        global __h_objs__

        for h in objs:
            __h_objs__.append(h)

        canvas.cd(1)
        objs[0].Draw(opt)
        draw_mapping(objs[0],__h_objs__,maps,catch,1)

        canvas.cd(2)
        objs[1].Draw(opt)

        canvas.cd(3)
        objs[2].Draw(opt)
        draw_mapping(objs[2],__h_objs__,maps,catch,0)
        
        if opt=='SAME':
            pad = canvas.cd(3)

            it = ROOT.TIter(pad.GetListOfPrimitives())
            hh={}
            while True:
                obj = it.Next()
                if not obj: break
                if obj.Class().GetName()!='TH1F':
                    continue
                hh[obj==objs[2]] = obj

            assert len(hh)==2
            
            canvas.cd(4)
            h_thr_diff = make_thr_diff(hh[0],hh[1])
            h_thr_diff.Draw()
            __h_objs__.append(h_thr_diff)
            draw_mapping(h_thr_diff,__h_objs__,maps,catch,0)

def make_thr_diff(h1,h2):
    #ROOT.gPad.SetGrid(0,1)
    global channel_axis_label

    h_thr_diff = ROOT.TH1F('h_thr_diff','Thresholds difference',1024,0,1024)
    h_thr_diff.SetStats(0)
    h_thr_diff.GetXaxis().SetTitle(channel_axis_label)
    h_thr_diff.GetYaxis().SetTitle('Thresholds difference, mV')
    h_thr_diff.SetDirectory(None)

    h_thr_diff.Add(h1,h2,1,-1)
    h_thr_diff.SetMinimum(-800)
    h_thr_diff.SetMaximum( 800)
    return h_thr_diff

if __name__=='__main__':

    import optparse

    parser = optparse.OptionParser()
    parser.add_option('', '--maps',dest='maps',default='STRAW.xml',
                      help='STRAW map file [default=%default]', type='string')
    parser.add_option('', '--run',dest='run',default=0,
                      help='Run number (to access mapping info)', type='int')
    parser.add_option('', '--files',dest='files',
                      help='List of threshold files.', type='string')
    parser.add_option('', '--color',dest='color',default='black',
                      help='Color to be used to draw histograms [default=%default]', type='string')
    parser.add_option('', '--files2',dest='files2',
                      help='List of threshold files.', type='string')
    parser.add_option('', '--color2',dest='color2',default='red',
                      help='Color to be used to draw histograms [default=%default]', type='string')

    parser.usage = '%prog [options] <list of threshold files>\n' \
                   'Example1:  python show.py --files=threshold_322.dat\n' \
                   'Example2:  python show.py --color=green --files="`echo tests/01/threshold_3*`" --color2=red --files2="`echo tests/05/threshold_3*`"'
    (options, args) = parser.parse_args()

    #if len(args)==0:
    #    parser.print_help()
    #    sys.exit(1)

    maps = mapping.read_mapping(options.maps,options.run)
    
    canvases = {}

    if options.files:
        fill_canvases(canvases,maps,options.files .split(),options.color )
    if options.files2:
        fill_canvases(canvases,maps,options.files2.split(),options.color2)

    ps = ROOT.TPostScript('thresholds.ps',112)

    for catch,canvas in canvases.iteritems():
        ps.NewPage()
        canvas.cd(0)
        #canvas.Update()
        canvas.Draw()
        canvas.Update()
        #canvas.Print()
    
    ps.Close()
