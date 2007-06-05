import sys, os, string, re
import ROOT
#from ROOT import gROOT,TFile,TChain
from optparse import OptionParser

import CS.STRAW.names

ROOT.gROOT.Reset()

def get_files(d):
    files = []
    for f in os.popen('rfdir %s' % d):
        f = string.strip(f.split(' ')[-1])
        r = re.match('cdr\d+-\d+\.root',f)
        if r==None:
            continue
        files.append('%s/%s' % (d,f))
    return files


parser = OptionParser(usage="<script> <dir> <output.root>")
(options, args) = parser.parse_args()
if len(args)!=2:
    print 'Bad options/arguments. Type -h for help'
    sys.exit(1)


files = get_files(args[0])

root_file = ROOT.TFile.Open(args[1],'RECREATE','',9)

chains = []

for st in CS.STRAW.names.straw_full_names(2007):
    print 'Merging %s' % st
    root_file.cd()
    chains.append(ROOT.TChain('%s_CORAL' % st))
    #continue
    #print ch
    for f in files:
        print f
        chains[-1].Add('rfio:%s' % f)
    #continue
    #ch.Merge(args[1])
    root_file.cd()
    chains[-1].Write()

root_file.Write()
root_file.Close()
