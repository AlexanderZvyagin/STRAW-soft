import re,copy
import ROOT
from   ROOT import gStyle, kWhite

##  Class to handle pictures printing
#
class Plot:
    ##  Constructor
    #
    #   @arg \c ext     Picture type (\a ps,\a eps,\a gif)
    #   @arg \c size    Picture size, ex: '200x300'
    #
    def __init__(self,ext='eps',size='400x300'):
        self.ext  = ext
        self.name = ''
        
        csize = re.match('(?P<x>\d+)x(?P<y>\d+)',size)
        self.size_x = int(csize.group('x'))
        self.size_y = int(csize.group('y'))
    
    ##  Make a picture
    #
    #   @arg \c canvas Object to be printed.
    #   @arg \c ext Use this picture type instead of the constructor's one
    #
    def make(self,canvas,ext=None):
        if not ext:
            ext = self.ext
        canvas.Print(self.name,'.'+ext)

##  ROOT style manipulation
#
class RootStyle:
    ##  Constructor
    #
    #   @arg \c rstyle Either RootStyle or ROOT.TStyle object.
    #
    def __init__(self,rstyle):

        if rstyle.__class__==RootStyle:
            self = copy.deepcopy(rstyle)
        else:
            ## Dictionary for all style attributes to be saved.
            self.rstyle_attrs = {}

            for c in dir(rstyle):
                if c[:3]!='Get':
                    continue    # Ignore all function calls which start without "Get"

                try:
                    self.rstyle_attrs[c[3:]] = rstyle.__getattribute__(c)()
                except:
                    #print 'RootStyle failed on', c
                    pass
    ##  Set the style
    #
    #   @arg \c rstysle Change the style of this object.
    #
    def set(self,rstysle):
        for name,value in self.rstyle_attrs.items():
            try:
                rstysle.__getattribute__('Set'+name)(value)
            except:
                #print 'set: RootStyle failed on', name
                pass

    def __setitem__(self,key,item):
        self.rstyle_attrs[key] = item

    def __getitem__(self,key):
        return self.rstyle_attrs[key]

## Class to help to draw a ROOT objects (TH1,TH2,TGraph*)
#
class DrawObj:

    ## Main constructor
    #
    #   @arg \c obj Object to be drawn
    #
    #   For default arguments it takes the default style
    #
    def __init__(self,obj,style=None):

        if not style:
            style = RootStyle(ROOT.gStyle)

        self.obj = obj
        self.draw_opts = ''
        self.style = copy.deepcopy(style)
        self.calls_before=[]
        self.calls_after=[]

    ##  Register a function which should be called after/before a real 'drawing'.
    #
    #   @arg \c when \b 0 for calling before and \b 1 for calling after the 'drawing'.
    #   @arg \c func Function to be called
    #   @arg \c List of function arguments
    #
    def call(self,when,func,args):
        if when==0:
            l = self.calls_before
        elif when==1:
            l = self.calls_after
        else:
            raise 'Bad argument'
        
        l.append( (func,args) )
            
    ## Set options before draw.
    #
    def before_draw(self):
        self.style_save = RootStyle(ROOT.gStyle)
        self.style.set(ROOT.gStyle)

        for f in self.calls_before:
            apply(f[0],f[1])

        ROOT.gROOT.ForceStyle()
        #try:
        #    self.obj.UseCurrentStyle()
        #except:
        #    pass

    ##  Set options after draw and a canvas update.
    #
    def after_draw(self):
        for f in self.calls_after:
            apply(f[0],f[1])

        self.style_save.set(ROOT.gStyle)
        del self.style_save

        ROOT.gROOT.ForceStyle()

    ##  Compatability call with ROOT
    #
    def Draw(self,opts=''):
        self.draw(opts)
    
    ##  Draw an object
    #
    #   First, set graphics parameters, then draw.
    def draw(self,opts=''):
        if not opts:
            opts = self.draw_opts

        self.before_draw()
        self.obj.Draw(opts)
        self.after_draw()
        
        return ROOT.gPad
    
    ## Return the object itself
    def __call__(self):
        return self.obj

    def __setitem__(self,key,item):
        self.style[key] = item

    def __getitem__(self,key):
        return self.style[key]

##  Set default ROOT style
#
def set_default():
    gStyle.SetOptDate(0)
    gStyle.SetOptFit(1)
    gStyle.SetStatFormat('8.2g')
    gStyle.SetFitFormat('8.2g')
    gStyle.SetCanvasColor(kWhite)
    gStyle.SetStatColor(kWhite)
    #gStyle.SetTitleColor(kWhite)
    gStyle.SetTitleFillColor(kWhite)
    gStyle.SetOptTitle(False)               # Latex will take care of it!

##  Define comand line parameters
#
def optparse_parameters(parser,default_ext='.eps',default_size='400x300'):
    parser.add_option('', '--pic-name',dest='pic_name',
                      help='Picture name [default: the code choice]', type='string',metavar='NAME')
    parser.add_option('', '--pic-ext',dest='pic_ext',default=default_ext,
                      help='Picture type [default: "%default"]', type='string',metavar='NAME')
    parser.add_option('', '--pic-size',dest='pic_size',default=default_size,
                      help='Picture size [default: "%default"]', type='string',metavar='NAME')
    parser.add_option('', '--root',dest='root_file',
                      help='Put histograms to the root file', type='string',metavar='PATH')
    parser.add_option('', '--root-dir',dest='root_dir',default='',
                      help='Put files to this subdirectory of the root file',
                      type='string',metavar='PATH')

##  Save objects to a root file.
#   
def save_to_root(objs,root_file,root_dir=''):
    f = ROOT.TFile(root_file,'UPDATE','',9)
    if not f.IsOpen():
        raise 'Failed to open the file %s' % root_file
    
    if root_dir not in ['','.','./']:
        for d in root_dir.split('/'):
            if not d:
                continue
            if not ROOT.gDirectory.GetKey(d):
                if not ROOT.gDirectory.mkdir(d):
                    raise 'Can not create a directory!'
            if not ROOT.gDirectory.cd(d):
                raise 'Can not change to a directory!'

    for o in objs:
        if o.__class__ == DrawObj:
            root_obj = o()
        else:
            root_obj = o

        root_obj.Write()

    f.Write()
    f.Close()

#if __name__=='__main__':
def test():
    from ROOT import TH1F
    
    h = []
    
    print 'start1:',ROOT.gStyle.GetTitleX()
    ROOT.gStyle.SetCanvasColor(ROOT.kWhite)

    h.append( DrawObj( TH1F('h%d' % (len(h)+1),'The title',100,0,1) ) )
    print 'start2:',ROOT.gStyle.GetTitleX()
    h[-1].obj.GetXaxis().SetTitle('aaaaaa')

    print 'Style change!'
    ROOT.gStyle.SetTitleX(0.4)

    h.append( DrawObj( TH1F('h%d' % (len(h)+1),'The title',100,0,1) ) )
    h.append( DrawObj( TH1F('h%d' % (len(h)+1),'The title',100,0,1) ) )
    
    c = ROOT.TCanvas('c','c')
    c.Divide(4,1)
    for hh in h:
        c.cd(h.index(hh)+1)
        hh.draw()

    c.cd(0)
    c.Print('all.ps','.ps')
