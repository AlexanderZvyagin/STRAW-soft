import re,os,sys,optparse
from CS import db,castor

def main():
    parser = optparse.OptionParser()
    parser.add_option('', '--phast',dest='phast',default='phast',
                      help='Path to PHAST program', metavar='PATH')
    parser.add_option('', '--phast-opts',dest='phast_opts',default='-D1 -m',
                      help='PHAST options', type='string', metavar='OPTS')

    parser.usage = '%prog <options> [mDST dir]'
    parser.description = 'Output file will be named (assuming it was a run 55555) mDST-55555.root'

    (options, args) = parser.parse_args()

    if len(args)!=1:
        parser.print_help()
        return 1

    files = []
    for f in castor.castor_files(args[0],'mDST.*\.root'):
        if not files:
            # Get run number from file name
            r=re.search('.*cdr\d+-(?P<run>\d+).*',f)
            if not r:
                print 'Bad file name:',f
                return 1
            run = int(r.group('run'))
        files.append(f)

    command  = options.phast + ' ' + options.phast_opts
    command += ' -o %s/mDST-%d.root -h %s/hist-%d.root ' % (args[0],run,args[0],run)
    command += ' '.join(files)

    return os.system(command)

if __name__ == '__main__':
    sys.exit(main())
