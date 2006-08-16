#include "TROOT.h"
#include "TH2.h"
#include "TFile.h"

#include "StrawTubes.h"

using namespace CS;
using namespace std;

TROOT root("","");

void fill(const StrawTubes &stdc)
{
    char name[222];

    sprintf(name,"%s_Xcor0",stdc.GetName().c_str());
    TH1F *h1 = new TH1F(name,"STRAW signal propagation update",222,0,222);

    sprintf(name,"%s_Xcor",stdc.GetName().c_str());
    TH2F *h2 = new TH2F(name,"X-ray correction",222,0,222,200,-200,200);
    
    for( int ch=0; ch<222; ch++ )
    {
        const CS::DriftDetectorChannel *channel=stdc.FindChannel(short(ch),0);

        float channel_start = -(stdc.GetChannelsN()-1.)/2*stdc.GetPitchAvr();
        double wi=channel_start + ch*stdc.GetPitchAvr();

        if( channel!=NULL )
            try
            {
                float du_sp = channel->GetCorrSpacersY(0)-wi;
                
                //printf("%s ch=%d start=%g  cor=%g du_sp=%g\n",
                //        stdc.GetName().c_str(),ch,channel_start,channel->GetCorrSpacersY(0),du_sp);

                h1->Fill(ch,du_sp);
            }
            catch(...)
            {
            }
        else
            try
            {
                const CS::DriftDetectorChannel
                    *ch_d=stdc.FindChannel(short(ch),-1),
                    *ch_u=stdc.FindChannel(short(ch), 1);
                if( ch_u==NULL || ch_d==NULL )
                    continue;
                float du_sp = ((ch_u->GetCorrSpacersY(15)-wi)+(ch_d->GetCorrSpacersY(-15)-wi))/2;
                h1->Fill(ch,du_sp);
            }
            catch(...)
            {
            }

            for( float y=-200; y<200; y+=1 )
            {
                if( channel==NULL )
                    channel = stdc.FindChannel(short(ch),y<0?-1:1);

                if( channel==NULL )
                    break;

                try
                {
                    float du_sp = channel->GetCorrSpacersY(0)-wi;
                    h2->SetBinContent(h2->FindBin(ch,y),du_sp);
                }
                catch(...)
                {
                }
            }
    }
}

int main(int argc,char **argv)
{
    try
    {
        CS::Detectors dets;
        dets.ReadDetectorsXML("calib.xml");
        
        TFile f("x.root","RECREATE","",9);
        
        for( CS::Detectors::const_iterator d=dets.begin(); d!=dets.end(); d++ )
        {
            const CS::StrawTubes *st=dynamic_cast<const CS::StrawTubes*>(*d);
            assert(st!=NULL);
            fill(*st);
        }
        
        f.Write();
    }

    // Something is wrong...
    // Print error message and finish this data file.

    catch( const std::exception &e )
    {
        cerr << "exception:\n" << e.what() << "\n";
    }
    catch( const char *s )
    {
        cerr << s << "\n";
    }
    catch( ... )
    {
        cerr << "Oops, unknown exception!\n";
    }

    return 0;
}
