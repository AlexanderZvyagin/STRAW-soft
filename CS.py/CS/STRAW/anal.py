import ROOT
import os

def read_table(table,detector):
    ##Returns a dictionary made of information taken from the given table for the given detector in the format {channel}{position}{value}
    ch_dic = {}

    #Checking if the table is in the database   
#    command = "mysql -s -B %s -e 'DESCRIBE STDC.%s'" % (dbaccess,table)
#    command = "mysql -s -B -hna58pc052 -pHMcheops -e 'DESCRIBE STDC.%s'" % (table)
    command = "mysql -s -B -hna58pc052 -uanonymous -e 'DESCRIBE STDC.%s'" % (table)
    
    table_info = os.popen(command).readlines()
#    print table_info
    if not table_info:
#        print 'Cannot acces table'
        raise 'Cannot access the table!'

#    command = "mysql -s -B -hna58pc052 -e 'select detector,chf,chl,pos,T0,data from STDC.%s'" % (table)
#    command = "mysql -s -B -hna58pc052 -e 'select detector,chf,chl,pos,T0,W,data from STDC.%s == %s'" % (table)
#    command = "mysql -s -B -hna58pc052 -e 'select chf,chl,pos,T0,W,data from STDC.%s where detector='%s''" % (table,detector)
#    command = "mysql -s -B -hna58pc052 -e 'select chf,chl,pos,T0,W,data from STDC.%s where detector='%s''" % (table,detector)
    command = "mysql -s -B -hna58pc052 -uanonymous -e 'select chf,chl,pos,T0,W,resCORAL_RMS_750,data from STDC." + table + ' where detector="' + detector + '"' + "'"
    for f in os.popen(command):
#        det,chf,chl,pos,t0,w,res,data = f.strip().split()
        chf,chl,pos,t0,w,res,data = f.strip().split()
        chf  = int(chf)
        chl  = int(chl)
        pos  = int(pos)
        t0   = float(t0)
        w    = float(w)
        res  = float(res)*10000#To have in micrometer units
        data = int(data)
        
#        if not data:
#            continue

        #DB contains entries for channal > 191 although they do not exists here we exclude them
        if detector[4]=='Y' and chf>191:
            continue
        try:
            ch_dic[chf]
        except:
            ch_dic[chf]={}
        #Create a dictionnary for every position in a channel
        try:
            ch_dic[chf][pos]
        except:
            ch_dic[chf][pos]={}
        ch_dic[chf][pos]['T0'] = t0 
        ch_dic[chf][pos]['W0'] = w 
        ch_dic[chf][pos]['RES'] = res 
        ch_dic[chf][pos]['DATA'] = data 
        
    if len(ch_dic) == 0:
        raise 'Could not create dictionnary for %s on table %s'%(detector,table)
    return ch_dic



#This function takes a dictionnary (created by the function read_table)
#and return of a hist. value (T0 or W0) vs channel for a given position
def plot_ValuevsCH(D_V_ch,Values = 'T0',pos = 21):
    h_vs_ch = ROOT.TH1F("h_vs_ch",Values + " vs Channel, " + "pos= " + str(pos),len(D_V_ch),0,len(D_V_ch))
    for i in D_V_ch.items():
        try:
            h_vs_ch.Fill(i[0],i[1][pos][Values])
        except:
            print 'skipping channel' + str(i[1])
            h_vs_ch.Fill(i[0],0)
               
    return h_vs_ch   


def w0vsCH(table = 'ST03_4',detector = 'ST03X1ub',pos = 21):
    my_dict = read_table(table,detector)
    h = plot_ValuevsCH(my_dict,'W0',pos)
    h.Draw()


def t0vsCH(table,detector,pos):
    my_dict = read_table(table,detector)
    h = plot_ValuevsCH(my_dict,'T0',pos)
    h.Draw()


#This function returns a distribution histogram of "Value"
#note: value = 0 are excluded
def dist_Value(D_V,value,pos):

    if value == 'W0':
        rng = 0.05
    elif value == 'RES':
        rng = 100
    else:
        rng = 10
    #get the average for histogram range
    avg = av_value(D_V,value,pos)
#    h_d = ROOT.TH1F("h_d",value + " distribution, " + "pos= " + str(pos),50,avg-10,avg+10)#CHANGE THIS to dynamical input
    h_d = ROOT.TH1F("h_d",value + " distribution, " + "pos= " + str(pos),50,avg-rng,avg+rng)#CHANGE THIS to dynamical input
    
    for i in D_V.items():
        #We remove value that are exactly 0
        if i[1][pos][value] == 0:
            continue
        #Execption will come for physical hole (pos = 0)
        try :
            h_d.Fill(i[1][pos][value])
        except :
            if i[1] not in range(95,126) and pos == 0:#Has to be modify for Y-chamber
                print 'skipping channel' + str(i[1])
            continue #Skipping when it does not exist
    return h_d


#This function returns the average of a parameter "value" in dictionary "d" 
#Note : It exclude 0 values  
def av_value(d,value,pos) :
    if len(d)==0:
        raise 'Received empty dictionary!'
    v_sum = 0;
    v_num = 0;
    for i in d.items():
        #Execption will come for physical hole (pos = 0)
        try :
            if i[1][pos][value] == 0:
                continue
            v_sum = v_sum + i[1][pos][value]
            v_num = v_num + 1
        except :
            print 'Skipping ch ' + str(i[0])
            continue
    return v_sum/v_num
 

#This function fits a gaussian to the given histogram
def fit_val_dist(h):
    f = ROOT.TF1("f","gaus")
    f.SetParLimits(0,h.GetEntries()/50,h.GetEntries())#Limite on amplitude
    f.SetParameter(0,h.GetEntries()/5)#Starting amplitude
    f.SetParameter(1,h.GetMean())#Starting gaussian center
#    h.Fit("f","Q")
    return f



#DIFF section


#This function returns a histogram of a value(pos1) - value(pos2) vs ch
def diff_pos_vsCH(d,value,pos1,pos2):
    h_vs_ch = ROOT.TH1F("h_vs_ch","#Delta"+value + " vs Channel, " + "pos:("+str(pos1) +","+ str(pos2) +")",len(d),0,len(d))
    for i in d.items():
        try:
            h_vs_ch.Fill(i[0],i[1][pos1][value] - i[1][pos2][value])
        except :
            if i[1] not in range(95,126) and (pos1 == 0 or pos2 == 0):
                print 'skipping channel' + str(i[1]) +" and fill with 0"
            h_vs_ch.Fill(i[0],0)
    
    h_vs_ch.SetXTitle("Channel")
    h_vs_ch.SetYTitle("#Delta"+str(value))
    return h_vs_ch   


#This function returns a distribution histogram of value(pos1) - value(pos2)
def diff_pos_dist(d,value,pos1, pos2):
    h_d = ROOT.TH1F("h_d","#Delta"+value + " distribution, " + "pos=(%s,%s)"%(str(pos1),str(pos2)),50,-5,+5)
    for i in d.items():
        try:
            if i[1][pos1][value]==0 or i[1][pos2][value]==0 :
                print  'skipping channel ' + str(i[1]) + ' because value = 0'
                continue
            h_d.Fill(i[1][pos1][value] - i[1][pos2][value])
        except :
            if i[1] not in range(95,126) and (pos1 == 0 or pos2 == 0):
                print 'skipping channel ' + str(i[1]) + ' because error in getting the data (probably no data at all)'
    h_d.SetXTitle("#Delta"+str(value))
    h_d.SetYTitle("Entries")
        
    return h_d

