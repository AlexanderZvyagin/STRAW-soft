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

if __name__=='__main__':
    page = get_html_page('http://na58dst1.home.cern.ch/na58dst1/dstprod.html')
    periods = read_periods_from_page(page)

    for p in periods.values():
        p.pprint()

def main():
    parser = optparse.OptionParser(version='1.0.0')

    parser.usage = '%prog <options>\n'\
                   'Author: Zvyagin.Alexander@cern.ch'

    parser.description = 'Get DST producation information.'

    parser.add_option('', '--test',action='store_true',dest='test',default=False,
                      help='Run the test suite')
    parser.add_option('', '--page',dest='page',default='http://na58dst1.home.cern.ch/na58dst1/dstprod.html',
                      help='Run number', type='string')

    (options, args) = parser.parse_args()

    work = False

    if options.test==True:
        work = True
        return unittest.main()

    if 1:
        work = True
        page = get_html_page(options.page)
        periods = read_periods_from_page(page)
        for p in periods.values():
            p.pprint()

    if not work:
        parser.print_help()
        return 1

    return 0

if __name__ == '__main__':
    sys.exit(main())
