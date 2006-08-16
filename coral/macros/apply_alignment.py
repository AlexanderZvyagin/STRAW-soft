import os,re

def apply_alignment(detdat,detector,offset,pitch_corr):
    for d in range(len(detdat)):
        dd = detdat[d].split()
        if len(dd)<=2: continue
        if not (dd[0]=='det' and dd[2]==detector):  continue
        #print d
        print detector,offset,pitch_corr
        
        wire1 = float(dd[14])
        wire1 -= offset
        dd[14] = str(wire1)

        pitch = float(dd[17])
        pitch *= 1-pitch_corr
        dd[17] = str(pitch)

        detdat[d] = ' '+'  '.join(dd)+'\n'
        

alignment={}  # detector <==> [offset,pitch-cor]

def read_alignment(file_name):
    for o in os.popen('cat %s' % file_name).readlines():
        r = re.match('alignment of .*(?P<a>Udist_ST......).*',o)
        if not r: continue
        detector = r.group('a')[6:]
        assert len(detector)==8
        offset    = float(o.split(' ')[-2])
        pitch_cor = float(o.split(' ')[-1])
        #print detector,offset,pitch_cor
        alignment[detector]=[offset/10000,pitch_cor/1000]

read_alignment('log')
detdat=os.popen('cat detectors.dat').readlines()

for det,a in alignment.items():
    apply_alignment(detdat,det,a[0],a[1])

open('d','w').write(''.join(detdat))
