import os,re,sys

import draw
from dT_card import Card
from calib import read_T0_from_latex

import ROOT

def read_coral_T0(file_name):
    det_T0 = {}
    for l in os.popen('cat %s' % file_name).readlines():
        det,T0 = l.strip().split()
        det_T0[det] = float(T0)
    return det_T0

def read_maps(prog_='/afs/cern.ch/user/z/zvyagin/DDD.git/examples/how-to/maps --run=37059 --maps=/afs/cern.ch/compass/detector/maps/2004.xml'):

    prog = '%s --det="ST.*" --chan=-1' % prog_
    maps={}

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

        maps[geoID]=det
    return maps

def make_plot(cards_t0,coral_t0,t0_error):
    coral_T0 = read_coral_T0(coral_t0)
    cards_T0  = read_T0_from_latex(cards_t0)    # dictionary:   card:T0
    
    maps=read_maps()

    t0s_diff={}

    h={}

    name='coral_t0_diff'
    h[name] = draw.DrawObj(ROOT.TH1F(name,'T_{0}^{card} - T_{0}^{detector}',100,-10,10))
    h[name]() . GetXaxis().SetTitle('T_{0}^{C} - T_{0}^{CORAL}   [ns]')
    h[name]() . GetYaxis().SetTitle('Entries')
    
    name='coral_t0_diff_det'
    h[name] = draw.DrawObj(ROOT.TH1F(name,'T_{0}^{card} - T_{0}^{detector} vs card',195,0,195))
    h[name]() . GetXaxis().SetTitle('Card')
    h[name]() . GetYaxis().SetTitle('T_{0}^{C} - T_{0}^{CORAL}   [ns]')
    h[name]() . SetMinimum(-15)
    h[name]() . SetMaximum( 15)
    h[name]() . SetStats(False)
    h[name]   . call(0,h[name]().LabelsOption,['v','X'])
    h[name]() . GetXaxis() . SetLabelOffset(100)

    for card,T0 in cards_T0.items():
        #print card,maps[card],T0,coral_T0[maps[card]]
        detector = maps[card]
        h['coral_t0_diff']().Fill(T0-coral_T0[detector])
        try:
            t0s_diff[detector]
        except:
            t0s_diff[detector] = {}
        t0s_diff[detector][card] = T0 - coral_T0[detector]

    hist = h['coral_t0_diff_det']()
    bin=1
    for det,cards in t0s_diff.items():
        for card,T0_diff in cards.items():
            hist.SetBinContent(bin,T0_diff)
            hist.SetBinError(bin,t0_error)
            hist.GetXaxis().SetBinLabel(bin,'%s %d' % (det,card))
            bin = bin+1

    return h

if __name__=='__main__':
    ROOT.gROOT.SetBatch(1)
    draw.set_default()
    
    try:
        import optparse
    except:
        sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
        import optparse

    parser = optparse.OptionParser()
    draw.optparse_parameters(parser,'eps','400x300')
    parser.add_option('', '--dir',dest='dir',default='./',
                      help='Directory with the root files [default: "%default"]', type='string',metavar='PATH')

    parser.usage = '%prog [options] <cards.tex> <coral-t0.txt> <t0_error>'
    parser.description = 'Create plots for CORAL:T0 CARDS:T0 difference.'

    (options, args) = parser.parse_args()
    
    if len(args)!=3:
        parser.print_help()
        sys.exit(1)
    
    print_plot = draw.Plot(options.pic_ext,options.pic_size)
    objs = make_plot(args[0],args[1],float(args[2]))

    for o in objs.values():
        c = ROOT.TCanvas(o().GetName(),o().GetTitle(),print_plot.size_x,print_plot.size_y)
        o.draw()
        print_plot.make(c)

    if options.root_file:
        draw.save_to_root(objs.values(),options.root_file,options.root_dir)
