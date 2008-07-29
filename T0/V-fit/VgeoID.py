import sys,os,re

try:
    import optparse
except:
    sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
    import optparse

##  The class which holds a card T0 fit results
#
class Card:
    ##  Constructor
    #
    #   @arg See the class attribute descriptions.
    #
    def __init__(self,det,geoID,chf,chl,pos):

        ##  Detector name
        self.detector = det
        
        ##  The card geoID number
        self.geoID    = geoID
        
        ##  First detector channel of the card.
        self.chf      = chf
        
        ##  Last detector channel of the card.
        self.chl      = chl
        
        ##  Card position.
        self.pos      = pos

    def channels(self):
        return self.chl-self.chf+1

def read_maps(prog,maps_dir,run):
    
    maps={}   # (detector,geoID) => Card
    
    command='%s --det="ST.*" --maps=%s  --run=%d' % (prog,maps_dir,options.run)

    for l in os.popen(command).readlines():
    
        r = re.match('(?P<det>\w+.)\s.*\swire=(?P<wire>\d+)\s.*\spos=(?P<pos>-*\d+)\s+.*geoID/port=(?P<geoID>\d+).*mode=(?P<mode>\d).*',l.strip())
        if r==None:
            continue

        det   = r.group('det')
        wire  = int(r.group('wire'))
        pos   = int(r.group('pos'))
        geoID = int(r.group('geoID'))
        mode  = int(r.group('mode'))

        if mode==0:
            continue    # Skip debug mode

        key = (det,geoID)
        card = Card(det,geoID,wire,wire,pos)
        
        if not maps.has_key(key):
            maps[key] = card
        else:
            c = maps[key]
            if c.chf>card.chf:
                c.chf=card.chf
            if c.chl<card.chl:
                c.chl=card.chl

    return maps

# This function uses the global options, see the file top.
def t0_geoID(card,Vopt):
    if card.detector[3] in '56':
        mcuts = Vopt.cuts_ST0506
    else:
        mcuts = Vopt.cuts
    if mcuts!='':
        mcuts += '&&'
    mcuts += 'geo==%d' % card.geoID
    if card.detector[-1]=='b':
        RT = Vopt.RT6mm
        dt = 100
    else:
        RT = Vopt.RT10mm
        dt = 130
    command = '%s --det=%s --svel=28 --data=%s/%s.root --pos=%d,0 --chf=%d --chl=%d --chg=%d ' \
              '--cuts="%s" --RT="%s" --comment="%s" %s ' \
              '--dt=%g --V-fit-max-points=%g --V-center-rm=%g --V-leg-dist=%g %s %s' \
               % (Vopt.V,card.detector,Vopt.data,card.detector,card.pos,card.chf,card.chf,card.channels(),
                  mcuts,RT,str('geoID %s' % card.geoID),Vopt.t0_ref,dt,
                 Vopt.V_fit_max_points,Vopt.V_center_rm,Vopt.V_leg_dist,
                 Vopt.db,Vopt.session)
    command += '; mv %s.root T0_%s_geoID%d.root\n\n' % (Vopt.session,card.detector,card.geoID)
    
    print command,'\n'

if __name__ == '__main__':

    parser = optparse.OptionParser()
    parser.add_option('', '--RT6mm',dest='RT6mm',action='store',
                      default='RT-Grid 0:0 0.033:10.4 0.066:15.2 0.165:27.4 0.264:40.4 0.297:49.3 0.33:58.4',
                      help='RT for 6mm straws [default: "%default"]', type='string',metavar='STRING')
    parser.add_option('', '--RT10mm',dest='RT10mm',action='store',
                      default='RT-Grid 0:0 0.048:13.0 0.096:19.8 0.240:36.4 0.384:57.0 0.432:65.9 0.48:83.4',
                      help='RT for 10mm straws [default: "%default"]', type='string',metavar='STRING')
    parser.add_option('', '--data',dest='data',default='',action='store',
                      help='Directory with ROOT ntuple files. [default: %default]', type='string',metavar='PATH')
    parser.add_option('', '--V-prog',dest='V',action='store',
                      default='V',
                      help='Path to a V-fit program [default: %default]', type='string',metavar='PATH')
    parser.add_option('', '--maps-prog',dest='maps',action='store',
                      default='/afs/cern.ch/compass/detector/straw/DaqDataDecoding/SLC4/bin/maps',
                      help='Path to a maps program [default: %default]', type='string',metavar='PATH')
    parser.add_option('', '--maps-dir',dest='maps_dir',action='store',
                      default='/afs/cern.ch/compass/detector/maps/',
                      help='Path to a mapping directory [default: %default]', type='string',metavar='PATH')
    parser.add_option('', '--cuts',dest='cuts',action='store',
                      default='(tr_Xi2/tr_nh)<3&&tr_z1<1450&&(abs(tr_t)<4)&&tr_q!=0',
                      help='Cuts for tracks [default: "%default"]', type='string',metavar='CUTS')
    parser.add_option('', '--cuts_ST0506',dest='cuts_ST0506',action='store',
                      default='(tr_Xi2/tr_nh)<3&&tr_z2>30000&&(abs(tr_t)<4)&&tr_q!=0',
                      help='Cuts for tracks at positions of ST05 and ST06 [default: "%default"]', type='string',metavar='CUTS')
    parser.add_option('', '--V-fit-max-points',dest='V_fit_max_points',action='store',
                      default=3, help='[default: %default]', type='float',metavar='NUMBER')
    parser.add_option('', '--V-center-rm',dest='V_center_rm',action='store',
                      default=0.02, help='[default: %default]', type='float',metavar='NUMBER')
    parser.add_option('', '--V-leg-dist',dest='V_leg_dist',action='store',
                      default=0.1, help='[default: %default]', type='float',metavar='NUMBER')
    parser.add_option('', '--t0-ref',dest='t0_ref',action='store',
                      default='/afs/cern.ch/user/z/zvyagin/Detectors.git/programs/V-fit/coral-t0-mod.txt',
                      help='Starting values for T0 fit [default: %default]', type='string',metavar='PATH')
    parser.add_option('', '--dbhost',dest='db_host',action='store',default='na58pc052.cern.ch',
                      help='DB host name [default: %default]', type='string',metavar='NAME')
    parser.add_option('', '--dbuser',dest='db_user',action='store',default='',
                      help='DB user name. You should provide it to write to the DB.', type='string',metavar='NAME')
    parser.add_option('', '--dbpassword',dest='db_password',action='store',default='',
                      help='DB user password.', type='string',metavar='NAME')


    parser.usage = '%prog [options] <run> <session-name>'

    (options, args) = parser.parse_args()
    
    if len(args)!=2:
        parser.print_help()
        sys.exit(1)
    
    options.run     = int(args[0])
    options.session = args[1]
    
    db_opt = ''
    options.db = ''
    if options.db_host:
        db_opt += ' -h ' + options.db_host
        options.db += ' --dbhost=%s' % options.db_host
    if options.db_user:
        db_opt += ' -u ' + options.db_user
        options.db += ' --dbuser=%s' % options.db_user
    if options.db_password:
        db_opt += ' -p' + options.db_password
        options.db += ' --dbpassword=%s' % options.db_password
    
    if os.system('mysql %s -e "USE STDC;" > /dev/null' % db_opt ):
        print 'DB "STDC" on "%s" is not accessable!' % db_opt
        sys.exit(2)
    
    if 0==os.system('mysql -h %s -e "describe STDC.%s;" > /dev/null 2>&1' % (options.db_host,options.session) ):
        print 'Table STDC.%s already exists!! Remove it from the DB, or use another session name!' % options.session
        sys.exit(3)

    if options.t0_ref:
        options.t0_ref = ' --t0-ref=%s' % options.t0_ref

    # ===========================

    straw_maps = read_maps(options.maps,options.maps_dir,options.run)
    
    for m in straw_maps.values():
        #print m.detector,'geoID=',m.geoID,'pos=',m.pos,'chf=',m.chf,'chl=',m.chl,'channels=',m.channels()
        t0_geoID(m,options)

