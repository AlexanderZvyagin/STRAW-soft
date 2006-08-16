import sys, optparse, os, re

def change_line(line,settings):
    r = re.match('(?P<channel>.*);(?P<alias>.*);(?P<V0>.*);(?P<I0>.*);(?P<V_max>.*);(?P<ramp_speed>.*)',line)
    if r==None:
        return line

    #print r.group('channel'), r.group('alias'), r.group('V0'), r.group('I0'), r.group('V_max'), r.group('ramp_speed')
    
    channel = r.group('alias')
    
    if None!=re.match('Notused\d*',channel):
        return line

    r2 = re.match('St_Hv_Dl(?P<DL>\d+)_(?P<straw>\d\d.\d)_(?P<diameter>\d+)m{2,3}_(?P<subname>.*)',channel)
    if r2==None:
        print 'Unrecognized name (no changes for it):',channel
        return line
    else:
        diameter = int(r2.group('diameter'))
        if diameter==6:
            v     = settings.HV_6mm
            v_max = settings.HV_6mm_max
        elif diameter==10:
            v     = settings.HV_10mm
            v_max = settings.HV_10mm_max
        else:
            raise 'Unknown straw diameter %d' % diameter
        l = r.group('channel')+';'
        l+= r.group('alias')+';'
        l+= str(v)+';'
        l+= r.group('I0')+';'
        l+= str(v_max)+';'
        l+=r.group('ramp_speed')
        l+='\n'
        return l
    raise 'Internal problem!'

def change_config(fname,new_name,settings):
    f = file(new_name,'w')
    for line in os.popen('cat %s' % fname):
        new_line = change_line(line,settings)    
        f.write(new_line)

if __name__=='__main__':

    parser = optparse.OptionParser()

    parser.usage = '%prog [options] <input-config.Straws> <output-config.Straws>'
    parser.description = 'Change STRAWs DCS config files for HV settings.'

    parser.add_option('', '--HV-6mm',dest='HV_6mm',default=0,
                      help='High voltage for 6mm straws, [default: %default]', type='int')
    parser.add_option('', '--HV-10mm',dest='HV_10mm',default=0,
                      help='High voltage for 10mm straws, [default: %default]', type='int')
    parser.add_option('', '--HV-6mm-max',dest='HV_6mm_max',default=2500,
                      help='Maximum high voltage for 6mm straws, [default: %default]', type='int')
    parser.add_option('', '--HV-10mm-max',dest='HV_10mm_max',default=2500,
                      help='Maximum high voltage for 10mm straws, [default: %default]', type='int')

    (options, args) = parser.parse_args()

    if len(args)!=2:
        parser.print_help()
        sys.exit(1)

    change_config(args[0],args[1],options)
