import sys,re,optparse

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
    def __init__(self,name,mode,slot):

        ##  Period name
        self.name = name
        
        ##  Data taking mode: 'L' or 'T'
        self.mode = mode

        ##  Slot number in the DST production
        self.slot = slot
        
    def year(self):
        return 2000+int(self.name[:2])
        
    def pprint(self):
        print '%s year=%d mode=%s slot=%d' % (self.name,self.year(),self.mode,self.slot)

def add_period(period,d):
    d[period.name] = period

def read_periods_from_page(page):

    periods={}

    ##  @brief @return regular expression for an html table cell
    def htc(name):
        return '\s*\<td\>\s*(?P<%s>.*?)\s*\</td\>\s*' % name

    for l in page.split('\n'):
        #r = re.match('\<tr\>\s*\<td\>\s*(?P<period>\d\d\w+)\s*\</td\>\<td\>',l)
        r = re.match('\<tr\>%s%s%s%s%s%s%s' % (htc('period'),htc('mode'),htc('start'),htc('stop'),htc('run_min'),htc('run_max'),htc('slot')),l)
        if not r: continue
        #print r.group('period'),r.group('mode'),r.group('start'),r.group('stop'),r.group('run_min'),r.group('run_max'),r.group('slot')
        
        if r.group('period')[0]!='0':
            continue
        
        rr = re.match('.*\s*(?P<mode>[LT]).*',r.group('mode'))
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

def main():
    parser = optparse.OptionParser(version='1.0.1')

    parser.usage = '%prog <options>\n'\
                   'Author: Zvyagin.Alexander@cern.ch'

    parser.description = 'Get DST producation information.'

    parser.add_option('', '--test',action='store_true',dest='test',default=False,
                      help='Run the test suite')
    parser.add_option('', '--page',dest='page',default='http://na58dst1.home.cern.ch/na58dst1/dstprod.html',
                      help='DST producation status page', type='string')
    parser.add_option('', '--verbose',dest='verbose',default=False,action='store_true',
                      help='Be verbose')
    parser.add_option('', '--mdst-scan',dest='mdst',default=False,
                      help='Print mDST files for selected years (default is all years; example: or "2002,2004")',
                      type='string')

    (options, args) = parser.parse_args()

    work = False

    if options.test==True:
        work = True
        return unittest.main()

    years=[]
    if not options.mdst:
        for year in range(2002,2011):
            years.append(year)
    else:
        for year in options.mdst.split(','):
            years.append(int(year))

    if options.verbose:
        work = True
        print 'Years to scan:', years

    if options.verbose:
        print 'Reading page: ', options.page
    page = get_html_page(options.page)

    if options.verbose:
        print 'Analysing it...'
    periods = read_periods_from_page(page)

    if options.verbose:
        for p in periods.values():
            p.pprint()

    if not work:
        parser.print_help()
        return 1

    return 0

if __name__ == '__main__':
    sys.exit(main())
