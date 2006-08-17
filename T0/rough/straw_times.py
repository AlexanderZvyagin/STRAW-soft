from __future__ import generators

import re,os,sys
import db

try:
    import MySQLdb, optparse
except:
    sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
    import MySQLdb, optparse


def main():
    parser = optparse.OptionParser()
    parser.add_option('', '--db-access',dest='dbaccess',default='-hna58pc052.cern.ch -uanonymous',
                      help='DB access options (ex: -hhost -ume -ppass) [default: %default]', type='string',metavar='OPTIONS')
    parser.add_option('', '--maps',dest='maps',default='/afs/cern.ch/compass/detector/maps',
                      help='Decoding maps directory [default: %default]', type='string',metavar='PATH')
    parser.add_option('', '--plugin',dest='plugin',default='./straw_times.so',
                      help='Plugin code [default: %default]', type='string',metavar='PATH')
    parser.add_option('', '--events',dest='events',default=-1,
                      help='Number of events to analyze (per file) [default: %default]', type='int',metavar='NUMBER')

    parser.usage = '%prog [options] <run> <ddd>'

    (options, args) = parser.parse_args()

    if len(args)!=2:
        parser.print_help()
        sys.exit(1)
    
    if options.events>0:
        events = '--events=%d' % options.events
    else:
        events = ''

    for f in db.get_run_files(int(args[0]),True,options.dbaccess):
        os.system('echo %s %s --maps=%s --plugin=%s %s' % (args[1],events,options.maps,options.plugin,f) )

if __name__=='__main__':
    main()
