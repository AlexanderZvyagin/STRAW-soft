import sys,re,optparse
try:
    from colors import *
except:
    from CS.colors import *

def get_html_page(page):
    import httplib
    
    r = re.match('http://(?P<host>[\w.]+)(?P<page>/.*)',page)
    if not r:
        raise 'bad page name:',page
    conn = httplib.HTTPConnection(r.group('host'))
    conn.request("GET", r.group('page'))
    r1 = conn.getresponse()
    #print r1.status, r1.reason
    return r1.read()

##  @brief Period properties
##
class DstPeriod:
    ##  @brief Constructor
    ##
    def __init__(self,full_name,mode,slot):

        ##  Period name
        self.full_name = full_name
        
        ##  Data taking mode: 'L' or 'T'
        self.__mode = mode

        ##  Slot number in the DST production
        self.__slot = slot

    def mode(self):
       return self.__mode

    def slot(self):
        return self.__slot

    def name(self):
        return self.full_name[2:]
        
    def year(self):
        return 2000+int(self.full_name[:2])
        
    def __str__(self):
        return '%s year=%d mode=%s slot=%d' % (self.name(),self.year(),self.mode(),self.slot())

    def pprint(self):
        print self

def add_period(period,d):
    d[period.full_name] = period

##  @brief @return regular expression for an html table cell
def htc(name):
    return '\s*\<td\>\s*(?P<%s>.*?)\s*\</td\>\s*' % name

def read_periods_from_page(page):

    periods={}

    for l in page.split('\n'):
        #r = re.match('\<tr\>\s*\<td\>\s*(?P<period>\d\d\w+)\s*\</td\>\<td\>',l)
        r = re.match('\<tr\>%s%s%s%s%s%s%s' % (htc('period'),htc('mode'),htc('start'),htc('stop'),htc('run_min'),htc('run_max'),htc('slot')),l)
        if not r: continue
        #print r.group('period'),r.group('mode'),r.group('start'),r.group('stop'),r.group('run_min'),r.group('run_max'),r.group('slot')
        
        if r.group('period')[0]!='0':
            continue
        
        rr = re.match('.*\s*(?P<mode>[LTH]).*',r.group('mode'))
        if not rr:  continue
        mode = rr.group('mode')
        
        slot = int(r.group('slot'))

        p = DstPeriod(r.group('period'),mode=mode,slot=slot)
        p.time_start = r.group('start')
        p.time_stop = r.group('stop')
        p.run_min = int(r.group('run_min'))
        p.run_max = int(r.group('run_max'))
        add_period(p,periods)
    
    return periods

def decode_mDST_name(name):
    import re
    r = re.match('mDST-(?P<run>\d+)(-(?P<cdr>\d+))?-(?P<slot>\d)-(?P<phast>\d+)\.root(\.(?P<num>\d+))?',name)
    num = r.group('num')
    if num!=None:
        num = int(num)
    return int(r.group('run')),r.group('cdr'),int(r.group('slot')),int(r.group('phast')),num

def get_period_files(period,dir_name='mDST',print_files=False):

    from CS.castor import castor_files
        
    compass_data = '/castor/cern.ch/compass/data/'
    d = compass_data+str(period.year())+'/oracle_dst/'+period.name()+'/'+dir_name

    files=[]
    for f in castor_files(d):
        import os
        name = os.path.split(f)[1]
        try:
            run,cdr,slot,phast,n = decode_mDST_name(name)
        except:
            print 'Bad name:',name
            continue
        if slot!=period.slot():
            continue
        files.append(f)

        if print_files:
            print f
    
    return files

def main():
    parser = optparse.OptionParser(version='1.1.3')

    parser.usage = '%prog <options>\n'\
                   'Author: Zvyagin.Alexander@gmail.com'

    parser.description = 'Get DST producation information.'

    parser.add_option('', '--test',action='store_true',dest='test',default=False,
                      help='Run the test suite')
    parser.add_option('', '--page',dest='page',default='http://na58dst1.home.cern.ch/na58dst1/dstprod.html',
                      help='DST producation status page (%default)', type='string')
    parser.add_option('', '--verbose',dest='verbose',default=False,action='store_true',
                      help='Be verbose')
    parser.add_option('', '--time',dest='time',default='',
                      help='Print mDST files for selected years (example: "2002,2004,03P1D")',
                      type='string')
    parser.add_option('', '--mode',dest='mode',default='LTH',
                      help='Select Longitudinal/Transversity/Hadron data, (use L,T,H,LT,etc). Default is %default.',
                      type='string')
    parser.add_option('', '--mDST-chunks',dest='mDST_chunks',default=False,action='store_true',
                      help='Print mDST files on chunks')
    parser.add_option('', '--mDST-merged',dest='mDST_merged',default=False,action='store_true',
                      help='Print mDST merged files')

    (options, args) = parser.parse_args()

    work = False

    if options.test==True:
        work = True
        return unittest.main()

    years=[]
    user_periods=[]
    for y in options.time.split(','):
        if not y:
            continue
        try:
            years.append(int(y))
        except:
            user_periods.append(y)
        work = True

    if options.verbose:
        work = True
        print 'Years to scan:', years
        print 'Periods to scan:',user_periods

    if options.verbose:
        print 'Reading page: ', options.page
    page = get_html_page(options.page)

    if options.verbose:
        print 'Analysing it...'
    periods = read_periods_from_page(page)
    #if options.verbose:
    #    print CYAN,periods,RESET

    if options.verbose:
        for p in periods.values():
            p.pprint()

    if 1:
        for period in periods.values():

            if (not period.year() in years) and (not period.full_name in user_periods):
                continue

            if options.verbose:
                print 'Found user-requested period:',period


            # Check longitudinal or/and transversity data
            if not period.mode() in options.mode:
                continue

            dd = []

            if options.mDST_merged:
                dd.append('mDST')

            if options.mDST_chunks:
                dd.append('mDST.chunks')
            
            for d in dd:
                files = get_period_files(period,d,True)
            
                if options.verbose:
                    print BOLD,GREEN,'There are %d files in %s for period %s' % (len(files),d,period.name()),RESET

                if 0==len(files):
                    print 'No files found for year %d period %s slot %d' % (period.year(),period.name(),period.slot())
                    print 'Directory:',d

    if not work:
        parser.print_help()
        return 1

    return 0

if __name__ == '__main__':
    sys.exit(main())
