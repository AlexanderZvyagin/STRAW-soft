import re,os,sys

try:
    import optparse
except:
    sys.path.append('/afs/cern.ch/user/z/zviagine/public/local/lib')
    import optparse

def main():
    parser = optparse.OptionParser()
    parser.description = 'Run LSF jobs.'
    parser.usage = 'python lsf.py [options] <coral1.opt> [<coral2.opt> ...]'
    parser.add_option('', '--queue',dest='queue',action='store',
                      help='LSF queue name, type [no-name] for the list of possible queues',
                      metavar='NAME', type='string')
    parser.add_option('', '--lsf-opts',dest='opts',
                      help='List of extra options to be passed to the LSF', type='string')
    parser.add_option('', '--name',dest='name',
                      help='Job name', type='string')
    parser.add_option('', '--coral',dest='coral',
                      help='Path to a CORAL executable.', type='string')
    parser.add_option('', '--output',dest='output',
                      help='Output castor directory.', type='string')

    (options, args) = parser.parse_args()

    if len(sys.argv)<=1:
        parser.print_help()
        return

    #if options.test==True:
    #    unittest.main()

    if options.queue:
        name = '%s.sh' % options.queue
        all_cmds = file(name ,'w')
        os.system('chmod +x %s' % name)
    else:
        os.system('bqueues')
        return

    for opt in args:
        if not os.access(opt,os.R_OK):
            print 'File %s is not accessable.' % opt
            continue
        
        # Now the shell script is created.
        name,ext = os.path.splitext(opt)
        script_sh = os.getcwd()+'/'+name+'.sh'
        script = file(script_sh,'w')
        os.system('chmod +x %s' % script_sh)
        coral_run = '%s %s' % (options.coral,os.path.abspath(opt))        
        script.write(coral_run + '\n\n')

        for f in [ ('mDST-','.root'), ('','.root'), ('','.gfile')]:
            f_in  = '/tmp/%s%s%s' % (f[0],name,f[1])
            f_out = '%s/%s%s%s' % (options.output,f[0],name,f[1])
            script.write( 'rfcp %s %s\n' % (f_in,f_out))
            script.write('rm -f %s\n' % f_in)

        if options.queue:
            all_cmds.write('bsub -q %s -oo %s.log ' % (options.queue,name) )
            if options.name:
                all_cmds.write('-J %s ' % options.name)
            all_cmds.write('%s\n' % script_sh)

if __name__ == '__main__':
    main()
