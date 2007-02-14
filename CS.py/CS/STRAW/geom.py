##  @addtogroup STRAW
##  @{


##Module to read detector.dat files... in construction
#To do :
#1- Function that compare position for same detector in two different years (especially for z position)
#2- Function that compares internal distances for two different years
import string
import CS.STRAW.names

#Dic of the path to detector.dat files
fname_dic = {2002:'/afs/cern.ch/compass/detector/geometry/2002/detectors.22018.minus.dat',\
            2003:'/afs/cern.ch/compass/detector/geometry/2003/detectors.28568.plus.dat',\
            2004:'/afs/cern.ch/compass/detector/geometry/2004/detectors.35273.plus.dat',\
            2006:'/afs/cern.ch/compass/detector/geometry/2006/detectors.52565.1T.minus.dat'}
            #2006:'/afs/cern.ch/compass/detector/geometry/2006/detectors.52668.plus.dat'}#Corrected alignment (19/01/07)

#l_val = ['ID','TBname','det','unit','type','rad.len.','zsize','xsize','ysize','zcen','xcen','ycen','rot_matr','Wire_dist','angle','Nwires','pitch','effic','backgr','tgate','dr.vel.','T0','res2hit','space','tslice']

straw_dic = {'stw_diam':{'b':0.6144,'a':0.9654,'c':0.9654},'stw_length':{'X':302.8,'Y':365.8,'U':302.8,'V':302.8}}

def det_val(det,value,year):
    ##Returns value from detector det in specified year
    val_dic ={}
    ddatfile = open(fname_dic[year])
    for l in ddatfile.readlines():
        ls_1 = l.split()
        ls = []
        
        #Putting rot matr into one element rot_matr (same with Wire dist)
        for el_1 in ls_1:
            #string.lower(el_1)
            if el_1 == 'rot':
                el_1 = 'rot_matr'
            if el_1 == 'Wire':
                el_1 = 'Wire_dist'
            if el_1 in ('matr','dist'):
                continue
            if ls_1[0] == 'ID':
                ls.append(string.lower(el_1))
            else:
                ls.append(el_1)
        
        if len(ls) == 0:
            continue
        #print ls[0]
        if ls[0] == 'id':
            el_num = 0
            for el in ls[1:]:
                if el == 'mrs:':
                    continue
                val_dic[el] = {}
                val_dic[el]['col'] = el_num+2
                el_num = el_num+1
    
        if ls[0] != 'det':
            continue
        if 'tbname' not in val_dic:
            #print 'tbname not found'
            continue
        if ls[val_dic['tbname']['col']] == det:
            try :
                return float(ls[val_dic[value]['col']])
            except KeyError:
                print '%s does not exist in detector.dat file!'%value
                print 'Choose from :'
                for k in val_dic:
                    print k
                return
    #When nothing is found
    print '%s not found for year %d!'%(det,year)
 

def num_wire(det):
    ##Returns the number of wire in a detector
    if det[4] in ('X','U','V'):
        if det[7] in ('a','c'):
            return 96
        elif det[7] == 'b':
            return 222
        else:
            raise 'Wrong detector name : %s'%det
    elif det[4] == 'Y':
        if det[7] in ('a','c'):
            return 64
        elif det[7] == 'b':
            return 192
        else:
            raise 'Wrong detector name : %s'%det

def d_det(det1,det2,value,year):
    ##Returns the difference btw the same value from det1 and det2 in the specified year
    return det_val(det1,value, year) - det_val(det2,value, year)
        

def w_pitch(det):
    ##Returns the static wire pitch of the given detector (an individual pitch can be taken with 'pitch' value from det_val)
    if det[7] in ('a','c'):
        return 0.9654
    elif det[7] == 'b':
        return 0.6144
    else:
        raise 'Wrong detector name : %s'%det

def cen_wEnd(det,year):
    ##Returns the center in the direction perpenticular of the wire
    if det[4] in ('X','U','V'):
        cen = det_val(det,'xcen',year)
    elif det[4] == 'Y':
        cen = det_val(det,'ycen',year)
    else:
        raise 'Wrong detector name : %s'%det_w
    return cen

def first_wire(det,year):
    ##Returns the position of the first wire in the direction perpendicular to the wire in MRS
    return det_val(det,'wire_dist', year) + cen_wEnd(det,year)

def last_wire(det,year):
    ##Returns the position of the last wire in the direction perpendicular to the wire in MRS
    return first_wire(det, year) + (num_wire(det)-1)*det_val(det,'pitch',year)

def cen_fwir_dist(det_c,det_w,year):
    ##Returns the distance btw the FIRST wire of det_w and the center of det_c 
    if det_c[4] != det_w[4]:
        raise 'Detectors have to be of the same type'    
    return first_wire(det_w,year) - cen_wEnd(det_c,year)

def cen_lwir_dist(det_c,det_w,year):
    ##Returns the distance btw the LAST wire of det_w and the center of det_c 
    if det_c[4] != det_w[4]:
        raise 'Detectors have to be of the same type'
    
    return last_wire(det_w,year) - cen_wEnd(det_c,year)
    

def year_comp(value,det1,det2,year1,year2):
    ##Compares a given value for a given detector for the two given years
    val_y1 = det_val(det1,value,year1)
    val_y2 = det_val(det2,value,year2)
    diff = val_y2 - val_y1
    print '%s : %s (%d) = %.2f'%(det1,value,year1,val_y1)
    print '%s : %s (%d) = %.2f'%(det2,value,year2,val_y2)
    print 'Diff (%d-%d) = %.2f'%(year1,year2,diff)
    return diff


def ud_Woffset(det,year):
    #returns dic of offset btw upstream and downstream wire for the 3 section of the double layer ex : ST03X1
    if len(det) != 6:
        raise 'Wrong det name : %s, should be like ST0???'%det
    offset = {}
    for s in ('a','b','c'):
        offset[s] = first_wire(det+'d'+s,year)-first_wire(det+'u'+s,year)
    return offset

def Woffset_rep(year):
    ##Makes a report of the all detector from the given year
    offset_lim = 1.5#radius factor defining the limit of accepted offset
    prob_det = 0#Number of 'problematic' planes
    for det in CS.STRAW.names.straw_short_names(year):
        offset_dic = ud_Woffset(det,year)
        for sec in ('a','b','c'):
            if abs(offset_dic[sec]) > offset_lim*(0.5)*straw_dic['stw_diam'][sec]:#If the wire offset is bigger than offset_lim times radius...
                print '%s : First wire offset:%.2f, is bigger than %.2f*radius, %.2f'%(det+'_'+sec,offset_dic[sec] ,offset_lim,offset_lim*(0.5)*straw_dic['stw_diam'][sec])

    
def scan_1w_dist(year):
    ##Scans the difference btw 1st wires of 10mm sections the 1rst wire of the 6mm section in the same double plane
    for det in CS.STRAW.names.straw_short_names(year):
        print det
        for plane in ('u','d'):
            for sec in ('a','c'):
                #print det+plane+sec
                print '%s : %.2f'%(plane+sec,first_wire(det+plane+sec,year) - first_wire(det+plane+'b',year))



##  @}
