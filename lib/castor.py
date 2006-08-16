import string,re,os,sys

## Dictionary with the CDR file attributes
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
                    f['year']=2004
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
                f['year']=2004
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


# Generator of CDR files from castor
def cdr_files(printout=0):
    for f in cdr_files_2002():
        yield f
    for f in cdr_files_2003():
        yield f
    for f in cdr_files_2004():
        yield f

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

if __name__ == '__main__':
    print 'This module is not callable.'
    print 'Running self-test...'
    unittest.main()
