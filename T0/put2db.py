import os,re
import names

# Run it from a gateway machine.

t_start = '2006-01-01-00:00:00'
t_end   = '2008-12-31-23:59:59'

if 0:
    calib_path='/afs/cern.ch/user/z/zvyagin/STRAW-soft.git/T0/rough/macros/%s-T0.calib'
    for detector in names.straw_full_names(2006):
        fname = calib_path % detector
        os.system('addFileInDB -begin %s -end %s -detname %s -typecalib T0 -passwd HMcheops %s' % (t_start,t_end,detector,fname))
if 1:
    rf = '/afs/cern.ch/user/r/rajotte/w0/python_tests/av_T0_dtr_ALL.txt'
    RT_10mm = \
"""
RTGrid
 0.0    0.00    0.5
13.0    0.48    0.36
19.8    0.96    0.40
36.4    2.40    0.35
57.0    3.84    0.31
65.9    4.32    0.03
83.4    4.80    0.03
"""

    RT_6mm = \
"""
RTGrid
 0.0    0.00    0.3
 8.8    0.31    0.22
14.0    0.62    0.27
25.4    1.55    0.22
36.4    2.48    0.18
42.1    2.79    0.14
50.3    3.10    0.12
"""

    det_t0 = {}
    for l in open(rf).readlines():
        r = re.match('(?P<det>[\w\d]+)\s+(?P<T0>[-\.\d]+)',l.strip())
        if not r:
            print 'Bad line:',l.strip()
            continue
        det_t0[r.group('det')] = float(r.group('T0'))

    for detector in names.straw_full_names(2006):
        fname = '%s-RT.calib' % detector
        f = open(fname,'w')
        f.write('%.2f' % det_t0[detector])
        if detector[7]=='b':
            rt = RT_6mm
        else:
            rt = RT_10mm
        f.write(rt)
        f.close()
        #fname = calib_path % detector
        os.system('addFileInDB -begin %s -end %s -detname %s -passwd HMcheops %s' % (t_start,t_end,detector,fname))
    
