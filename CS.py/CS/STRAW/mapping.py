##  @addtogroup STRAW
##  @{

import xml.dom.minidom
import re

##  @brief Detector mapping.
#
class Map:
    names={}

    ##  @brief Initialization
    #
    def __init__(self):
        self.detector   = ''
        self.srcID      = -1
        self.port       = -1
        self.geoID      = -1
        self.chanF      = 0
        self.chanS      = 0
        self.chanN      = 0
        self.wireF      = 0
        self.wireL      = 0
        self.wireS      = 0
        self.wireP      = 0

        if len(Map.names)==0:
            Map.names[(0,31)]='start 1'

    def number(self):
        return {0:1,32:2,64:3,80:3,96:4,112:4,128:5,160:6,95:4,127:5,158:6,190:7}[self.wireF]

    ##  @brief Make a nice name.
    #
    def __str__(self):
        #return '%s [%d,%d]' % (self.detector,self.wireF,self.wireL)

        mm = ('10mm',' 6mm')[self.detector[7]=='b']
        number = self.number()

        if self.wireP>0:
            res = 'PH'
        else:
            if self.detector[4]=='Y':
                side = ('B','T')[self.detector[7]=='c']
                if side=='T':
                    number = 3-number
            else:
                side = ('S','J')[self.detector[7]=='c']
                if side=='J':
                    number = 4-number
            res = '%s%d' % (side,number)
        
        name = '%s-%s-%s' % (self.detector[3:6],mm,res)
        
        #return '%s   %s [%d,%d]' % (name,self.detector,self.wireF,self.wireL)
        return name

##  @internal
#
def map_line_analyse(data,mapping):
    d = data.strip()
    if not d:
        return

    # name  srcID port geoID chanF chanS chanN  wireF wireL wireS [wireP]
    r = re.match('\s*'    # Some space
                 '(?P<detector>\w+)\s*'
                 '(?P<src>\d+)\s*'
                 '(?P<port>\d+)\s*'
                 '(?P<geo>[\dxXABCDEFabcdef]+)\s*'
                 '(?P<chanF>\d+)\s*'
                 '(?P<chanS>-?\d+)\s*'
                 '(?P<chanN>\d+)\s*'
                 '(?P<wireF>\d+)\s*'
                 '(?P<wireL>\d+)\s*'
                 '(?P<wireS>-?\d+)\s*'
                 '((?P<wireP>[-+]?\d+)\s*)?'
                 ,d)
    if not r:
        print 'Unrecognized line:\n',d
        return

    m = Map()
    m.detector  = r.group('detector')
    m.srcID     = int(r.group('src'))
    m.port      = int(r.group('port'))
    base = (10,16)[r.group('geo')[:2]=='0x']    # checking for a 0x prefix
    m.geoID     = int(r.group('geo'),base)
    m.chanF     = int(r.group('chanF'))
    m.chanS     = int(r.group('chanS'))
    m.chanN     = int(r.group('chanN'))
    m.wireF     = int(r.group('wireF'))
    m.wireL     = int(r.group('wireL'))
    m.wireS     = int(r.group('wireS'))
    m.wireP     = r.group('wireP')
    if m.wireP:
        m.wireP = int(m.wireP)
    else:
        m.wireP = 0
    
    if m.wireF>m.wireL:
        m.wireF, m.wireL = (m.wireL, m.wireF)
    
    key = '%d %d' % (m.srcID,m.port)
    mapping[key] = m

##  @internal
#
def handle_ChipF1(m,mapping):
    for node in m.childNodes:
        if node.nodeType == node.TEXT_NODE:
            for data in node.data.split('\n'):
                map_line_analyse(data,mapping)

##  @internal
#
#   @arg \b m XML-mapping after xml.dom.minidom.parse(map_file)
#   @arg \b run Run number
#   @arg \b mapping dictionary to be filled.
#
def handle_Map(m,run,mapping):
    for element in m.getElementsByTagName('ChipF1'):
        #runs = element.getAttribute('runs')
        handle_ChipF1(element,mapping)

##  @brief Read a map file
#
#   @return Dictionary with a mapping.
#
def read_mapping(map_file,run):
    full_map_file = xml.dom.minidom.parse(map_file)
    mapping = {}
    handle_Map(full_map_file,run,mapping)
    return mapping

if __name__=='__main__':
    read_mapping('STRAW.xml',44444)

##  @}
