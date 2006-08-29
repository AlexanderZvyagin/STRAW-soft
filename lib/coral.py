import re,os,sys

sys.path.append('/afs/cern.ch/user/z/zvyagin/w0/cs.git')
import db

try:
    import optparse
except:
    sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
    import optparse

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


def set_options(options,data_file,output_path=''):
    r = re.match('.*/(?P<file>cdr.*)\.raw',data_file)
    if r==None:
        raise 'Bad file name: %s' % data_file
    ff = r.group('file')
    root_file = ff+'.root'
    mDST_file = 'mDST-'+ff+'.root'
    log_file  = ff+'.log'
    opt_file  = os.getcwd()+'/'+ff+'.opt'

    options[1] = "mDST file %smDST-%s\n" % (output_path,root_file)
    options[2] = "histograms home %s%s\n" % (output_path,root_file)
    options[3] = "Data file %s\n" % data_file

    for i in range(len(options)):
        r = re.match('(?P<start>.*)(?P<cdr>\?\?\?CDR\?\?\?)(?P<end>.*)',options[i])
        if r:
            options[i] = r.group('start')+ff+r.group('end')
#            print options[i]

    return ff

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
        return

    #if options.test==True:
    #    unittest.main()

    if options.opt==None:
        raise 'Option file is missing.'
    coral_options = make_options(options.opt,options.events)

    if options.output and options.output[-1]!='/':
        options.output += '/'

    if options.run!=None:
        for f in db.get_run_files(options.run,True,options.dbaccess):
            # copy options
            opts = []
            for o in coral_options:
                opts.append(o)
            # set them!
            short_name = set_options(opts,f,options.output)
            fname = short_name+'.opt'
            open(fname,'w').write(''.join(opts))
            print fname


if __name__ == '__main__':
    main()
