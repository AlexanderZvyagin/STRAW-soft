# mysql -B -s -e 'select detector,channel,time,current from dcs.current' |bzip2 > ~/current/current.txt.bz2

import ROOT
import os,re,sys

def I_graph(det,chan,file_name,t1,t2):

    gr = ROOT.TGraph()
    h  = ROOT.TH1F('h_I','Current distr in [%s,%s]' % (t1.AsString(),t2.AsString()),200,0,2e-6)

    for l in os.popen('bzcat %s' % file_name):
        r = re.match('(?P<det>[\w\d]+)\s+(?P<chan>[\w\d]+)\s+(?P<date>[\w\d-]+\s[\w\d:]+)\s+(?P<I>[\.\d\-e]+)',l.strip())
        if not r:
            print 'Bad line: %s' % l.strip()
            continue

        if det!=r.group('det') or chan!=r.group('chan'):
            continue

        date = ROOT.TDatime(r.group('date'))

        if date.Convert()<t1.Convert() or date.Convert()>t2.Convert():
            continue
        
        I = float(r.group('I'))

        #print r.group('date'),date.AsSQLString(),r.group('I')
        gr.SetPoint(gr.GetN(),date.Convert(),I)
        h.Fill(I)

    return (gr,h)


def make_h(detector,channel,t1,t2):

    cmd = "mysql -B -s -e \"select current from dcs.current where detector=\'%s\' AND channel=\'%s\' AND time>\'%s\' AND time<\'%s\'\"" % (detector,channel,t1.AsSQLString(),t2.AsSQLString())
    #print cmd
    lines = os.popen(cmd).readlines()

    h = ROOT.TH1F('h_I','Current distr in [%s,%s]' % (t1.AsString(),t2.AsString()),200,0,2e-6)
    for l in lines:
        h.SetBinContent(float(l.strip()))
    return h

# \arg \c t1   SQL time
# \arg \c t2   SQL time
def get_current(t1,t2):
    cmd = "mysql -h na58pc052 -B -s -e \"select detector,channel,time,current from dcs.current where time>=\'%s\' AND time<=\'%s\'\"" % (t1,t2)
    for l in os.popen(cmd).readlines():
        r = re.match('(?P<det>[\w\d]+)\s+(?P<chan>[\w\d]+)\s+(?P<date>[\w\d-]+\s+[\w\d:]+)\s+(?P<I>[\.\d\-e]+)',l.strip())
        if not r:
            print 'Bad line: %s' % l.strip()
            continue

        # Retrun the result!
        
        res = {}
        res['detector']   = r.group('det')
        res['channel']    = r.group('chan')
        res['date']       = r.group('date')
        res['current']    = float(r.group('I'))
        
        yield res

"""
import ROOT,current
h=current.make_h('ST03X1','6mm_0_PH',ROOT.TDatime('2004-06-01 00:00:00'),ROOT.TDatime('2004-07-01 00:00:00'))
g=current.I_graph('ST03X1','6mm_0_PH','current.txt.bz2',ROOT.TDatime('2004-06-01 00:00:00'),ROOT.TDatime('2004-07-01 00:00:00'))
"""
if __name__=='__main__':
    detector  = 'ST03X1'
    channel   = '6mm_0_PH'
    file_name = 'current.txt.bz2'
    t1        = ROOT.TDatime('2004-08-01 00:00:00')
    t2        = ROOT.TDatime('2004-09-01 00:00:00')
    
    #cmd = "mysql -B -s -e \"select * from dcs.current where detector=\'%s\' AND channel=\'%s\' AND time>\'%s\' AND time<\'%s\'\"" % (detector,channel,t1.AsSQLString(),t2.AsSQLString())
    #print cmd
    #lines = os.popen(cmd).readlines()
    
    #I_graph(detector,channel,file_name,t1,t2)


#ST03X1
#{'10mm_2': 0, '10mm_3': 0, '6mm_7_Jura': 0, '6mm_1_2': 0, '10mm_4': 0, '6mm_0_PH': 0, '6mm_3': 0, '6mm_5': 0, '6mm_6': 0, '10mm_1': 0, '6mm_4_Ph': 0, '10mm_Jura': 0, '6mm_1_Saleve': 0, 'notused2': 0, '10mm_Saleve': 0}
#ST03Y1
#{'10mm_2': 0, '10mm_1': 0, '6mm_1_BOTTOM': 0, '6mm_0_PH': 0, '6mm_2': 0, '6mm_3_PH': 0, 'notused2': 0, '6mm_5': 0, '6mm_4_Ph': 0, '10mm_TOP': 0, '6mm_6_TOP': 0, 'notused4': 0, 'notused5': 0, 'notused1': 0, '10mm_BOTTOM': 0, 'notused3': 0}

    channels = [{},{}]  # channels for X,Y
    channels[0]['10mm_Saleve']  =  1
    channels[0]['10mm_1']       =  2
    channels[0]['10mm_2']       =  3
    channels[0]['6mm_1_Saleve'] =  4
    channels[0]['6mm_1_2']      =  5
    channels[0]['6mm_3']        =  6
    channels[0]['6mm_4_Ph']     =  7
    channels[0]['6mm_5']        =  8
    channels[0]['6mm_6']        =  9
    channels[0]['6mm_7_Jura']   = 10
    channels[0]['10mm_3']       = 11
    channels[0]['10mm_4']       = 12
    channels[0]['10mm_Jura']    = 13
    channels[0]['6mm_0_PH']     = 16

    channels[1]['10mm_BOTTOM']  =  1
    channels[1]['10mm_1']       =  2
    channels[1]['6mm_1_BOTTOM'] =  3
    channels[1]['6mm_2']        =  4
    channels[1]['6mm_3_PH']     =  5
    channels[1]['6mm_4_Ph']     =  6
    channels[1]['6mm_5']        =  7
    channels[1]['6mm_6_TOP']    =  8
    channels[1]['10mm_2']       =  9
    channels[1]['10mm_TOP']     = 10
    channels[1]['6mm_0_PH']     = 13
    
    f = ROOT.TFile('I.root','RECREATE','',9)
    
    detectors = ['ST03X1','ST03X2','ST03Y1']
    hists = {}
    h_IT = {}
    for h in detectors:
        hists[h] = ROOT.TH1F(h,h,16,0,16)
        for channel in channels[h[4]=='Y']:
            name = '%s_%s' % (h,channel)
            hists[name] = ROOT.TH1F(name,name,1000,0,1e-5)
            name = 'h_IT_%s_%s' % (h,channel)
            h_IT[name] = ROOT.TH2F(name,'Current versus time',200,t1.Convert(),t2.Convert(),100,0,1e-5)
            h_IT[name].GetXaxis().SetTitle("Time")
            h_IT[name].GetXaxis().SetTimeDisplay(1)
            h_IT[name].GetXaxis().SetTimeFormat("#splitline{%d/%m}{%H:%M}")
            h_IT[name].GetXaxis().SetLabelSize(0.03)
            h_IT[name].GetXaxis().SetLabelOffset(0.02)
    
    for I in get_current(t1.AsSQLString(),t2.AsSQLString()):
        try:
            ### h = hists[I['detector']]
            
            h_name = '%s_%s' % (I['detector'],I['channel'])
            
            h = hists[h_name]
            #print h_name

            #the_channels = channels[I['detector'][4]=='Y']
            #print the_channels,I['channel']
            #bin = the_channels[I['channel']]

            #print bin
            h.Fill(I['current'])
            
            h_name = 'h_IT_%s_%s' % (I['detector'],I['channel'])
            h_IT[h_name].Fill(ROOT.TDatime(I['date']).Convert(),I['current'])

        except:
            pass
    
    for h_name,h in hists.items():
        detector = h_name[:6]
        channel  = h_name[7:]
        print channel
        if not channel:
            continue

        c = ROOT.TCanvas()
        h.Draw()
        peak = h.GetXaxis().GetBinCenter(h.GetMaximumBin())
        #print h_name,peak

        the_channels = channels[detector[4]=='Y']
        bin = the_channels[channel]
        hists[detector].SetBinContent(bin,peak)
        
        c.Print(h.GetName()+'.ps','ps')


    for h in detectors:
        c = ROOT.TCanvas()
        hists[h].Draw()
        c.Print(hists[h].GetName()+'.ps','ps')

    f.Write()
