import os

mysql_run     = 'mysql -h lxfs1657 -u anonymous'
mysql_command = "select detname,filename from calibdb.tb_calibDB WHERE (tb_calibDB.entrytime >= '04-09-01') and (tb_calibDB.starttime <= '2004-07-03 02:49:15') and (tb_calibDB.endtime >= '2004-07-03 02:49:15') and (tb_calibDB.dettype = 'ST') and ( tb_calibDB.typecalib like 'default')"
cdb_dir = '/afs/cern.ch/compass/detector/calibrations/MySQLDB_files3/'

for sel in os.popen('%s --batch --silent -e \"%s\"' % (mysql_run,mysql_command) ).readlines():
    det,fname = sel.strip().split()
    os.system('cp %s/%s %s.RT' % (cdb_dir,fname,det))
    #print det,fname
    #t0 = file(cdb_dir+fname).readlines()[0]
    #t0 = float(t0)
    #print det,t0

