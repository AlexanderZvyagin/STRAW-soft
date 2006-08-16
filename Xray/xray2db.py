#!/bin/env python

import getopt,sys,os,math

def read_spacers(line):
    spacers=[]
    sp_avr = 0
    for sp in line.split()[2:]:
        sp_avr += float(sp)
        spacers.append(float(sp))
    assert len(spacers)==6
    sp_avr /= len(spacers)
    for i in range(len(spacers)):
        spacers[i] -= sp_avr
    return spacers

def make_detector(name,spacers,lines,options):
    print name
    #print ''.join(lines)
    f = open(name,'w')
    channels=[]
    channels_x_pos=[]
    N = 6
    assert N==len(spacers)
    angle = 0
    x_avr = 0
    y_avr = 0
    for l in lines:
        l = l.strip().split()
        assert len(l)==7
        spx=[]
        channel_x_pos = 0
        for sp in l[1:]:
            x = float(sp)/1000.
            spx.append(x)
            channel_x_pos += x
            x_avr += x
        channel_x_pos /= N
        channels_x_pos.append(channel_x_pos)
        angle += (spx[-1]-spx[0])/(spacers[-1]-spacers[0])
        #print 'angle:',spx[-1],spx[0],spacers[-1],spacers[0]
        #print '==>>',(spx[-1]-spx[0]),(spacers[-1]-spacers[0]),(spx[-1]-spx[0])/(spacers[-1]-spacers[0])
    angle = math.atan(angle/len(lines))
    x_avr /= N*len(lines)
    #print 'x_avr=',x_avr
    d_sum = 0
    d_sum2 = 0
    for i in range(1,len(channels_x_pos)):
        d = channels_x_pos[i]-channels_x_pos[i-1]
        d_sum += d
        d_sum2 += d*d
        #print 'pitch: ',d
    d_avr   = d_sum/(len(channels_x_pos)-1)
    d_sigma = math.sqrt(d_sum2/(len(channels_x_pos)-2)-d_avr*d_avr)
    #print d_avr,d_sigma
    assert d_sigma<2000
    pitch_average = d_avr
    print 'pitch=',pitch_average,' pitch_sigma=',d_sigma,'   angle=',angle
    channel_first_pos = -(len(lines)-1)/2.*pitch_average
    
    f.write('spacers: ')
    for sp in spacers:
        f.write(' %g' % sp)
    f.write('\n')

    for n in range(len(lines)):
        ch = lines[n].strip().split()
        for i in range(6):
            # Original value
            sp = float(ch[i+1])/1000.

            # Offset
            sp -= x_avr

            # Rotation
            sp = sp*math.cos(angle)-spacers[i]*math.sin(angle)
            
            # Convert absolute coordinate to correction
            sp -= pitch_average*n+channel_first_pos
            
            # Convert to milimiters
            #sp /= 1000

            f.write('%g ' % sp)
        f.write('\n')
    #sys.exit(0) # DEBUG

def convert_file(name_xray,name_db,options=''):
    lines = os.popen('cat %s' % name_xray).readlines()
    
    #spacers = read_spacers(lines[2].strip())
    #print spacers
    
    #spacers={}
    #for l in lines[4:]:
    #    sp=6*[0]
    #    fields = l.strip().split()
    #    channel=int(fields[0])-1
    #    spacers[channel]=[]
    #    for f in fields[1:]:
    #        spacers[channel].append(float(f))
    #    print spacers
        
    for abc in ('a','b','c'):
        if name_xray[-3]=='Y':
            sp = 651
            if abc=='a':
                line_start = 0
                line_end   = 64
            if abc=='b':
                line_start = 64
                line_end   = 64+192
            if abc=='c':
                line_start = 64+192
                line_end   = 64+192+64
        else:
            sp = 547
            if abc=='a':
                line_start = 0
                line_end   = 96
            if abc=='b':
                line_start = 96
                line_end   = 96+222
            if abc=='c':
                line_start = 96+222
                line_end   = 96+222+96
        ll=lines[4+line_start:4+line_end]
        #print 'ASSERT:',name_xray,len(ll),line_end-line_start,line_start,line_end
        assert len(ll)==line_end-line_start
        spacers = [-sp*2.5,-sp*1.5,-sp*0.5,sp*0.5,sp*1.5,sp*2.5]
        make_detector(name_db+abc,spacers,ll,options)

def convert(xray,db):
    try:
        # Try to create a db directory.
        # It is ok if we fail here
        os.mkdir(db)
    except:
        pass

    for i in range(1,10):
        for xyuv in ('X','Y','U','V'):
            for n in (1,2):
                for ud in ('u','d'):
                    name_xray = '%s/ST%d%c%d%c'  % (xray,i,xyuv,n,ud)
                    name_db   = '%s/ST0%d%c%d%c' % (db,  i,xyuv,n,ud)
                    try:
                        os.stat(name_xray)
                    except:
                        for abc in ('a','b','c'):
                            try:
                                os.remove(name_db+abc)
                            except:
                                pass
                        continue
                    convert_file(name_xray,name_db)


if __name__ == '__main__':


    db='.'
    xray='.'

    try:
        opts,vargs = getopt.getopt(sys.argv[1:],'',['xray=','db=','help'])
    except Exception, what:
        print sys.exc_info()[0],':',what
        sys.exit(1)
    for opt, arg in opts:
        if opt=='--help':
            print 'xray2db --xray=<path> [--db=<path>]'
            sys.exit(0)
        elif opt=='--xray':
            xray=arg
        elif opt=='--db':
            db=arg
        else:
            raise Exception('internal problem')

    convert(xray,db)
