import os

repository = 'file://$HOME/w0/svn/Detectors'
os.system('rm -fr Detectors.svn')
#os.system('mv -f Detectors Detectors.old')
os.system('svn co %s Detectors.svn' % repository)
#os.system('find Detectors.svn -name .svn | xargs rm -rf')
os.system('cp Detectors.svn/src/* Detectors/src')
