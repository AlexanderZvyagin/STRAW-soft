#include <fstream>
#include <cmath>
#include <popt.h>
#include "dlfcn.h"

#include "TROOT.h"
//#include "TFile.h"
//#include "TTree.h"
//#include "TChain.h"

#include "Detectors/s_stream.h"
#include "V.h"
#include "VS/VS.h"

using namespace std;
using namespace CS;

namespace {

TROOT root("","");

int
    channel_first       = 0,
    channel_last        = 222,
    channels_group      = 1;
vector< pair<int,int> >
    straw_regions;
const char
    *detector           = "",
    *file_in            = "",
    *cuts               = "(tr_Xi2/tr_nh<3)&&(tr_z1<1450)&&(abs(tr_t)<4)&&(tr_q!=0)";

V* VConstruct_default(void) {return new VS;}
void VDestroy_default(V *v) {delete v;}
V* (*VConstruct)(void) = VConstruct_default;
void (*VDestroy)(V *v) = VDestroy_default;
V *v_code=VConstruct_default();
} // namespace

int minuit_printout     = -1;
int minuit_max_calls    = 10000;

////////////////////////////////////////////////////////////////////////////////

void many_V_fit(void)
{
    if( v_code==NULL )
        throw "many_V_fit(): v_cide==NULL";

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
            
            printf("# %s chf=%d chl=%d pos=[%d-%d;%d+%d]\n",detector,ch,ch+channels_group-1,pos,-delta,pos,delta);

            V::VFitResult result;

            result.detector         = detector;
            result.pos              = pos;
            result.delta            = delta;
            result.cuts             = cuts;
            result.channel_first    = ch;
            result.channel_last     = ch+channels_group-1;

            result.vdata.clear();

            //printf("Create vdata for channels [%d,%d] ....",result.channel_first,result.channel_last);
            //fflush(stdout);
            v_code->VCreate(file_in,result);
            //printf("  %d points created\n",result.vdata.size());
            
            printf("%d\n",result.vdata.size());
            for( std::vector<V::VData>::const_iterator it=result.vdata.begin(); it!=result.vdata.end(); it++ )
                printf("%g  %g  %g\n",it->x,it->t,it->w);
            printf("\n");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc,const char *argv[])
{
    try
    {
        int ac=0;
        char *av[]={"NULL"}; 

        struct poptOption options[] =
        {
            { "chf",        '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &channel_first,  0,
                                          "First channel to fit", "NUMBER" },
            { "chl",        '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &channel_last,   0,
                                          "Last channel to fit", "NUMBER" },
            { "chg",        '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &channels_group, 0,
                                          "Group channels togerther", "NUMBER" },
            { "pos",        '\0', POPT_ARG_STRING,  NULL,                                  'P',
                                          "This will select events in the straw region [center-delta,center+delta]. "
                                          "Example: --pos=-30,5", "center,delta" },
            { "cuts",       '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &cuts,        0,
                                          "Extra cuts to be used in events selection.", "STRING" },
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        poptContext poptcont=poptGetContext(NULL,argc,argv,options,0);
        poptSetOtherOptionHelp(poptcont,
            "<options...> [options] <file> <detector>\n"
            "  Generate V-plot data (will print it to standard output).\n"
            "  Format:\n"
            "      # comment\n"
            "      <number of points>\n"
            "      <distance [mm]>  <time [ns]>  <weight>\n"
            "      ......\n"
            "  \n"
            "  Author: Alexander Zvyagin <Alexander.Zvyagin@cern.ch>\n"
        );
        
        int rc;
        while( (rc=poptGetNextOpt(poptcont))>0 )
        {
            switch( rc )
            {
                case 'P':
                    int p,d;
                    if( 2!=sscanf(poptGetOptArg(poptcont),"%d,%d",&p,&d) )
                        throw "Bad straw region.";
                    straw_regions.push_back( pair<int,int>(p,d) );
                    break;

                default:
	                fprintf(stderr, "bad argument %s: %s\n",
		                    poptBadOption(poptcont, POPT_BADOPTION_NOALIAS),
		                    poptStrerror(rc));
                    return 1;
            }
        }

        file_in  = poptGetArg(poptcont);
        detector = poptGetArg(poptcont);
        
        if( file_in==NULL || detector==NULL || poptPeekArg(poptcont)!=NULL )
        {
            poptPrintHelp(poptcont,stdout,0);
            return 1;
        }

        many_V_fit();
        
        VDestroy(v_code);

        return 0;
    }
    catch(const char *e)
    {
        fprintf(stderr,"%s\n",e);
    }
    catch(const std::exception &e)
    {
        fprintf(stderr,"%s\n",e.what());
    }
    catch(...)
    {
        fprintf(stderr,"Unknown exception.\n");
    }

    delete v_code;

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
