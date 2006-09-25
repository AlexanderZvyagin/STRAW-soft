# @brief Submit CORAL jobs to LSF

import os,sys,optparse,shutil

def main():

    parser = optparse.OptionParser()

    parser.usage = '%prog <CORAL dir> <CORAL exe>  <CORAL opts> <output-dir> <run>'
    parser.description = 'Submit CORAL jobs to LSF.'
    parser.long_description = 'The output will be written to '\
                         '<output-dir-prefix>/<run>'

    parser.add_option('', '--test',dest='test',default=False,action='store_true',
                      help='Test mode (one job, 8nm queue, 10 events)')

    parser.add_option('', '--submit',dest='submit',default=False,action='store_true',
                      help='Submit jobs at the end!')
    
    (options, args) = parser.parse_args()

    if len(args)!=5:
        parser.print_help()
        return 1

    coral_dir = args[0]
    coral_exe = args[1]
    coral_opt = args[2]
    output    = args[3]
    run       = int(args[4])
    run_dir   = os.getcwd()+'/'+str(run)

    # First we check for a 'CORAL' environment variable.
    # If it is not available, we set it and run the script again.
    if not os.environ.get('CORAL'):
        print 'CORAL is not initialized. I will do this now.'
        # We have to do (under bash):
        # 1) start a bash script which will:
        # 3) cd $CORAL
        # 4) . setup.sh
        # 5) Go back to the present directory
        # 6) Run the current script with the same parameters again (now with $CORAL)

        commands = []
        commands.append('cd %s' % coral_dir)
        commands.append('. setup.sh')
        commands.append('cd %s' % os.getcwd())
        
        if os.path.splitext(sys.argv[0])[1]:
            run_me = 'python'
        else:
            run_me = 'cs'
        commands.append('%s %s' % (run_me,' '.join(sys.argv)))
        commands.append('exit')
        
        bash_run = 'bash -c "%s"' % ' && '.join(commands)
        print '******************************'
        print '*** Executing the command: ***'
        print bash_run
        print '******************************'

        return os.system(bash_run)
    else:
        print 'CORAL path is: ',os.environ.get('CORAL')

    if os.path.abspath(os.environ.get('CORAL'))!=os.path.abspath(coral_dir):
        print 'Your CORAL variable is not the same as in the command line!'
        print '  ENV: ',os.path.abspath(os.environ.get('CORAL'))
        print '  CMD: ',os.path.abspath(coral_dir)
        return 1

    print 'Checking the CORAL executable...'
    sys.stdout.flush()
    if os.system('%s 2>&1 | grep "usage:" > /dev/null' % coral_exe):
        print 'Problem in running the CORAL!'
        os.system(coral_exe)
        return 1

    print 'Creating output directories...'

    # Main output directory...
    if os.system('rfmkdir %s' % output):
        print 'Failed to create the output directory!'
        return 1
    
    # Create directory for temporary files
    os.mkdir(run_dir)
    os.chdir(run_dir)
    
    shutil.copyfile(coral_opt,'coral.opt')
    if os.system('grep \?\?\?DATA\?\?\? coral.opt > /dev/null'):
        print 'Bad options file (no data tag ???DATA??? is found).'
        return 1

    print 'Getting run data files and generating the CORAL option files...'
    sys.stdout.flush()
    if os.system('cs coral --run=%d --opt=coral.opt > coral_opts.log ' % run):
        return 1
    chunks = int(os.popen('wc coral_opts.log').readline().split()[0])
    print 'Run %d has %d chunks.' % (run,chunks)
    
    if options.test:
        queue = '8nm'
        extra = '--jobs-max=1'
    else:
        queue = '1nd'
        extra = ''

    if os.system('cs lsf --queue=%s --coral=%s --output=%s %s cdr*.opt' % (queue,coral_exe,output,extra)):
        print 'Failed to execute the jobs! Why?'
        return 1
    
    if options.submit:
        if os.system('./jobs.sh > jobs.log'):
            print 'Something went wrong in the jobs submitting!'
        n_jobs = int(os.popen('wc jobs.log').readline().split()[0])
        print 'It was submitted %d jobs for %d chunks' % (n_jobs,chunks)
    else:
        print 'File "%s" is ready for an execution.' % os.path.abspath('jobs.sh')
    
    return 0

if __name__=='__main__':
    sys.exit(main())
