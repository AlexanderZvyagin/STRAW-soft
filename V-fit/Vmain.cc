#include <fstream>
#include <cmath>
#include <popt.h>
#include "dlfcn.h"

#include "TROOT.h"
#include "TFile.h"
#include "TH2.h"
#include "TMinuit.h"
#include "TRint.h"
#include "TGraph.h"
#include "TTree.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TStyle.h"
#include "TSQLServer.h"

#include "s_stream.h"
#include "V.h"
#include "VS/VS.h"
#include "RTRelationGrid.h"
#include "RTRelationPoly.h"

using namespace std;
using namespace CS;

namespace {

TROOT root("","");

int
    channel_first       = -1,
    channel_last        = -1,
    channels_group      = 1,
    rt_limits           = 0;
float
    dt                  = 0,
    r                   = 0,
    V_center_coridor    = 0,
    V_leg_max_dist      = 9e9,
    V_fit_max_points    = 1,
    t0_start            = 0,
    w0_start            = 0,
    svel                = 0;
vector< pair<int,int> >
    straw_regions;
const char
    *session            = NULL,
    *comment            = "",
    *detector           = "",
    *file_in            = "",
    *db_name            = "STDC",
    *plugin_code        = "VS/libVS.so",
    *rt_string          = NULL,
    *fit_function       = "",
    *cuts               = "(tr_Xi2/tr_nh<3)&&(tr_z1<1450)&&(abs(tr_t)<4)&&(tr_q!=0)",
    *rt_r               = NULL,
    *filter             = "",
    *t0_ref             = "";
string
    program_full_name;
TSQLServer
    *db                 = NULL;

V* VConstruct_default(void) {return new VS;}
void VDestroy_default(V *v) {delete v;}
V* (*VConstruct)(void) = VConstruct_default;
void (*VDestroy)(V *v) = VDestroy_default;
V *v_code=VConstruct();
} // namespace

int minuit_printout     = -1;

////////////////////////////////////////////////////////////////////////////////

float get_t0_ref(const string &detector,const string &t0_ref)
{
    if( t0_ref=="" )
        return 0;

    float t0 = atof(t0_ref.c_str());
    if( t0!=0 )
        return t0;
    
    ifstream f(t0_ref.c_str());
    if( !f.is_open() )
        throw "get_t0_ref(): can not open file.";
        
    char det[22];
    while( f >> det >>t0 )
        if( detector==det )
            return t0;
     
     throw "get_t0_ref(): Can not find detector.";   
}

////////////////////////////////////////////////////////////////////////////////

void many_V_fit(void)
{
    char name[88], title[88];
    sprintf(name,"%s.root",session);
    TFile f(name,"UPDATE","",9);

    // By default we should fit the full straw! ==>> Use (0,0) as a (pos,delta).
    if( straw_regions.empty() )
        straw_regions.push_back( pair<int,int>(0,0) );

    for( int ch=channel_first; ch<=channel_last; ch+=channels_group )
    {
        for( vector< pair<int,int> >::const_iterator pos_it=straw_regions.begin();
             pos_it!=straw_regions.end(); pos_it++ )
        {
            int
                pos   = pos_it->first,
                delta = pos_it->second;
            
            sprintf(name,"%s_ch%d_ch%d_pos%s%d_delta%d",detector,ch,ch+channels_group-1,pos<0?"M":"",abs(pos),delta);
            sprintf(title,"Session \"%s\" %s channels [%dm%d]",session,detector,ch,ch+channels_group-1);

            V::VFitResult result;

            result.program          = program_full_name;
            result.comment          = comment;
            result.detector         = detector;
            result.pos              = pos;
            result.delta            = delta;
            result.cuts             = cuts;
            result.channel_first    = ch;
            result.channel_last     = ch+channels_group-1;
            result.dt               = dt;
            result.r                = r;
            result.V_center_coridor = V_center_coridor;
            result.V_leg_max_dist   = V_leg_max_dist;
            result.V_fit_max_points = V_fit_max_points;
            result.t0_start         = t0_start;
            result.w0_start         = w0_start;
            result.t0_ref           = get_t0_ref(detector,t0_ref);
            result.signal_velocity  = svel;
            result.fit_function     = fit_function;

            if( rt_string!=NULL )
            {
                if( 0==strncmp(rt_string,"RT-Grid ",8) )
                    result.rt = new RTRelationGrid(rt_string);
                else
                if( 0==strncmp(rt_string,"RT-Poly ",8) )
                    result.rt = new RTRelationPoly(rt_string);
                else
                    throw "Unknwon RT-relation.";

                if( result.r==0 )
                    result.r = result.rt->GetRMax();

                if( result.dt==0 )
                    result.dt = result.rt->GetTMax() - result.rt->GetT0();

                result.rt->SetUseLimits(rt_limits);
            }

            if( rt_r!=NULL )
            {
                istringstream s(rt_r);
                float r;
                while( s>>r )
                    result.rt_r.push_back(r);
            }

            result.vdata.clear();

            printf("Create vdata for channels [%d,%d] ....",result.channel_first,result.channel_last);
            fflush(stdout);
            v_code->VCreate(file_in,result);
            printf("  %d points created\n",result.vdata.size());
            result.hV = v_code->MakeHistogram(name,title,result.vdata,result.r*2.4,result.dt);

            f.cd();

            if( *filter!=0 )
            {
                printf("Applying the filter %s\n",filter);

                istringstream s(filter);
                int Nx,Nt;
                float t_min,t_max, bkg;
                if( !(s >> Nx >> Nt >> t_min >> t_max >> bkg) )
                    throw "Bad filter-string.";

                // Create the histogram!
                result.hVf = new TH2F(name,title,Nx,-1.4*result.r,1.4*result.r,Nt,t_min,t_max);
                result.hVf->GetXaxis()->SetTitle("Distance [cm]");
                result.hVf->GetYaxis()->SetTitle("Time [ns]");

                // Now we fill the histogram...
                for( vector<V::VData>::const_iterator it=result.vdata.begin(); it!=result.vdata.end(); it++ )
                    result.hVf -> Fill(it->x,it->t);

                // Calculate number of background events in every bin.
                if( bkg!=0 )
                    if( bkg<1 )
                    {
                        bkg=int(result.hVf->GetMaximum()*bkg);
                        if( bkg<=0 )
                            bkg=1;
                    }
                // Done. Every time it will be cuted away 'bkg' number of events from every bin.

                // Now we are going to replace the selected events by histogram enties, with the background subtraction.
                result.vdata.clear();

                float total=0;

                for( int bx=1; bx<=result.hVf->GetXaxis()->GetNbins(); bx++ )
                    for( int bt=1; bt<=result.hVf->GetYaxis()->GetNbins(); bt++ )
                    {
                        int bin = result.hVf->GetBin(bx,bt);
                        V::VData v;
                        v.x = result.hVf->GetXaxis()->GetBinCenter(bx);
                        v.t = result.hVf->GetYaxis()->GetBinCenter(bt);
                        v.w = result.hVf->GetBinContent(bx,bt) - bkg;
                        if( v.w>0 )
                        {
                            result.vdata.push_back( v );
                            total += v.w;
                            result.hVf->SetBinContent(bin,v.w);
                        }
                        else
                            result.hVf->SetBinContent(bin,0);
                    }

                printf("Background events (cut=%g) removed. There are %d bins with %g points.\n",bkg,result.vdata.size(),total);
            }

            v_code->VFit(result);
            assert(result.hV!=NULL);
            v_code->FillResidualPlots(result);
            v_code->MakeReport(result)->Write();

            if( db!=NULL )
                v_code->VStore(db,session,result);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void load_plugins(const char *path,V* (*&VConstruct)(void), void (*&VDestroy)(V *v))
{
    if( path==NULL || *path==0 )
        throw "load_plugins(): can not load an empty plugin.";

    printf("Loading plugin %s ...\n",path);
    void *handle=NULL;

    char *error=NULL;

    handle = dlopen (path, RTLD_LAZY);
    if (!handle)
        throw dlerror();

    VConstruct = (V* (*)(void)) dlsym(handle, "VConstruct");
    if ((error = dlerror()) != NULL)
        throw error;

    VDestroy = (void (*)(V*)) dlsym(handle, "VDestroy");
    if ((error = dlerror()) != NULL)
        throw error;

    //dlclose(handle);
    printf("Plugin %s loaded successfully.\n",path);
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc,const char *argv[])
{
    try
    {
        int ac=0, graphics=0;
        char *av[]={"NULL"}; 

        char 
            *db_host="na58pc052.cern.ch",
            *db_user="",
            *db_password="";

        struct poptOption options[] =
        {
            { "code",       '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &plugin_code, 'p',
                                          "V code to load", "library.so" },
            { "data",       '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &file_in,     0,
                                          "Input ROOT file. "
                                          "Sasha' ntuples: /home2/zvyagin/results-coral/7/27573.root. "
                                          "Klaus' ntuples: /home/rgeyer/Klaus/reiner_V/ROOT_files/", "PATH" },
            { "det",        '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &detector,    0,
                                          "STRAW detector to be used", "NAME" },
            { "mdebug",     '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &minuit_printout,0,
                                          "Minuit printout level (3,2,1,0,-1)", "LEVEL" },
            { "chf",        '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &channel_first,  0,
                                          "First channel to fit", "NUMBER" },
            { "chl",        '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &channel_last,   0,
                                          "Last channel to fit", "NUMBER" },
            { "chg",        '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &channels_group, 0,
                                          "Group channels togerther", "NUMBER" },
            { "pos",        '\0', POPT_ARG_STRING,  NULL,                                  'P',
                                          "This will select events in the straw region [center-delta,center+delta]. "
                                          "Example: --pos=-30,5", "center,delta" },
            { "RT",         '\0', POPT_ARG_STRING,  &rt_string,                             0,
                                          "RT relations to bee used in fitting.", "STRING" },
            { "RT-r",       '\0', POPT_ARG_STRING,  &rt_r,                                 'R',
                                          "List of deistances (separated by space) to be used for fixed RT", "STRING" },
            { "rt-range",   '\0', POPT_ARG_NONE, &rt_limits,                               'l',
                                          "Use RT range in V-fitting procedure, default - no range limitation.", "NUMBER" },
            { "dt",         '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &dt,           0,
                                          "Maximum drift time, use 0 if unknown.", "FLOAT" },
            { "r",          '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &r,            0,
                                          "Maximum drift distance, use 0 if unknown.", "FLOAT" },
            { "V-center-rm",'\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &V_center_coridor, 0,
                                          "Do not take into account points of the V-plot with "
                                          "the distance closer then specified.", "FLOAT" },
            { "V-leg-dist", '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &V_leg_max_dist, 0,
                                          "For V-fit: maximum distance between RT-leg and a V-plot point.", "FLOAT" },
            { "V-fit-max-points", '\0', POPT_ARG_NONE,  &V_fit_max_points, 0,
                                          "For V-fit: maximum number of fitted points.", "FLOAT" },
            { "t0-start",   '\0', POPT_ARG_FLOAT,  &t0_start, 0,
                                          "Starting value for t0 calculation. If not given, "
                                          "it is taken from --t0-ref option or (if it the option is missing) "
                                          "automatic t0 calculation is used.", "FLOAT" },
            { "w0-start",   '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &w0_start, 0,
                                          "Starting value for w0 calculation (wire position).", "FLOAT" },
            { "t0-ref",     '\0', POPT_ARG_STRING,  &t0_ref, 0,
                                          "T0 refernce used for 'uncorrected' residual plot creation. "
                                          "It can be either a float number: --t0-ref=-1600.16 or a text file "
                                          "with the format 'detector T0' in every line.", "FLOAT or STRING" },
            { "svel",       '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &svel, 0,
                                          "Signal propagation velocity. Use 0, if you don't want to use it.", "FLOAT" },
            { "cuts",       '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &cuts,        0,
                                          "Extra cuts to be used in events selection.", "STRING" },
            { "fit-func",   '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &fit_function,0,
                                          "Name of the fit function to be used.", "STRING" },
            { "filter",     '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &filter,          0,
                                          "Filter-histogram for background subtraction \"Nx Nt t_min t_max bkg\". "
                                          "It uses 'r' argument as well."
                                          "The argument 'bkg' - background events. If bkg>=1 we cut away this number of events from every bin. "
                                          "If 0<bkg<1 - this fraction of events will be cuted from maximum bin content."
                                          , "STRING" },
            { "dbhost",     '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &db_host,     0,
                                          "Database to store results", "NAME" },
            { "dbuser",     '\0', POPT_ARG_STRING,  &db_user,                               0,
                                          "Database user", "NAME" },
            { "dbpassword", '\0', POPT_ARG_STRING,  &db_password,                           0,
                                          "COMPASS password", "NAME" },
            { "comment",    '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &comment,     0,
                                          "Comment to the data fit", "NAME" },
            { "graphics",   '\0', POPT_ARG_NONE,  &graphics,    0,
                                          "Run the program with grpahics", "" },
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        poptContext poptcont=poptGetContext(NULL,argc,argv,options,0);
        poptSetOtherOptionHelp(poptcont,
            "<options...> <session-name>\n"
            "  General interface to V-plot fitting.\n"
            "  The output root file will be '<session-name>.root'\n"
            "  You must specify either --RT (fit with a fixed RT) or --dt (get RT) option.\n"
            "  Author: Alexander Zvyagin <Alexander.Zvyagin@cern.ch>\n"
        );
        
        int rc;
        while( (rc=poptGetNextOpt(poptcont))>0 )
        {
            switch( rc )
            {
                case 'p':
                    load_plugins(poptGetOptArg(poptcont),VConstruct,VDestroy);
                    VDestroy(v_code);
                    v_code = VConstruct();
                    break;

                case 'P':
                    int p,d;
                    if( 2!=sscanf(poptGetOptArg(poptcont),"%d,%d",&p,&d) )
                        throw "Bad straw region.";
                    straw_regions.push_back( pair<int,int>(p,d) );
                    break;

                case 'l':
                    rt_limits=1;
                    break;

                default:
	                fprintf(stderr, "bad argument %s: %s\n",
		                    poptBadOption(poptcont, POPT_BADOPTION_NOALIAS),
		                    poptStrerror(rc));
                    return 1;
            }
        }

        session = poptGetArg(poptcont);
        if( session==NULL || poptPeekArg(poptcont)!=NULL )
        {
            poptPrintHelp(poptcont,stdout,0);
            return 1;
        }

        if( !graphics )
            gROOT->SetBatch();

        gStyle->SetOptFit(1);
        gStyle->SetStatFormat("8.6g");
        gStyle->SetFitFormat("8.6g");
        gStyle->SetFrameFillColor(kWhite);
        gStyle->SetCanvasColor(kWhite);
        
        TApplication app("app",&ac,av);

        if( *db_user!=0 )
        {
            string db_path=string("mysql://")+string(db_host);
            db = TSQLServer::Connect(db_path.c_str(),db_user,db_password);
            if( db==NULL )
                throw "Can not open DB.";
            
            char buf[111];
            sprintf(buf,"USE %s;",db_name);

            if( !db->Query(buf) )
            {
                printf("**** FAILED: %s\n",buf);
                throw "Error in communication with the DB.";
            }
        }

        if( 1 )
        {
            program_full_name="";
            // Create the program full name
            for( int a=0; a<argc; a++ )
            {
                if( strchr(argv[a],' ')!=NULL )
                {
                    program_full_name += '"';
                    program_full_name += argv[a];
                    program_full_name += '"';
                }
                else
                    program_full_name += argv[a];
                program_full_name += ' ';
            }
        }
        
        if( t0_start==0 && *t0_ref!=0 )
            t0_start = get_t0_ref(detector,t0_ref);

        printf("%s\n",program_full_name.c_str());
        printf("Session name .................... %s\n", session);
        printf("Output file ..................... %s.root\n",session);
        printf("Comment ......................... %s\n", comment);
        printf("Input V data .................... %s\n", file_in);
        printf("Detector ........................ %s\n", detector);
        printf("Channel first ................... %d\n", channel_first);
        printf("Channel last .................... %d\n", channel_last);
        printf("Channels in a group ............. %d\n", channels_group);
        for( size_t i=0; i<straw_regions.size(); i++ )
            printf("Detector V-plot region .......... [%d-%d,%d+%d]\n",
                    straw_regions[i].first,straw_regions[i].second,
                    straw_regions[i].first,straw_regions[i].second);
        printf("RT .............................. %s\n", rt_string);
        printf("RT range is ..................... %s\n", rt_limits?"ON":"OFF");
        printf("V center exclude ................ |r|<%g\n",V_center_coridor);
        printf("RT-leg to V-point max distance... <%g\n",V_leg_max_dist);
        printf("V-fit maximize points factor .... %g\n",V_fit_max_points);
        printf("Maximum drift time .............. %g\n", dt);
        printf("Maximum drift distance .......... %g\n", r);
        printf("Starting t0 for the fit ......... ");
        if( t0_start==0 )
            printf("automatic calculation\n");
        else
            printf("%g\n",t0_start);
        printf("Starting w0 for the fit ......... %g\n", w0_start);
        printf("cuts ............................ %s\n", cuts);
        printf("fit function .................... %s\n", fit_function );
        printf("filter: Nx Nt t_min t_max bkg.... %s\n", filter);
        printf("graphics ........................ %s\n", graphics?"ON":"OFF");

        if( db==NULL )
            printf("DB .............................. is not used\n");
        else
        {
            printf("DB host ......................... %s\n",db_host);
            printf("DB name (+session) .............. %s.%s\n",db_name,session);
        }

        many_V_fit();
        
        VDestroy(v_code);

        return 0;
    }
    catch(const char *e)
    {
        printf("%s\n",e);
    }
    catch(const std::exception &e)
    {
        printf("%s\n",e.what());
    }
    catch(...)
    {
        printf("Unknown exception.\n");
    }

    delete v_code;

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
