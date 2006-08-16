from __future__ import generators
import os,sys
try:
    import MySQLdb, optparse
except:
    sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
    import MySQLdb, optparse

class DataBaseMySQL:
    def __init__(self,host_name,user_name='anonymous',password=''):
        # pccoeb03
        # tbed014d
        self.db = MySQLdb.connect(host=host_name, user=user_name, passwd=password)
        self.cursor = self.db.cursor()

    def send(self,command,args=()):
        #print command
        #print args
        self.cursor.execute(command,args)
        return self.cursor

## Print run information from a COMPASS DB machine
def run_info_db(run,dbhost='lxfs1657.cern.ch'):
    table='runlb.tb_run'
    db = DataBaseMySQL(dbhost)
    data={}
    keys=[]

    # Fill the 'data' dictionary with variable types.
    # We only create dictionary structure here, it will be filled later.
    for f in db.send('describe ' + table):
        keys.append(f[0])
        if f[1][:3]=='int':
            data[f[0]]=0    # int  - type
        else:
            data[f[0]]=''   # char - type

    # Now read the 'data' with real values.
    #command = 'SELECT '+','.join(data.keys())+' FROM '+table+' WHERE runnb=%d'%run
    counter=0
    for f in db.send('SELECT * from '+table+' WHERE runnb=%d'%run):
        for i in f:
            data[keys[counter]]=i
            counter+=1
    
    for k,v in data.items():
        print "%11s  %s"%(k,v)

def cdr_files_to_db(auto_delete=False):
    db = DataBaseMySQL('na58pc052.cern.ch','','HMcheops')

    tables = ['run.files','run.info']
    
    #try:
    #    for f in db.send('SELECT COUNT(*) FROM %s' % table):
    #        table_size=int(f[0])
    #except:
    #    table_size=0
    
    if auto_delete==False:
        if 'YES'==raw_input('REMOVING TABLES? Type YES if you are sure: '):
            for t in tables:
                print 'removing the table %s' % t
                db.send('DROP TABLE IF EXISTS %s' % t)
        else:
            print 'OK! Keeping it.'
    
    db.send(r"CREATE TABLE IF NOT EXISTS %s  (`run` INT,`feor` INT,`size` INT,`file` VARCHAR(222), PRIMARY KEY(file)) TYPE=MyISAM;" % tables[0])
    db.send(r"CREATE TABLE IF NOT EXISTS %s  (`run` INT,`year` INT,`period` VARCHAR(11), PRIMARY KEY(run)) TYPE=MyISAM;" % tables[1])
    print 'The tables has been created!'

    for f in cdr_files():
        print f['fname']
        db.send(r"REPLACE INTO %s (run,feor,size,file) VALUES(%d,%d,%d,'%s');" % (tables[0],f['run'],f['feor'],f['size'],f['fname']))
        db.send(r"REPLACE INTO %s (run,year,period) VALUES(%d,%d,'%s');" % (tables[1],f['run'],f['year'],f['period']))

def get_run_files(run,without_feor=False,db_access_opt='-hna58pc052.cern.ch'):
    feor=''
    if without_feor:
        feor = 'AND feor=0'
    command = "mysql -s -B %s -e 'select file from run.files where run=%d %s'" % (db_access_opt,run,feor)
    for f in os.popen(command):
        yield f.strip()

import unittest

class TestCase(unittest.TestCase):
    #def setUp(self):
        #self.top = SE(0,1)
    #    pass

    def test_compass_db(self):
        tmp = sys.stdout
        #sys.stdout = file('/dev/null','w')
        try:
            run_info_db(37059)
        finally:
            sys.stdout = tmp
        

def TheTestSuite():
    return unittest.makeSuite(TestCase,'test')

if __name__ == '__main__':

    parser = optparse.OptionParser()
    parser.add_option('', '--test',action='store_true',dest='test',default=False,
                      help='Run the test suite')
    parser.add_option('', '--run',dest='run',
                      help='Run number', type='int')
    parser.add_option('', '--db-access',dest='dbaccess',default='-hna58pc052.cern.ch',
                      help='DB access options (ex: -hhost -ume -ppass)', type='string')

    (options, args) = parser.parse_args()

    if options.test==True:
        unittest.main()

    if options.run!=None:
        for f in get_run_files(options.run,True,options.dbaccess):
            print f
