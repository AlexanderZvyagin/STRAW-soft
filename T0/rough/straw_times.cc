#include <map>
#include <string>
#include <cstdlib>

#include "DaqEventsManager.h"
#include "ChipF1.h"
#include "utils.h"

#include "TNtuple.h"
#include "TFile.h"

using namespace std;
using namespace CS;

extern "C"
{
void ddd_call(const DaqEventsManager &manager);
}

namespace {
vector<TFile*> files;

void files_save(void)
{
    printf("Saving the straw_times files...\n");
    for( vector<TFile*>::iterator f=files.begin(); f!=files.end(); f++ )
    {
        (**f).Purge();
        (**f).Write();
        (**f).Close();
        delete *f;
    }
    printf("Saving the straw_times files has finished!\n");
}

}

void ddd_call(const DaqEventsManager &manager)
{
    static bool init=true;
    static string prefix;
    if(init)
    {
        init=false;
        if( 0!=atexit(files_save) )
        {
            printf("straw_times: Can not set atexit function!\n");
            exit(75);
        }
        
        const char *env_var = "STRAW_TIMES_FILES";
        char *s=getenv(env_var);
        printf("Environment variable %s=%s\n",env_var,s);
        if( s==NULL )
            printf("The files will be created in the current directory.\n");
        else
        {
            prefix=s;
            if( prefix[prefix.length()-1] != '/' )
                prefix += '/';
        }
    }

    map<ChipF1::DataID,pair<int,int> > detector_channel;
    map<ChipF1::DataID,float> t1;       // TDC spectrum for a first time measurement
    map<ChipF1::DataID,float> mhdt;     // Multihit distribution
    map<ChipF1::DataID,string> det;

    typedef multimap<CS::DetID,CS::Chip::Digit*>::const_iterator m_it; // iterator type

    // Loop on all found digits
    for( m_it d_it=manager.GetEventDigits().begin(); d_it!=manager.GetEventDigits().end(); d_it++ )
    {
        const ChipF1::Digit *digit = dynamic_cast<const ChipF1::Digit *>(d_it->second);
        if( digit==NULL )
            continue;

        if( !string_match(d_it->first.GetName(),"ST.*") )
            continue;

        // The digit source
        ChipF1::DataID id(digit->GetDataID());

        // Get the detector name
        const string &name=digit->GetDetID().GetName();

        // remember detector <--> DataID association
        det[id]=name;

        if( t1.count(id)==0 )
        {
            t1[id]=digit->GetTimeDecoded(); // time in [ns]
            detector_channel[id]=pair<int,int>(digit->GetChannel(),digit->GetChannelPos());
        }
        else
            if( mhdt.count(id)==0 )
                mhdt[id]=digit->GetTimeDecoded()-t1[id];
    }

    for( map<ChipF1::DataID,float>::const_iterator it=t1.begin(); it!=t1.end(); it++ )
    {
        const ChipF1::DataID &id = it->first;

        // Find the ntuple
        assert(det[id].length()==8);
        const char *nt_name = det[id].c_str();
        static map<string,TNtuple*> ntuples;
        TNtuple * &ntuple = ntuples[nt_name];
        if( ntuple==NULL )
        {
            //printf("%s\n",nt_name);
            TDirectory *dir_save=gDirectory;
            
            TFile *f = TFile::Open((prefix+det[id]+".root").c_str(),"UPDATE");
            files.push_back(f);
            ntuple = dynamic_cast<TNtuple*>(f->Get(nt_name));
            if( ntuple==NULL )
                ntuple = new TNtuple(nt_name,nt_name,"catch:card:ch:chp:ech:t1:t2:mhdt");
            
            dir_save->cd();
        }

        ChipF1::DataID id2=id;
        id2.u.s.chip_chan++;
        assert((id2.u.s.chip_chan-id.u.s.chip_chan)==1);
        map<ChipF1::DataID,float>::const_iterator it_t2 = t1.find(id2);
        float time2 = it_t2==t1.end()?0:it_t2->second;
        int
            card   = (unsigned int)(id.u.s.geoID_or_port),
            catch_ = (unsigned int)(id.u.s.src_id);

        // For the debug mode use non-positive numbers
        if( id.u.s.mode==0 )
            card = -card;

        ntuple->Fill(catch_, card, detector_channel[id].first,detector_channel[id].second,
                     id.u.s.chip_chan,t1[id],time2,mhdt[id]);
    }
}
