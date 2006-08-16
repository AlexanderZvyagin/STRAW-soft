#python T0_card_pics.py --db-access="-hna58pc052.cern.ch -uzvyagin -pHMcheops" T0_geoID_r T0-r

import sys, os
import draw

if __name__=='__main__':

    try:
        import optparse
    except:
        sys.path.append('/afs/cern/ch/user/z/zviagine/public/local/lib')
        import optparse

    parser = optparse.OptionParser()

    parser.usage = '%prog [options] <table> <dir>'
    parser.description = 'Create list of pictures.'

    draw.optparse_parameters(parser,'eps','400x300')
    parser.add_option('', '--db-access',dest='dbaccess',default='-hna58pc052.cern.ch',
                      help='DB access options (ex: -hhost -ume -ppass)', type='string')
    parser.add_option('', '--T0-error',dest='T0_error',default=0.21,
                      help='T0 error', type='float')

    (options, args) = parser.parse_args()
    
    if len(args)!=2:
        parser.print_help()
        sys.exit(1)

    table = args[0]
    d     = args[1]

    root_opts = ''
    if options.root_file:
        root_opts += '--root=%s' % options.root_file
        if options.root_dir:
            root_opts += ' --root-dir=%s' % options.root_dir

    os.system('cd ..;python dT_card.py --pic-ext=%s --pic-size=%s --db-access=\"%s\" %s %s' % \
              (options.pic_ext,options.pic_size,options.dbaccess,root_opts,table) )
    os.system('cd ..;python det_card.py --pic-ext=%s --pic-size=%s --t0-tex=t0-results.tex --db-access=\"%s\" %s %s all %g' % \
              (options.pic_ext,options.pic_size,options.dbaccess,root_opts,table,options.T0_error) )

    for card in [436,312,259,260,385,513,279,452,501,130,72,376,568]:
        os.system('cd ..;python Vs_card.py --pic-ext=%s --pic-size=%s %s --dir=%s %d' % \
                   (options.pic_ext,options.pic_size,root_opts,d,card) )

    os.system('cd ..;python T0_V_CORAL_diff.py --pic-ext=%s --pic-size=%s %s cards.tex coral-t0.txt %g' % \
               (options.pic_ext,options.pic_size,root_opts,options.T0_error) )
