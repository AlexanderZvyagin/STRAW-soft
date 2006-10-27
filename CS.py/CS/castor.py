from __future__ import generators
import string,re,os,sys,time

def path_stat(path):
    stat={}
    for l in os.popen('rfstat %s 2>&1' % path).readlines():
        r = re.match('(?P<key>.*?):(?P<data>.*)',l.strip())
        if not r:
            raise 'bad format:',l
        stat[r.group('key').strip()] = r.group('data').strip()
        
    return stat

def copy(src,dst,move=False):
    print src,'==>>',dst
    if path_stat(src)['protection'][0]=='-':
        res = os.system('rfcp %s %s' % (src,dst) )
        if res==0 and move:
            os.system('rfrm %s' % src)

    elif path_stat(src)['protection'][0]=='d':
        if path_stat(dst)['protection'][0]=='-':
            raise "copy: I don't want to copy directory to a file %s" % dst
        dst = dst+'/'+os.path.split(src)[1]
        res = os.system('rfmkdir %s' % dst )
        if res:
            # Error!
            return res

        for f in castor_files(src):
            name = os.path.split(f)[1]
            if name in ['.','..']:
                continue
            res = copy(f,dst+'/'+name,move)
            if res:
                return res

        if move:
            os.system('rfrm -r %s' % src)

        return 0

##  Get list of files with given pattern (regular expression)
def castor_files(directory,pattern=None):
    if os.system('rfstat %s > /dev/null 2>&1' % directory):
        return
    for f in os.popen('rfdir ' + directory):
        name=f.strip().split()[-1]
        if pattern:
            r = re.match(pattern,name)
            if not r:  continue
        yield os.path.join(directory,name)

##  Dictionary with the CDR file attributes
#
def file_cdr(d,f):
    q = {}

    if d=='':
        q['name']   = f
        q['fname']  = f
    else:
        if d[-1]!='/':
            d += '/'
        q['name']  = string.strip(f.split(' ')[-1])
        q['fname']  = d+q['name']
    res = re.match(r'.*cdr(?P<cdr>\d\d)\d+-(?P<run>\d+).*',q['name'])
    q['run']    = int(res.group('run'))
    q['pccoeb'] = int(res.group('cdr'))
    q['feor']   = None!=re.match('.*00\-\d+.*',q['name'])

    # Now get the file size
    size = os.popen('rfstat %s | grep Size' % q['fname']).readlines()[0]
    size = size.split()[-1]
    try:
        q['size'] = int(size)
    except:
        q['size'] = 0
    
    return q

def rfdir_cdr_files(directory,printout=0):

    if os.system('rfstat %s > /dev/null 2>&1' % directory):
        return

    for p in os.popen('rfdir ' + directory):
        if printout>2:
            print directory,p
        try:
            yield file_cdr(directory,p)
        except KeyboardInterrupt:
            raise
        except:
            if printout>1:
                print sys.exc_value

## Generator of CDR files in 2002 castor directory
def cdr_files_2002(printout=0):
    dirs='/castor/cern.ch/compass/data/2002/raw_migration/'

    for i in range(1,4):
        for c in 'ABCDEFGHI':
            period='P%d%c' % (i,c)

            for j in range (0,9):
                d = dirs+period+('/%d0/' % j)
                #print 'The database has %d runs' % len(self.__run_db)
                if printout>0:
                    print 'Scanning directory',d
                for f in rfdir_cdr_files(d,printout):
                    f['year']=2002
                    f['period']=period
                    yield f

## Generator of CDR files in 2003 castor directory
def cdr_files_2003(printout=0):
    dirs='/castor/cern.ch/compass/data/2003/raw/'
    for i in range(1,4):
        for c in 'ABCDEFGHI':
            period='P%d%c' % (i,c)

            d = dirs+period
            if printout>0:
                print 'Scanning directory',d
            for f in rfdir_cdr_files(d,printout):
                f['year']=2003
                f['period']=period
                yield f

# Generator of CDR files in 2004 castor directory
def cdr_files_2004(printout=0):
    dirs='/castor/cern.ch/compass/data/2004/raw/'
    lst=[]
    for i in range(0,55):
        lst.append('T%2.2d'%i)
    for i in range(21,55):
        lst.append('W%2.2d'%i)

    for period in lst:
        d = dirs+period
        if printout>0:
            print 'Scanning directory',d
        for f in rfdir_cdr_files(d,printout):
            f['year']=2004
            f['period']=period
            yield f

# Generator of CDR files in 2006 castor directory
def cdr_files_2006(printout=0):
    dirs='/castor/cern.ch/compass/data/2006/raw/'
    lst=[]
    for i in range(0,55):
        lst.append('T%2.2d'%i)
        lst.append('W%2.2d'%i)
    lst.append('test')

    # This allows to add new files first.
    lst.reverse()

    for period in lst:
        d = dirs+period
        if printout>0:
            print 'Scanning directory',d
        for f in rfdir_cdr_files(d,printout):
            f['year']=2006
            f['period']=period
            yield f

# Generator of CDR files from castor
def cdr_files(years=[],printout=0):
    if not years:
        # Set default value
        years=(2002,2003,2004,2006,2007,2008,2009,2010)
    if 2002 in years:
        for f in cdr_files_2002():
            yield f
    if 2003 in years:
        for f in cdr_files_2003():
            yield f
    if 2004 in years:
        for f in cdr_files_2004():
            yield f
    if 2006 in years:
        for f in cdr_files_2006():
            yield f

class mDST:
    def __init__(self,name):
        self.name = name
        stat = path_stat(name)
        self.time = time.strptime(stat['Last modify'])
        self.size = int(stat['Size (bytes)'])
    
        self.short_name = os.path.split(name)[1]
        r = re.match('mDST-(?P<run>\d+)-(?P<slot>\d+)-(?P<version>\d+)\.root(\.(?P<n>\d+)){0,1}',self.short_name)
        if not r:
            raise 'Bad name:',self.short_name
        self.run     = int(r.group('run'))
        self.slot    = int(r.group('slot'))
        self.version = int(r.group('version'))
        self.n       = r.group('n')
        if self.n:
            self.n = int(self.n)
        else:
            self.n = 0
    
    def __str__(self):
        s = ''
        s += 'Name ............ %s\n' % self.name
        s += 'Time ............ %s\n' % time.asctime(self.time)
        s += 'Size ............ %d\n' % self.size
        s += 'Slot ............ %d\n' % self.slot
        s += 'Phast version ... %d\n' % self.version
        return s

class SlotVer:
    def __init__(self,run,slot_ver):
        self.run      = run
        self.slot_ver = slot_ver
        self.files    = []
        self.time     = None    # (min,max)

    def __iadd__(self,other):
        assert self.run == other.run
        assert self.slot_ver == (other.slot,other.version)
        self.files.append(other)
        if not self.time:
            self.time = [other.time,other.time]
        else:
            if self.time[0]>other.time:
                self.time[0]=other.time
            if self.time[1]<other.time:
                self.time[1]=other.time
        return self
    
    def __str__(self):
        s  = ''
        s += 'Run %d   slot %d   PHAST version %d\n' % (self.run,self.slot_ver[0],self.slot_ver[1])
        if self.time:
            s += 'Time range:  %s <----> %s\n' % (time.asctime(self.time[0]),time.asctime(self.time[1]))
        for f in self.files:
            s += f.name + '\n'
        return s

# Generator of CDR files in 2004 castor directory
def mDST_files(year,printout=0):

    try:
        import term
        terminal = term.TerminalController()
    except:
        terminal = None

    run_sv = {} # run-> slot,version -> SlotVer
    
    info = {}   # run -> (slot,phast) -> (time min,time max)

    mDSTs = {}  # run -> list of mDST files

    # It is used for tests
    n = 0
    n_max = 10000000000

    dirs='/castor/cern.ch/compass/data/%d/oracle_dst' % year

    periods=[]
    for period in castor_files(dirs):
        periods.append(period)

    if terminal:
        progress = term.ProgressBar(terminal, '%s: %d periods' % (dirs,len(periods)))

    for i in range(len(periods)):
        period = periods[i]

        if terminal:
            progress.update(float(i)/len(periods), 'Scanning %s' % os.path.split(period)[1])

        for f in castor_files(period+'/mDST'):

            try:
                q = mDST(f)
            except:
                continue

            key = (q.slot,q.version)

            if not mDSTs.has_key(q.run):
                mDSTs[q.run] = {}
            assert not mDSTs[q.run].has_key(q.short_name)
            mDSTs[q.run][q.short_name] = q

            if not run_sv.has_key(q.run):
                run_sv[q.run] = {}
            if not run_sv[q.run].has_key(key):
                run_sv[q.run][key] = SlotVer(q.run,(q.slot,q.version))
            run_sv[q.run][key] += q

            if not info.has_key(q.run):
                info[q.run] = {}
            t = time.mktime(q.time)
            if info[q.run].has_key(key):
                t_min,t_max = info[q.run][key]
                if t_min>t:
                    t_min=t
                if t_max<t:
                    t_max=t
                info[q.run][key] = (t_min,t_max)
            else:
                info[q.run][key] = (t,t)

            n += 1
            if n>n_max: break
        if n>n_max: break
    
    if terminal:
        progress.update(1,'Scan is finished.')

    if 0:
        for run,lst in mDSTs.iteritems():
            print 'run',run
            for q in lst.values():
                print q

    #print 'Sort and select the files...'

    mDST_latest = {}  # Run -> SlotVer

    for run,d in run_sv.iteritems():
        for sv,f in d.iteritems():
            if not mDST_latest.has_key(run) or mDST_latest[run].time[1]<f.time[1]:
                mDST_latest[run] = f

    print 'Writing to the DB...'
    for run,sv in mDST_latest.iteritems():
        #os.system('mysql -ucompass -pHMcheops -e "DELETE FROM run.mDST WHERE run=%d"' % run)
        files=[]
        for f in sv.files:
            files.append(f.name)
        os.system('mysql -ucompass -pHMcheops -e "REPLACE INTO run.mDST (run,file,time,size) VALUES(%d,\'%s\',\'%s\',%d)"' % \
                  (run,' '.join(files),time.strftime('%Y-%m-%d %H:%M:%S',sv.time[1]),0))

########################################################################
### The self test
########################################################################

import unittest

class TestCase(unittest.TestCase):
    #def setUp(self):
        #self.top = SE(0,1)
    #    pass

    def test_rfdir(self):
        res = os.system('rfdir /castor/cern.ch/compass/data/2002 > /dev/null')
        self.failUnless(res==0, 'rfdir failed!')

def TheTestSuite():
    return unittest.makeSuite(TestCase,'test')

########################################################################
### The main program
########################################################################

def main():

    import optparse

    commands = ['ls','cp','mv','mDST']

    parser = optparse.OptionParser(version='1.2.1')
    parser.description = 'CASTOR file system utilities'
    parser.usage = 'cs %prog <command> [options]\n' \
                   '  Type "%prog <command>" for more help.\n' \
                   '  List of available commands: ' + ','.join(commands)+'\n'\
                   'Author: Alexander.Zvyagin@cern.ch'

    parser.add_option('', '--pattern',dest='pattern',default=None,
                      help='Pattern (regular expression) for files.', type='string', metavar='reg-expr')

    parser.add_option('', '--sep',dest='sep',default='EndOfLine',
                      help='Files separation symbol', type='string', metavar='symbol')

    #parser.add_option('', '--test',dest='test',default=False,action='store_true',
    #                  help='Run a test suit', type='string')
    
    (options, args) = parser.parse_args()

    #if options.test:
    #    print 'Running the test suit'
    #    return unittest.main()

    if not args:
        parser.print_help()
        return 1

    if options.sep=='EndOfLine':
        options.sep = '\n'

    if args[0]=='ls':
        del args[0]
        if len(args)==0:
            print 'Usage: castor ls [options] <dir> [<dir> ....]'
            return 1
        files=[]
        for d in args:
            for f in castor_files(d,options.pattern):
                files.append(f)
        print options.sep.join(files)
        return 0

    if args[0] in ['cp','mv']:
        move = args[0]=='mv'
        del args[0]
        if len(args)!=2:
            print 'Usage: castor cp <src> <dst>'
            print 'Usage: castor mv <src> <dst>'
            return 1
        copy(args[0],args[1],move)
        return 0

    if args[0]=='mDST':
        del args[0]
        if len(args)<1:
            print 'Usage: castor mDST year1 [year2...]'
            return 1
        for year in args:
            mDST_files(int(year))
        return 0

    print 'Unknwon command: %s' % args[0]
    print 'Possible commands are: %s' % ','.join(commands)
    return 1

if __name__ == '__main__':
    sys.exit(main())
