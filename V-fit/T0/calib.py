import os,re,sys
from dT_card import Card
from det_card import straw_full_names

run = 37059
__map_prog__ = '/afs/cern.ch/user/z/zvyagin/DDD.git/examples/how-to/maps --run=%d --maps=/afs/cern.ch/compass/detector/maps/2004.xml' % run

def read_T0_from_latex(file_name='cards.tex'):
    card_T0 = {}
    for line in file(file_name).readlines():
        #print line.strip()
        r = re.match('.*soft\}\{\$\<{0,1}\s{0,1}(?P<T0>.*[\.\d])\>{0,1}\s{0,1}\$\}.*(?P<det>ST....)\s+(?P<card>\d+)',line.strip())
        if not r:
            continue

        card = int(r.group('card'))
        assert not card_T0.has_key(card)
        card_T0[card] = float(r.group('T0'))
        #print r.group('det'), r.group('card'), r.group('T0') 
        
    return card_T0

class WP:
    def __init__(self,wire,pos):
        self.wire = wire
        self.pos  = pos
    def __str__(self):
        return str(self.wire)+' '+str(self.pos)
    #def __cmp__(self,other):
    #    if self.wire!=other.wire:
    #        return self.wire-other.wire
    #    return self.pos-other.pos

def read_map_file(detector,channels):

    prog = '%s --det=%s --chan=-1' % (__map_prog__,detector)

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

        wp = (wire,pos)  # Unique index

        assert not channels.has_key(wp)
    
        channels[wp] = Card(detector,geoID,wire,wire,pos,0)

def card_cmp(a,b):
    return a.chf-b.chf

def write_calib_file(detector,channels):
    file_name = detector+'-T0.calib'
    print 'Writing ',file_name
    f = file(file_name,'w')
    f.write('# Channel  Position  T0\n')
    
    #for key,card in channels.items():
    #    f.write('%3d %2d %6.2f\n' % (key[0],key[1],card.T0) )

    cards = {} # card -> channels
    card_chf = {} # First channel of a card
    chf_card = {} # chf => list of cards
    
    for ch in channels.values():
        if not cards.has_key(ch.card):
            cards[ch.card]    = []
            card_chf[ch.card] = 10000
        cards[ch.card] . append(ch)
        if card_chf[ch.card]>ch.chf:
            card_chf[ch.card]=ch.chf
    
    #print 'card_chf\n', card_chf

    for card,chf in card_chf.items():
        if not chf_card.has_key(chf):
            chf_card[chf] = []
        chf_card[chf].append(card)

    #print 'chf_card\n', chf_card
    
    # Loop over first channel numbers of a card
    chf_lst = chf_card.keys()
    chf_lst.sort()
    for chf in chf_lst:
        for card in chf_card[chf]:
            f.write('# Card %d\n' % card)
            ls = cards[card]
            ls.sort(card_cmp)
            for ch in ls:
                f.write('%3d %2d %6.2f\n' % (ch.chf,ch.pos,ch.T0) )

## Compare T0 with rough T0 calculations.
def read_rough_t0(fname,t0_V):
    import math
    from ROOT import gROOT,TH1F,TCanvas
    gROOT.SetBatch()
    c = TCanvas('T0_rough_cmp','T0_rough_cmp')
    h = TH1F('h_t0_rough_cmp','T0(rough)-T0(V)',100,-5,5)
    f = file(fname)
    for l in f.readlines():
        card,det,t0 = l.strip().split()
        card = int(card)
        t0 = float(t0)
        h.Fill( t0 - t0_V[card] )
        #if math.fabs(t0 - t0_V[card])>3:
        #    print 'card %d  diff %g' % (card,math.fabs(t0 - t0_V[card]))
    h.Draw()
    c.Print('','.gif')

if __name__=='__main__':

    db_table = 'STDC.T0_card'
    card_T0 = read_T0_from_latex()

    read_rough_t0('T0-rough.txt',card_T0)

    for detector in straw_full_names(2004):
        channels={}
        read_map_file(detector,channels)

        for card in channels.values():
            card.T0 = card_T0[card.card]
            os.system('mysql -e "REPLACE INTO %s (run,detector,card,T0) VALUES(%d,\'%s\',%d,%f)"' % (db_table,run,card.detector,card.card,card.T0))

        write_calib_file(detector,channels)
