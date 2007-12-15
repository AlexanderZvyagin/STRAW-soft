import re, sys, os
import optparse
from CS import castor

class Status:
    def __init__(self):
        
        ##  Chunk name, like 'cdr33022-55555'
        self.chunk = None
        
        ##  Number of CORAL FATAL messages
        self.fatal = 0
        
        ##  Number of CORAL ERROR messages
        self.errors = 0
        
        ##  Number of analyzed events
        self.events = 0
        
        ##  Did we have a complete CORAL crash?
        self.crash = None

    def ok(self):
        return self.events>0

    def __str__(self):
        s  = ''
        s += ' ok=%s' % self.ok()
        if self.fatal:
            s += ' FATAL ERROR'
        if self.crash:
            s += ' CRASH!'
        s += ' errors=%d' % self.errors
        return s

class Pattern:
    
    def __init__(self,pattern):
        self.expr = re.compile(pattern)
        self.counter = 0

    # FATAL, on Tue, 19/Sep/2006 12:01:24.853778 (GMT)
    def CORAL_log(p):
        return re.compile('^%s,\s+on\s+' % p)
    CORAL_log = staticmethod(CORAL_log)
    
    def match(self,txt):
        r = self.expr.match(txt)
        if r:
            self.last_accepted_result = r
        return r
    
    def __add__(self,a):
        self.counter += a
        return self.counter 

def coral_log_analyze(f):
    
    patterns={}
    patterns['CORAL fatal']  = Pattern(Pattern.CORAL_log('FATAL'))
    patterns['CORAL error']  = Pattern(Pattern.CORAL_log('ERROR'))
    patterns['CORAL events'] = Pattern('^\| The number of events \(def\)\s+(?P<events>\d+)\s+.*\|')
    patterns['VKF'] = Pattern('.*Vertex Kalman Filter statistics.*')
    
    line_VKF = 0
    
    line = 0

    status = Status()
    
    if f[:8]=='/castor/':
        program_cat = 'rfcat'
    else:
        program_cat = 'cat'
    
    for ll in os.popen('%s %s' % (program_cat,f)).readlines():
        line += 1
        l = ll.strip()
        for k,p in patterns.iteritems():
            if not p.match(l):
                continue
            p += 1

            if k=='VKF':
                line_VKF = line

            if k=='CORAL events' and (line-1)==line_VKF:
                status.events = int(patterns['CORAL events'].last_accepted_result.group('events'))

    status.fatal  = patterns['CORAL fatal'].counter
    status.errors = patterns['CORAL error'].counter
        
    return status

def main():
    parser = optparse.OptionParser(version='1.0.2')

    parser.add_option('', '--noterm',dest='noterm',default=False,action='store_true',
                      help='Do not use fancy output (terminal properties).')
    parser.add_option('', '--print-good-logs',dest='print_good_logs',default=False,action='store_true',
                      help='Print good log file names.')

    parser.usage = 'cs %prog <options> [dir-with-logs]\n'\
                   'Author: Alexander.Zvyagin@cern.ch'
    parser.description = 'Analyze CORAL log files.'

    (options, args) = parser.parse_args()

    if len(args)!=1:
        parser.print_help()
        return 1

    try:
        if options.noterm:
            raise ''
        import term
        terminal = term.TerminalController()
    except:
        terminal = None
        #no_colors()


    files = []
    run = 0
    for f in castor.castor_files(args[0],'cdr.*\.log'):
        if not files:
            # Get run number from file name
            r=re.search('.*cdr\d+-(?P<run>\d+).*',f)
            if not r:
                print 'Bad file name:',f
                return 1
            run = int(r.group('run'))
        files.append(f)

    if terminal:
        progress = term.ProgressBar(terminal, 'Analysing %d CORAL log files of the run %d' % (len(files),run))
    else:
        print 'There are %d CORAL log files to analyze.' % len(files)

    logs_ok  = []
    logs_bad = []

    i=0
    n_files = 0.001+len(files)
    for i in range(len(files)):
        
        f = files[i]

        if terminal:
            progress.update(i/n_files, 'Analysing file %s' % f)
        else:
            if i%10==0:
                print ' %d%%' % int((i*100)/n_files),
                sys.stdout.flush()
            
        
        status = coral_log_analyze(f)
        if status.ok():
            logs_ok.append(f)
        else:
            logs_bad.append(f)

    if terminal:
        progress.update((i+1)/n_files, 'FINISHED')
    else:
        print

    if options.print_good_logs:
        print 'List of good log files:'
        for f in logs_ok:
            print f

    ok  = len(logs_ok)
    bad = len(logs_bad)
    print 'Out of %d jobs %d(%d%%) finished successfully' % (ok+bad,ok,(ok*100)/(ok+bad+0.001))

if __name__=='__main__':
    sys.exit(main())
