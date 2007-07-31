import os,re,sys

# This function uses the global options, see the file top.
def t0_geoID(Vopt,st,geoID,chf,pos):
    if st[3] in '56':
        mcuts = Vopt.cuts_ST0506
    else:
        mcuts = Vopt.cuts
    if mcuts!='':
        mcuts += '&&'
    mcuts += 'geo==%d' % geoID
    if st[-1]=='b':
        RT = Vopt.RT6mm
        dt = 100
        if st[4] in 'XUV' and (chf == 64 or chf == 127):
            chg = 31
        else :
            chg = 32
    else:
        RT = Vopt.RT10mm
        dt = 130
        chg = 32
    command = '%s --det=%s --svel=28 --data=%s/%s.root --pos=%d,0 --chf=%d --chl=%d --chg=%d ' \
              '--cuts="%s" --RT="%s" --comment="%s" %s ' \
              '--dt=%g --V-fit-max-points=%g --V-center-rm=%g --V-leg-dist=%g %s %s' \
               % (Vopt.V,st,Vopt.data,st,pos,chf,chf,chg
                  mcuts,RT,str('geoID %s' % geoID),Vopt.t0_ref,dt,
                 Vopt.V_fit_max_points,Vopt.V_center_rm,Vopt.V_leg_dist,
                 Vopt.db,Vopt.session)
    command += '; mv %s.root T0_%s_geoID%d.root\n\n' % (Vopt.session,st,geoID)

    options.output.write(command)

################################################################################

try:
    import optparse
except:
    sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
    import optparse

if __name__ == '__main__':

    parser = optparse.OptionParser()
    parser.add_option('', '--RT6mm',dest='RT6mm',action='store',
                      default='RT-Grid 0:0 0.033:10.4 0.066:15.2 0.165:27.4 0.264:40.4 0.297:49.3 0.33:58.4',
                      help='RT for 6mm straws [default: "%default"]', type='string',metavar='STRING')
    parser.add_option('', '--RT10mm',dest='RT10mm',action='store',
                      default='RT-Grid 0:0 0.048:13.0 0.096:19.8 0.240:36.4 0.384:57.0 0.432:65.9 0.48:83.4',
                      help='RT for 10mm straws [default: "%default"]', type='string',metavar='STRING')
    parser.add_option('', '--data',dest='data',default='/home/data/37059-align/',action='store',
                      help='Directory with ROOT ntuple files. [default: %default]', type='string',metavar='PATH')
    parser.add_option('', '--V-prog',dest='V',action='store',
                      default='/afs/cern.ch/user/z/zvyagin/Detectors.git/programs/V-fit/V',
                      help='Path to a V-fit program [default: %default]', type='string',metavar='PATH')
    parser.add_option('', '--maps-prog',dest='maps',action='store',
                      default='/afs/cern.ch/user/z/zvyagin/DDD.git/examples/how-to/maps',
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
    parser.add_option('-o', '--output',dest='output',action='store',default='',
                      help='Output file name [default: "%default"]', type='string',metavar='PATH')

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
        db_opt += ' -p ' + options.db_password
        options.db += ' --dbpassword=%s' % options.db_password
    
    if os.system('mysql %s -e "USE STDC;" > /dev/null' % db_opt ):
        print 'DB "STDC" on "%s" is not accessable!' % db_opt
        sys.exit(2)
    
    if 0==os.system('mysql -h %s -e "describe STDC.%s;" > /dev/null 2>&1' % (options.db_host,options.session) ):
        print 'Table STDC.%s already exists!! Remove it from the DB, or use another session name!' % options.session
        sys.exit(3)

    if options.t0_ref:
        options.t0_ref = ' --t0-ref=%s' % options.t0_ref

    if options.output:
        name = options.output
        options.output = file(options.output,'w')
        os.system('chmod +x %s' % name)
    else:
        options.output = sys.stdout

    det_geoIDs = {}     # GeoIDs per detector
    geoID_channel_first = {} 

    command='%s --run=%d --det="ST.*" --maps=%s' % (options.maps,options.run,options.maps_dir)
    for out in os.popen(command).readlines():
        if out.strip()=='':
            continue

        r = re.match('(?P<det>\w+.)\s.*\swire=(?P<wire>\d+)\s.*\spos=(?P<pos>-*\d+)\s+.*geoID/port=(?P<geoID>\d+).*mode=(?P<mode>\d).*',out)
        if r==None:
            continue

        det   = r.group('det')
        wire  = int(r.group('wire'))
        pos   = int(r.group('pos'))
        geoID = int(r.group('geoID'))
        mode  = int(r.group('mode'))

        if mode==0:
            continue    # Skip debug mode
        
        #print det,geoID,wire,pos
        
        if not det_geoIDs.has_key(det):
            det_geoIDs[det] = []
        if det_geoIDs[det].count(geoID)==0:
            det_geoIDs[det].append(geoID)
            geoID_channel_first[geoID]=(1111,-1)  # channel,position

        if geoID_channel_first[geoID][0]>wire:
            geoID_channel_first[geoID] = (wire,pos)

    for st,geoIDs in det_geoIDs.items():
        for geoID in geoIDs:
            t0_geoID(options,st,geoID,geoID_channel_first[geoID][0],geoID_channel_first[geoID][1])
