import re,os,sys,optparse

def main():
    parser = optparse.OptionParser(version='1.0.1')
    parser.description = 'Prepare LSF jobs to start.'
    parser.usage = 'cs %prog [options] <coral1.opt> [<coral2.opt> ...]\n'\
                   'Author: Alexander.Zvyagin@cern.ch'
    parser.add_option('', '--queue',dest='queue',action='store',
                      help='LSF queue name, type [no-name] for the list of possible queues',
                      type='string',metavar='NAME')
    parser.add_option('', '--lsf-opts',dest='opts',
                      help='List of extra options to be passed to the LSF', type='string',metavar='OPTIONS')
    parser.add_option('', '--name',dest='name',
                      help='Job name', type='string',metavar='NAME')
    parser.add_option('', '--coral',dest='coral',
                      help='Path to a CORAL executable.', type='string',metavar='PATH')
    parser.add_option('', '--output',dest='output',
                      help='Output (castor) directory.', type='string',metavar='PATH')
    parser.add_option('', '--jobs-max',dest='jobs_max',default=999,
                      help='Maximum number of jobs to submit.', type='int',metavar='number')

    (options, args) = parser.parse_args()

    if len(sys.argv)<=1:
        parser.print_help()
        return 1

    #if options.test==True:
    #    unittest.main()

    if options.queue:
        name = 'jobs.sh'
        all_cmds = file(name ,'w')
        os.system('chmod +x %s' % name)
    else:
        os.system('bqueues')
        return 1

    if options.output[:8]=='/castor/':
        cp_command = 'rfcp'
    else:
        cp_command = 'cp'

    options.coral  = os.path.abspath(options.coral)
    options.output = os.path.abspath(options.output)

    for o in args:
        opt = os.path.abspath(o)
        options.jobs_max -= 1
        if options.jobs_max<0:
            print 'Maximum number of jobs is reached. Stopping.'
            break
        
        if not os.access(opt,os.R_OK):
            print 'File %s is not accessable.' % opt
            continue
        
        # Now the shell script is created.
        name,ext = os.path.splitext(opt)
        name = os.path.split(name)[1]
        script_sh = os.path.abspath(name)+'.sh'
        script = file(script_sh,'w')
        os.system('chmod +x %s' % script_sh)
        coral_run = '%s %s > /tmp/%s.log 2>&1' % (options.coral,opt,name)
        script.write(coral_run + '\n\n')

        for f in [ ('mDST-','.root'), ('','.root'), ('','.gfile'), ('','.log')]:
            f_in  = '/tmp/%s%s%s' % (f[0],name,f[1])
            f_out = '%s/%s%s%s' % (options.output,f[0],name,f[1])
            script.write('%s %s %s\n' % (cp_command,f_in,f_out))
            script.write('rm -f %s\n' % f_in)

        script.write('%s %s %s\n' % (cp_command,opt,options.output))
        script.write('rm -f %s %s\n' % (script_sh,opt))

        if options.queue:
            all_cmds.write('bsub -q %s -oo lsf-%s.log ' % (options.queue,name) )
            if options.name:
                all_cmds.write('-J %s ' % options.name)
            all_cmds.write('%s\n' % script_sh)

    return 0

if __name__ == '__main__':
    sys.exit(main())
