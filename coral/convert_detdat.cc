#include "TROOT.h"
#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#undef Check

#include <cassert>
#include <iostream>             // C++ I/O
#include <fstream>              // C++ files I/O

#include "ObjectXML.h"
#include "DaqOption.h"
#include "ChipF1.h"

#include "RTRelationGrid.h"
#include "DriftDetector.h"
#include "StrawTubes.h"

// -----------------------------------------------------------------------------

using namespace CS;
using namespace std;

namespace CS {
bool string_match(const string &str, const string &pattern);
}

TROOT root("","");

// -----------------------------------------------------------------------------

struct MissingT0
{
    DriftDetectorChannel *drift_chan;
    int geoID;
    int ech;
};

// -----------------------------------------------------------------------------

pair<int,int> get_geoID_ech(const Chip::Maps &maps_of_the_run,const string &det,int ch,int chp)
{
    for( Chip::Maps::const_iterator it=maps_of_the_run.begin(); it!=maps_of_the_run.end(); it++ )
    {
        if( !string_match(it->second->GetDetID().GetName(),det) )
            continue;
        
        const ChipF1::Digit *d_f1=dynamic_cast<const ChipF1::Digit *>(it->second);
        if( NULL!=d_f1 )
        {
            if( d_f1->GetChannel()==ch && d_f1->GetChannelPos()==chp )
            {
                ChipF1::DataID id = d_f1->GetDataID();
                if( id.u.s.mode==1 )
                    return pair<int,int>(id.u.s.geoID_or_port,id.u.s.chip_chan);
            }
        }
    }
    
    throw "get_geoID_ech(): not found!";
}

// -----------------------------------------------------------------------------

// Program start point
int main(int argc,char **argv)
{
    if( argc!=7 )
    {
        cout << "Usage: <exe> run db_entry_time maps.xml detectors.dat  geom.xml extension\n";
        return 1;
    }

    try
    {
        // Read options.
        int run = atoi(argv[1]);
        string
            entry_time      = argv[2],
            maps_dir        = argv[3],
            detectors_dat   = argv[4],
            geom_xml        = argv[5],
            extension       = argv[6];

        // Read map files
        Chip::Maps maps_of_the_run;
        DaqOption opts_of_the_run;
        vector<string> detectors_all;
        Chip::ReadMaps( run, maps_dir, maps_of_the_run, opts_of_the_run, detectors_all );
    
        // Read detectors.dat file
        Detectors dets;
        dets.ReadDetectorsDAT(detectors_dat);

        // This concerns only drift-time detectors.
        // Now we need to get for every channel (more precise: every geoID) the RT.
        // RT is available in the DB (na58pc052.cern.ch). It is mainly the table
        // detector:geoID:T0
        
        // For every channel of a detector we have to ask the map files (DaqDataDecdoing)
        // about geoID to which the channel belongs.

        // So the logic is:
        // - loop on detecors
        // - loop on detector channel
        // - ask map files about geoID for the channel (run number is needed as well)
        // - aks the DB about T0
        // - apply T0 to the channel

        // FIXME: username, password
        TSQLServer *db = TSQLServer::Connect("mysql://na58pc052.cern.ch","zvyagin","HMcheops");
        
        char buf[111];
        sprintf(buf,"SELECT * FROM cdb.STRAW_T0 WHERE entry='%s'",entry_time.c_str());

        TSQLResult *r = db->Query(buf);
        if( r==NULL )
            throw "Failed to select from the DB.";

        printf("Selected: %d\n",r->GetRowCount());
        if( r->GetRowCount()<=0 )
            throw "Nothing selected from the DB!";

        int
            fileds    = r->GetFieldCount(),
            f_run     = -1,
            f_layer   = -1,
            f_geoID   = -1,
            f_ech     = -1,
            f_T0      = -1,
            f_T0e     = -1,
            f_T0fit   = -1;

        for( int i=0; i<fileds; i++ )
        {
            if( 0==strcmp(r->GetFieldName(i),"run") )
                f_run = i;

            if( 0==strcmp(r->GetFieldName(i),"layer") )
                f_layer = i;

            if( 0==strcmp(r->GetFieldName(i),"geoID") )
                f_geoID = i;

            if( 0==strcmp(r->GetFieldName(i),"ech") )
                f_ech = i;

            if( 0==strcmp(r->GetFieldName(i),"T0") )
                f_T0 = i;

            if( 0==strcmp(r->GetFieldName(i),"T0e") )
                f_T0e = i;

            if( 0==strcmp(r->GetFieldName(i),"fit") )
                f_T0fit = i;
        }
        
        if( f_run==-1 || f_layer==-1 || f_geoID==-1 || f_ech==-1 || f_T0==-1 || f_T0e==-1 || f_T0fit==-1 )
            throw "Can not determinate fileds.";

        map<int, map<int,double> > geoID_ech_T0;
        map<int,string> geoID_layer;

        multimap<int,MissingT0> missing_T0;  // missing T0 for geoID

        // Loop on all found entries
        for( TSQLRow *rr=NULL; NULL!=(rr=r->Next()); )
        {
            double T0,T0e,T0fit;
            int run2, geoID, ech;
            char layer[22];

            if( 1!=sscanf(rr->GetField(f_run),"%d",&run2) )
                throw "bad run format";
            
            if( run2!=run )
                throw "Wrong run number.";

            if( 1!=sscanf(rr->GetField(f_layer),"%s",layer) )
                throw "bad layer fromat";

            if( 1!=sscanf(rr->GetField(f_geoID),"%d",&geoID) )
                throw "bad geoID format";

            if( 1!=sscanf(rr->GetField(f_ech),"%d",&ech) )
                throw "bad ech format";

            if( 1!=sscanf(rr->GetField(f_T0),"%lg",&T0) )
                throw "Hodoscope::Read(): bad T0 format";

            if( 1!=sscanf(rr->GetField(f_T0e),"%lg",&T0e) )
                throw "Hodoscope::Read(): bad T0e format";

            if( 1!=sscanf(rr->GetField(f_T0fit),"%lg",&T0fit) )
                throw "Hodoscope::Read(): bad T0fit format";

            map<int,double> &ech_T0 = geoID_ech_T0[geoID];
            ech_T0[ech] = T0;
            geoID_layer [geoID] = layer;
            
            //printf("geoID=%d ech=%d   T0=%g\n",geoID,ech,T0);
        }

        RTRelationGrid
            rt6 ("RT-Grid 0:0 0.017:6.7 0.037:11.2 0.068:15.7 0.147:24.7 0.205:30.2 0.303:39.2"),
            rt10("RT-Grid 0:0 0.017:6.7 0.037:11.2 0.068:15.7 0.147:24.7 0.205:30.2 0.303:38.7 3.83:46.7 4.55:54.7");

        //set<int> noT0;

        for( Detectors::iterator d=dets.begin(); d!=dets.end(); d++ )
        {
            DriftDetector *drift_det = dynamic_cast<DriftDetector*>(*d);
            if( drift_det==NULL )
                continue;
                
            if( dynamic_cast<StrawTubes*>(*d)==NULL )
                continue;

            printf("Detector %s\n",drift_det->GetName().c_str());
        
            //drift_det->Read(*db,entry_time,run);
        
            for( vector<DriftDetectorChannel>::iterator ch=drift_det->GetChannels().begin();
                 ch!=drift_det->GetChannels().end(); ch++ )
            {

                pair<int,int> geoID_ech;
                try
                {
                    geoID_ech = get_geoID_ech(maps_of_the_run,drift_det->GetName(),ch->GetChannel(),ch->GetChannelPos());
                }
                catch(...)
                {
                    printf("ERROR: %s: ch=%3d geoID=%3d\n",drift_det->GetName().c_str(),ch->GetChannel(),geoID_ech.first);
                }
                
                if( geoID_layer[geoID_ech.first]!="" )
                    if( drift_det->GetName().substr(0,6)!=geoID_layer[geoID_ech.first].substr(0,6) )
                        printf("bad name??? geoID=%d \"%s\"!=\"%s\"\n",geoID_ech.first, drift_det->GetName().c_str(),geoID_layer[geoID_ech.first].c_str());

                if( ch->GetRT()!=NULL )
                {
                    printf("%s ch=%d RT: %p\n",drift_det->GetName().c_str(),ch->GetChannel(),ch->GetRT());
                    ch->GetRT()->Print();
                }

                if( drift_det->GetName()[7]=='b' )
                    ch->SetRT(&rt6);
                else
                    ch->SetRT(&rt10);
//                 if( geoID_ech_T0[geoID]==0 && noT0.end()==noT0.find(geoID) )
//                 {
//                     noT0.insert(geoID);
//                     printf("T0 is absent for geo=%d\n",geoID);
//                 }
                if( geoID_ech_T0[geoID_ech.first][geoID_ech.second]==0 )
                {
                    //noT0.insert(geoID);
                    printf("%s: T0 is absent for geo=%d ech=%d\n",drift_det->GetName().c_str(),geoID_ech.first,geoID_ech.second);
                    MissingT0 m;
                    m.drift_chan = ch;
                    m.geoID      = geoID_ech.first;
                    m.ech        = geoID_ech.second;
                    missing_T0.insert( pair<int,MissingT0>(m.geoID,m) );
                }
                
                ch->GetRT()->SetT0(geoID_ech_T0[geoID_ech.first][geoID_ech.second]);

                // Set electronics position
                int sn = 1;
                
                if( drift_det->GetName()[3]>='5' )
                    sn = drift_det->GetName()[4]=='Y' ? 1:-1;
                
                ch->SetReadoutPosY(sn*15*ch->GetChannelPos());
                ch->SetSignalSpeed(18); // 18 cm/ns
            }
            
            void fix_missing_T0(map<int, map<int,double> > &geoID_ech_T0, const multimap<int,MissingT0> & missing_T0);
            
            fix_missing_T0(geoID_ech_T0,missing_T0);
        
            // -----------------
        
            if(1)
            {
                // Write everything!
                ofstream f((geom_xml+'/'+(*d)->GetName()+extension).c_str());
                if( !f.is_open() )
                    throw "Can not open file for writing the geometry";
                ObjectXML *xml=(*d)->CreateObjectXML();
                xml->Print(f);
                delete xml;
            }
        }

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

// -----------------------------------------------------------------------------

void drop_min_max_T0(map<int,double> &ech_T0)
{
    double min=0,max=0;
    int ch_min=-1,ch_max=-1;

    for( map<int,double>::iterator it=ech_T0.begin(); it!=ech_T0.end(); it++ )
        if( it->second!=0 )
            if( ch_min==-1 )
            {
                min=max=it->second;  // min/max is not known yet
                ch_min=ch_max=it->first;
            }
            else
            {
                if( min>it->second )
                {
                    min=it->second;
                    ch_min=it->first;
                }
                if( max<it->second )
                {
                    max=it->second;
                    ch_max=it->first;
                }
            }
    if( ch_min==-1 )
        throw "drop_min_max_T0(): all channels are empty!";
    
    ech_T0[ch_min]=0;
    ech_T0[ch_max]=0;
}


// -----------------------------------------------------------------------------

float calculate_average_T0(map<int,double> &ech_T0)
{
    float m0=0,m1=0,m2=0;
    
    for( map<int,double>::const_iterator it=ech_T0.begin(); it!=ech_T0.end(); it++ )
        if( it->second!=0 )
        {
            m0 += 1;
            m1 += it->second;
            m2 += it->second*it->second;
        }
    
    if( m0==0 )
        throw "calculate_average_T0(): too little channels";
    
    m1/=m0;
    m2 = std::sqrt(m2/m0 - m1*m1);
    
    if( m2>3 && m0>10 )
    {
        //for( map<int,double>::const_iterator it=ech_T0.begin(); it!=ech_T0.end(); it++ )
        //    printf("%d:%g ",it->first,it->second);
        //printf("\nT0_avr=%g RMS=%g\n",m1,m2);
        printf("calculate_average_T0(): too big RMS=%g. Trying to discard bad min/max channels...\n",m2);
        drop_min_max_T0(ech_T0);
        return calculate_average_T0(ech_T0);
    }
    
    return m1;
}

// -----------------------------------------------------------------------------

void fix_missing_T0(map<int, map<int,double> > &geoID_ech_T0, const multimap<int,MissingT0> & missing_T0)
{
    map<int,float> T0_avr; // average T0 per geoID
    set<int> known_geoID;
    
    for( multimap<int,MissingT0>::const_iterator it=missing_T0.begin(); it!=missing_T0.end(); it++ )
    {
        const int &geoID=it->first;
        if( known_geoID.end()==known_geoID.find(geoID) )  // if T0 is not known for this geoID....
        {
            known_geoID.insert(geoID);
            try
            {
                T0_avr[geoID] = calculate_average_T0(geoID_ech_T0[geoID]);
            }
            catch(const char *s)
            {
                printf("geoID=%d:   %s\n",geoID,s);
            }
        }
        
        assert(geoID_ech_T0[geoID][it->second.ech]==0);

        // Set average T0 for this geoID!
        if( T0_avr[geoID]!=0 )
        {
            //printf("fix_missing_T0(): geoID=%d ech=%d   T0avr=%f\n",geoID,it->second.ech,T0_avr[geoID]);
            it->second.drift_chan->GetRT()->SetT0(T0_avr[geoID]);
        }
    }
}

// -----------------------------------------------------------------------------
