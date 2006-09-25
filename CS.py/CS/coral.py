import re,os,sys,optparse
from CS import db

def read_coral_options_file(f):
    """Read an option file resolving 'include' directives"""
    options=[]
    for line in open(f).readlines():
        if re.match("\s*//.*",line):
            continue    # Skip the comments
        inc_flag = re.match("\s*include\s*.*",line)
        res = re.match("\s*include\s*(?P<file>[\_\-\.$\{\}a-zA-Z0-9/]*).*",line)
        if inc_flag:
            file_to_include = res.group('file')
            res = re.match("(.*)\$\{CORAL\}\/(?P<f>.*)( *)",file_to_include)
            if res:
                file_to_include = os.environ['CORAL']+'/'+res.group('f')
            else:
                if file_to_include[0]!='/':
                    file_to_include = start_dir+'/'+file_to_include
            options.extend(read_coral_options_file(file_to_include))
        else:
            options.append(line)
    return options

def make_options(file_opt,evjob):
    options = []

    # read a sample option file and change some lines of it
    for line in read_coral_options_file(file_opt):
        if re.match("^(\s*)histograms\s+package\s(.*)",line):        line = "// " + line
        if re.match("^(\s*)histograms\s+home\s(.*)",line):           line = "// " + line
        if re.match("^(\s*)mDST\s+file\s(.*)",line):                 line = "// " + line
        if re.match("^(\s*)events\s+to\s+read (.*)",line):           line = "// " + line
        if re.match("^(\s*)events\s+to\s+skip (.*)",line):           line = "// " + line
        if re.match("^(\s*)TraF\s+Graph\s+\[0\]([ \t]*)1(.*)",line): line = "TraF Graph [0] 0  // changed!\n"
        if re.match("^(\s*)Data\s+file\s(.*)",line):                 line = "// " + line
        if re.match("^(\s*)Data\s+run select\s(.*)",line):           line = "// " + line
        if re.match("^(\s*)Data\s+container\s(.*)",line):            line = "// " + line
        if re.match("^(\s*)Data\s+period\s(.*)",line):               line = "// " + line
        options.append(line)

    options.insert(0,"\n")
    options.insert(0,"// Unmodified options:\n")
    options.insert(0,"\n")
    options.insert(0,"events to read %d\n" % evjob)
    options.insert(0,"Data file ??\n")
    options.insert(0,"histograms home ???CDR???\n")
    options.insert(0,"mDST file ???CDR???\n")
    options.insert(0,"histograms package ROOT\n")
    
    return options


def __set_options(options,data_file,output_path='',events=111111):
    r = re.match('.*/(?P<file>cdr.*)\.raw',data_file)
    if r==None:
        raise 'Bad file name: %s' % data_file
    ff = r.group('file')
    root_file = ff+'.root'
    mDST_file = 'mDST-'+ff+'.root'
    log_file  = ff+'.log'
    opt_file  = os.getcwd()+'/'+ff+'.opt'

    for o in options:
        r = re.match('(?P<start>.*)(?P<cdr>\?\?\?CDR\?\?\?)(?P<end>.*)',o)
        if r:
            o = r.group('start')+ff+r.group('end')
            continue
        r = re.match('(?P<start>.*)(?P<events>\?\?\?EVENTS\?\?\?)(?P<end>.*)',o)
        if r:
            o = r.group('start')+str(events)+r.group('end')
            continue

    return ff

def set_options(options,tags):
    for i in range(len(options)):
        for tag,value in tags.iteritems():
            pattern = '(?P<start>.*)(?P<cdr>\?\?\?%s\?\?\?)(?P<end>.*)' % tag
            r = re.match(pattern,options[i])
            if r:
                options[i] = r.group('start')+value+r.group('end')

def main():
    
    parser = optparse.OptionParser()
    parser.add_option('', '--opt',dest='opt',
                      help='CORAL option file', metavar='FILE')
    parser.add_option('', '--run',dest='run',
                      help='Run number', type='int')
    parser.add_option('', '--db-access',dest='dbaccess',default='-hna58pc052.cern.ch -uanonymous',
                      help='DB access options (ex: -hhost -ume -ppass)', type='string')
    parser.add_option('', '--events',dest='events',default=33333,
                      help='Number of events for CORAL option "events to read"', type='int')
    parser.add_option('', '--output',dest='output',default='',
                      help='Output path for the root file(s). Can be rfio:/castor/...', type='string')

    (options, args) = parser.parse_args()

    #print 'OUTPUT="%s"' % options.output

    if len(sys.argv)<=1:
        parser.print_help()
        return 1

    #if options.test==True:
    #    unittest.main()

    if options.opt==None:
        raise 'Option file is missing.'
    #coral_options = make_options(options.opt,options.events)
    coral_options = read_coral_options_file(options.opt)

    if options.output and options.output[-1]!='/':
        options.output += '/'

    if options.run!=None:
    
        # List of Patters to be replaced
        tags={}
        tags['\?\?\?EVENTS\?\?\?'] = str(options.events)
    
        for f in db.get_run_files(options.run,True,options.dbaccess):

            # Decode data file name
            r = re.match('.*/(?P<file>cdr.*)\.raw',f)
            if r==None:
                print 'Skipping bad file name: %s' % f
                continue
            short_name = r.group('file')
            
            # Pattern to be changed (regular expression!)
            tags['\?\?\?CDR\?\?\?' ] = short_name
            tags['\?\?\?DATA\?\?\?'] = f

            # Create a file
            fname = short_name+'.opt'
            f = open(fname,'w')

            # write options

            for o in coral_options:

                # Skip all empty lines
                oo = o.strip()
                if not oo:
                    continue

                # Check do we need to change an option
                for tag,value in tags.iteritems():
                    pattern = '(?P<start>.*)(?P<tag>%s)(?P<end>.*)' % tag
                    r = re.match(pattern,oo)
                    if r:
                        f.write('//TAG: '+oo+'\n')      # Write the changed line
                        oo = r.group('start')+value+r.group('end')

                # Add it!
                f.write(oo+'\n')

            print fname

    return 0

if __name__ == '__main__':
    sys.exit(main())
