import os
run  = 37059
ddd  = '/afs/cern.ch/user/z/zvyagin/DDD.git/examples/how-to/ddd'
opts = '--maps=/afs/cern.ch/compass/detector/maps/2004.xml'
fout = file('run.sh','w')
for f in os.popen('python /afs/cern.ch/user/z/zvyagin/cs.git/db.py --run=%d' % run):
    fout.write('%s --plugin=%s/straw_times.so %s %s\n\n' % (ddd,os.getcwd(),opts,f.strip()) )
