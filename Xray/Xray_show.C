#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

#include "TCanvas.h"
#include "TLine.h"
#include "TH1.h"
#include "TGraphErrors.h"
#include "TGaxis.h"

using namespace std;

class XrayCalib
{
  public:
    XrayCalib(const string &det) : detector(det) {}
    void read(const std::string &st);
    void read_file(const std::string &name);
    /// Return the X-ray correction in [mm]
    float xray_correction(int channel,float y) const;
    void make_pictures(const string &stdc);
  public:
    string detector;
    vector<float> spacers_pos;      // coordinates of spacers
    map<int,vector<float> >  xray;  // Channel number --> List of spacer corrections.
};

float XrayCalib::xray_correction(int channel,float y) const
{
    // First of all we have to find the channel
    map<int,vector<float> >::const_iterator ch=xray.find(channel);
    if( ch==xray.end() )
        throw "XrayCalib::xray_correction(): channel was not found.";
    if( y<spacers_pos.front() )
        return ch->second.front();
    if( y>spacers_pos.back() )
        return ch->second.back();
    
    for( size_t i=1; i<spacers_pos.size(); i++ )
        if( spacers_pos[i-1]<=y && spacers_pos[i]>=y )
        {
            float dy=y-spacers_pos[i-1];
            float c = ch->second[i] - ch->second[i-1];
            //printf("==>> dy=%g\n",dy/(spacers_pos[i]-spacers_pos[i-1]));
            return ch->second[i-1] + c * dy/(spacers_pos[i]-spacers_pos[i-1]);
        }
    throw "XrayCalib::xray_correction(): internal problem";
}

void XrayCalib::read_file(const std::string &file)
{
    char *buf=new char[1000000];
    
    ifstream f(file.c_str());
    f.read(buf,1000000);
    //printf("%d\n",strlen(buf));
    
    read(buf);
    delete [] buf;
}

void XrayCalib::read(const std::string &data)
{
    istringstream is(data);

    spacers_pos=vector<float>(6,0);
    string line;
    char buf[22];

    getline(is,line);
    if( !is || 7!=sscanf(line.c_str(),"%s %g %g %g %g %g %g",buf,
           &spacers_pos[0],&spacers_pos[1],&spacers_pos[2],
           &spacers_pos[3],&spacers_pos[4],&spacers_pos[5]) )
        throw "Failed to read STRAW X-ray calibrations!";

    int ch=0;
    while( getline(is,line) )
    {
        float x[6];
        sscanf(line.c_str(),"%g %g %g %g %g %g",&x[0],&x[1],&x[2],&x[3],&x[4],&x[5]);
        xray[ch++] = vector<float>(x,x+6);
    }
}

void xray2graph(XrayCalib &xray)
{
    int chan_start=0, chan_tot=0, chan_N=0, sec_a=0, sec_c=0;

    if( xray.detector[4]=='Y' )
    {
        chan_tot = 64+192+64;
        sec_a=64;
        sec_c=64+192;
        if( xray.detector[7]=='a' )
        {
            chan_N = 64;
            chan_start = 0;
        }
        else
        if( xray.detector[7]=='b' )
        {
            chan_N = 192;
            chan_start = 64;
        }
        else
        if( xray.detector[7]=='c' )
        {
            chan_N = 64;
            chan_start = 64+192;
        }
        else
            throw "bad 'abc' secton name";
    }
    else
    {
        chan_tot = 96+222+96;
        sec_a=96;
        sec_c=96+222;
        if( xray.detector[7]=='a' )
        {
            chan_N = 96;
            chan_start = 0;
        }
        else
        if( xray.detector[7]=='b' )
        {
            chan_N = 222;
            chan_start = 96;
        }
        else
        if( xray.detector[7]=='c' )
        {
            chan_N = 96;
            chan_start = 96+222;
        }
        else
            throw "bad 'abc' secton name";
    }

    //printf("%s   %d %d\n",xray.detector.c_str(),chan_N,xray.xray.size());
    assert( chan_N==xray.xray.size() );
    
    static map<string,TCanvas*> canvases;
    static map<string,TH1F*>    frames;
    string det=xray.detector.substr(0,7);
    double k=50;
    
    TCanvas *&c = canvases[det];
    TH1F *&h = frames[det];
    if( c==NULL )
    {
        c = new TCanvas(det.c_str());
        assert(h==NULL);
        h = new TH1F(det.c_str(),det.c_str(),chan_tot,0,chan_tot);
        h->GetXaxis()->SetTitle("Wire number");
        h->GetYaxis()->SetTitle("Vertical position [cm]");
        float
            y_min = xray.spacers_pos.front()/10-20,
            y_max = xray.spacers_pos.back ()/10+20;
        h->SetMinimum(y_min);
        h->SetMaximum(y_max);
        h->Draw();
        (new TLine(sec_a,y_min,sec_a,y_max)) -> Draw();
        (new TLine(sec_c,y_min,sec_c,y_max)) -> Draw();
        
        for( size_t sp=0; sp<6; sp++ )
        {
            float y=xray.spacers_pos[sp]/10;
            if(0)
            {
                (new TLine(0,y-0.1*k,chan_tot,y-0.1*k)) -> Draw();
                (new TLine(0,y+0.1*k,chan_tot,y+0.1*k)) -> Draw();
            }
            else
            {
                (new TLine(0,y,chan_tot,y)) -> Draw();
            }
            
            if( sp==4 )
            {
                float length=0.5;
                TGaxis *axis = new TGaxis(chan_tot+8,y,chan_tot+8,y+length*k, 0, length, 200,"+L");
                axis->SetTitle("     500 #mu m");
                axis->SetTitleOffset(0.5);
                axis->Draw();
            }
        }        
    }
    

    for( int ch=0; ch<chan_N; ch++ )
    {
        //printf("%s ch=%d\n",xray.detector.c_str(),ch);
        assert(xray.spacers_pos.size()==6);
        assert(xray.xray[ch].size()==6);
        
        for( size_t sp=0; sp<6; sp++ )
        {
            float y = xray.spacers_pos[sp];
            //printf("y=%g\n",y);
            double cor=xray.xray_correction(ch,y);
            y/=10;
            //printf("%d %g %d %g\n",chan_start+ch,y-fabs(cor)/2,chan_start+ch,y+fabs(cor)/2);
            //printf("%d  %g   %g\n",ch,cor,xray.xray[ch][sp]);
            cor *= k;
            TLine *line=NULL;
            if(0)
            {
                line = new TLine(chan_start+ch,y-fabs(cor)/2,chan_start+ch,y+fabs(cor)/2);
                line->SetLineColor(cor<0?kRed:kBlue);
            }
            else
            {
                line = new TLine(chan_start+ch,y,chan_start+ch,y+cor);
            }
            line->Draw();
        }
        
    }
        
    c->Print("","eps");
}

void fill_hist(XrayCalib &xray,int spacer)
{
    assert( abs(spacer)<=3 );

    char name[44];
    sprintf(name,"%s_xray_",xray.detector.c_str());
    if( spacer==0 )
        sprintf(name+strlen(name),"0");
    else
        sprintf(name+strlen(name),"%c%d",spacer>0?'p':'m',abs(spacer));

    TH1F *h=new TH1F(name,"X-ray correction",xray.xray.size(),0,xray.xray.size());
    h->SetMinimum(-500);
    h->SetMaximum( 500);
    h->GetXaxis()->SetTitle("Channel number");
    h->GetYaxis()->SetTitle("X-ray correction, #mum");

    for(int ch=0; ch<xray.xray.size(); ch++ )
        h->SetBinContent(ch+1,1000*xray.xray_correction(ch,0));

    for(int ch=0; ch<xray.xray.size(); ch++ )
    {
        assert(xray.xray[ch].size()==6);
        double cor=0;
        if( spacer==0 )
            cor = (xray.xray[ch][2]+xray.xray[ch][3])/2;
        else
            cor = xray.xray[ch][spacer+3-(spacer>0)];
        
        h->SetBinContent(ch+1,1000*cor);
    }
}

void fill_hist(XrayCalib &xray)
{
    for( int spacer=-3; spacer<=3; spacer++ )
        fill_hist(xray,spacer);
}


void Xray_show(void)
{
    for( int st=3; st<=6; st++ )
        for( int xyuv=0; xyuv<4; xyuv++ )
            for( int n=1; n<=2; n++ )
                for( int ud=0; ud<2; ud++ )
                    for( char abc='a'; abc<='c'; abc++ )
                    {
                        char name[9];
                        sprintf(name,"ST0%d%c%d%c%c",st,"XYUV"[xyuv],n,"ud"[ud],abc);
                        try
                        {
                            XrayCalib x(name);
                            x.read_file(string("1/")+name);
                            xray2graph(x);
                            
                            fill_hist(x);
                        }
                        catch(...)
                        {
                        }
                    }
}

#include "TStyle.h"
#include "TRint.h"
TROOT root("","");

int main(int argc,char **argv)
{
    try
    {
        TRint theApp("App", &argc, const_cast<char**>(argv) );
        gStyle->SetCanvasColor(kWhite);
        gStyle->SetStatColor(kWhite);
        gStyle->SetTitleColor(kWhite);
        gStyle->SetOptStat(0);
        gStyle->SetOptDate(0);
        gStyle->SetOptTitle(0);
        TFile f("xray-delta.root","RECREATE","",9);
    
        Xray_show();
        
        f.Write();
        return 0;
    }
    catch( const char *s )
    {
        printf("%s\n",s);
    }
    catch( ... )
    {
        printf("Unknown exception!\n");
    }
    
    return 1;
}
