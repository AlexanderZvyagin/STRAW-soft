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

#include "Detectors/s_stream.h"
#include "V.h"
#include "VS/VS.h"
#include "Detectors/RTRelationGrid.h"
#include "Detectors/RTRelationPoly.h"

using namespace std;
using namespace CS;

namespace {

TROOT root("","");

float
    t0_start            = 0,
    w0_start            = 0,
    svel                = 0;
const char
    *session            = NULL,
    *detector           = "",
    *file_in            = "",
    *rt_string          = NULL,
    *cuts               = "(tr_Xi2/tr_nh<3)&&(tr_z1<1450)&&(abs(tr_t)<4)&&(tr_q!=0)";
string
    program_full_name;

V* VConstruct_default(void) {return new VS;}
void VDestroy_default(V *v) {delete v;}
V* (*VConstruct)(void) = VConstruct_default;
void (*VDestroy)(V *v) = VDestroy_default;
V *v_code=VConstruct_default();
} // namespace

int minuit_printout     = -1;

////////////////////////////////////////////////////////////////////////////////

void RT_fit(void)
{
    if( v_code==NULL )
        throw "many_V_fit(): v_code==NULL";

    char name[88], title[88];
    sprintf(name,"%s.root",session);
    TFile f(name,"UPDATE","",9);

    V::VFitResult result;

    result.program          = program_full_name;
    result.detector         = detector;
    result.pos              = 0;
    result.delta            = 0;
    result.cuts             = cuts;
    result.channel_first    = -1;
    result.channel_last     = -1;
    result.t0_start         = t0_start;
    result.w0_start         = w0_start;
    result.signal_velocity  = svel;

    if( rt_string==NULL )
        throw "RT_fit(): No initial RT!";

    if( 0==strncmp(rt_string,"RT-Grid ",8) )
        result.rt = new RTRelationGrid(rt_string);
    else
        throw "RT_fit(): Unknwon RT-relation.";

    if( result.t0_start!=0 )
        result.rt->SetT0(result.t0_start);
    else
        result.t0_start = result.rt->GetT0();

    result.r  = result.rt->GetRMax();
    result.dt = result.rt->GetTMax();

    result.vdata.clear();

    printf("Create vdata ....");
    fflush(stdout);
    v_code->VCreate(file_in,result);
    printf("  %d points created\n",result.vdata.size());

    f.cd();

    sprintf(name,"%s__V",detector);
    sprintf(title,"Session \"%s\" %s",session,detector);
    result.hV = v_code->MakeHistogram(name,title,result.vdata,result.r*2.4,result.dt);

    VS *v = dynamic_cast<VS*>(v_code);
    if( v==NULL )
        throw "RT_fit(): not a VS object!";

    v->CalculateRT2(result);
    v_code->FillResidualPlots(result);
    v_code->MakeReport(result)->Write();

    // Print the results.

    printf("*******************************************************\n");
    if( result.rt!=NULL )
    {
        printf("The RT is:\n");
        printf("%s\n",string(*result.rt).c_str());
        printf("t0=%g   w0=%g\n",result.t0,result.w0);
    }
    else
        printf("RT calculation has failed!\n");
    printf("*******************************************************\n");
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc,const char *argv[])
{
    try
    {
        int ac=0, graphics=0;
        char *av[]={"NULL"}; 

        struct poptOption options[] =
        {
            { "data",       '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &file_in,     0,
                                          "Input ROOT file. ", "PATH" },
            { "det",        '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &detector,    0,
                                          "STRAW detector to be used", "NAME" },
            { "mdebug",     '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &minuit_printout,0,
                                          "Minuit printout level (3,2,1,0,-1)", "LEVEL" },
            { "RT",         '\0', POPT_ARG_STRING,  &rt_string,                             0,
                                          "Starting RT relations", "STRING" },
            { "t0-start",   '\0', POPT_ARG_FLOAT,  &t0_start, 0,
                                          "Starting value for t0 calculation. If not given, "
                                          "it is taken from --t0-ref option or (if it the option is missing) "
                                          "automatic t0 calculation is used.", "FLOAT" },
            { "w0-start",   '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &w0_start, 0,
                                          "Starting value for w0 calculation (wire position).", "FLOAT" },
            { "svel",       '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &svel, 0,
                                          "Signal propagation velocity. Use 0, if you don't want to use it.", "FLOAT" },
            { "cuts",       '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &cuts,        0,
                                          "Extra cuts to be used in events selection.", "STRING" },
            { "graphics",   '\0', POPT_ARG_NONE,  &graphics,    0,
                                          "Run the program with grpahics", "" },
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        poptContext poptcont=poptGetContext(NULL,argc,argv,options,0);
        poptSetOtherOptionHelp(poptcont,
            "<options...> <session-name>\n"
            "  RT calculation from a V-plot.\n"
            "  Author: Alexander Zvyagin <Alexander.Zvyagin@cern.ch>\n"
        );

        int rc;
        while( (rc=poptGetNextOpt(poptcont))>0 )
        {
            switch( rc )
            {
                default:
	                fprintf(stderr, "bad argument %s: %s\n",
		                    poptBadOption(poptcont, POPT_BADOPTION_NOALIAS),
		                    poptStrerror(rc));
                    return 1;
            }
        }
        
        session = poptGetArg(poptcont);
        printf("%p %p\n",session,poptPeekArg(poptcont));
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
        
        printf("%s\n",program_full_name.c_str());
        printf("Session name .................... %s\n", session);
        printf("Output file ..................... %s.root\n",session);
        printf("Input V data .................... %s\n", file_in);
        printf("Detector ........................ %s\n", detector);
        printf("RT .............................. %s\n", rt_string);
        printf("Starting t0 for the fit ......... %g\n", t0_start);
        printf("Starting w0 for the fit ......... %g\n", w0_start);
        printf("cuts ............................ %s\n", cuts);
        printf("graphics ........................ %s\n", graphics?"ON":"OFF");

        RT_fit();
        
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
