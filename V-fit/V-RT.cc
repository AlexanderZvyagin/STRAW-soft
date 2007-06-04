#include <fstream>
#include <cmath>
#include <popt.h>
#include "dlfcn.h"

#include "TROOT.h"
#include "TFile.h"
#include "TRint.h"
#include "TStyle.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TCanvas.h"

#include "Detectors/s_stream.h"
#include "V.h"
#include "VS/VS.h"
#include "Detectors/RTRelationGrid.h"

using namespace std;
using namespace CS;

namespace {

TROOT root("","");

V* VConstruct_default(void) {return new VS;}
void VDestroy_default(V *v) {delete v;}
V* (*VConstruct)(void) = VConstruct_default;
void (*VDestroy)(V *v) = VDestroy_default;
V *v_code=VConstruct_default();
} // namespace

float V_leg_max_dist    = 9e9;
int minuit_printout     = -1;
int minuit_max_calls    = 10000;

////////////////////////////////////////////////////////////////////////////////

void report(V::VFitResult &result)
{
    TCanvas *c1 = new TCanvas("Vplot");

    // 1. draw data
    TH2 &hV = *const_cast<TH2*>(result.hV);
    gStyle->SetPalette(1);
    hV.Draw("COLZ");

    // 2. draw rt

    if( result.rt!=NULL )
        result.rt->MakeGraph(result.w0)->Draw("SL*");
    
    c1->Write();
    c1->Print();
    
    TCanvas *c2 = new TCanvas("Vresidual");

    // 3. draw residual plot
    if( result.residuals_corr[0].h!=NULL )
        result.residuals_corr[0].h->Draw();
        
    c2->Write();
    c2->Print();
}

////////////////////////////////////////////////////////////////////////////////

void RT_fit(V::VFitResult &result)
{
    printf("Starting the fit...\n");

    if( v_code==NULL )
        throw "many_V_fit(): v_code==NULL";

    result.r  = result.rt->GetRMax();
    result.dt = result.rt->GetTMax();
    result.V_leg_max_dist = V_leg_max_dist;

    char title[result.comment.size()+33];
    sprintf(title,"Session \"%s\"",result.comment.c_str());
    result.hV = v_code->MakeHistogram("V",title,result.vdata,result.r*2.4,result.dt);

    VS *v = dynamic_cast<VS*>(v_code);
    if( v==NULL )
        throw "RT_fit(): not a VS object!";

    v->CalculateRT2(result);
    v_code->FillResidualPlots(result,0.15,100,0.15);
    result.residuals_corr[0].Fit("gaus");
    v_code->MakeReport(result)->Write();

    // Print the results.

    printf("*******************************************************\n");
    if( result.rt!=NULL )
    {
        printf("The RT is:\n");
        printf("%s\n",string(*result.rt).c_str());
        printf("t0=%g   w0=%g\n",result.t0,result.w0);
        
        report(result);
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
        V::VFitResult result;

        int ac=0;
        char
            *av[]={"NULL"};
        const char
            *session   = NULL,
            *rt_string = NULL;

        float
            t0_start            = 0,
            w0_start            = 0,
            svel                = 0;

        struct poptOption options[] =
        {
            { "mdebug",     '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &minuit_printout,0,
                                          "Minuit printout level (3,2,1,0,-1)", "LEVEL" },
            { "max-calls",  '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &minuit_max_calls,0,
                                          "Minuit: maximum number of function calls.", "INTEGER" },
            { "RT",         '\0', POPT_ARG_STRING,  &rt_string,                             0,
                                          "Starting RT relation", "STRING" },
            { "V-leg-dist", '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &V_leg_max_dist, 0,
                                          "For V-fit: maximum distance between RT-leg and a V-plot point.", "FLOAT" },
            { "t0-start",   '\0', POPT_ARG_FLOAT,  &result.t0_start, 0,
                                          "Starting value for t0 calculation. If not given, "
                                          "it is taken from --t0-ref option or (if it the option is missing) "
                                          "automatic t0 calculation is used.", "FLOAT" },
            { "w0-start",   '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &result.w0_start, 0,
                                          "Starting value for w0 calculation (wire position).", "FLOAT" },
            { "svel",       '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &result.signal_velocity, 0,
                                          "Signal propagation velocity. Use 0, if you don't want to use it.", "FLOAT" },
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

        if( session==NULL || poptPeekArg(poptcont)!=NULL )
        {
            poptPrintHelp(poptcont,stdout,0);
            return 1;
        }

        gROOT->SetBatch();

        gStyle->SetOptFit(1);
        gStyle->SetStatFormat("8.6g");
        gStyle->SetFitFormat("8.6g");
        gStyle->SetFrameFillColor(kWhite);
        gStyle->SetCanvasColor(kWhite);
        
        TApplication app("app",&ac,av);

        if( 1 )
        {
            result.program="";
            // Create the program full name
            for( int a=0; a<argc; a++ )
            {
                if( strchr(argv[a],' ')!=NULL )
                {
                    result.program += '"';
                    result.program += argv[a];
                    result.program += '"';
                }
                else
                    result.program += argv[a];
                result.program += ' ';
            }
        }

        result.comment  = session;
        result.detector = "UNKNOWN";
        
        printf("%s\n",result.program.c_str());
        printf("Session name .................... %s\n", session);
        printf("Output file ..................... %s.root\n",session);
        printf("Detector ........................ %s\n", result.detector.c_str());
        printf("RT .............................. %s\n", rt_string);
        printf("Starting t0 for the fit ......... %g\n", result.t0_start);
        printf("Starting w0 for the fit ......... %g\n", result.w0_start);

        const string fname = string(session)+".root";
        TFile f(fname.c_str(),"UPDATE","",9);

        if(1)
        {
            if( rt_string==NULL )
                throw "RT_fit(): No initial RT!";

            if( 0==strncmp(rt_string,"RT-Grid ",8) )
                result.rt = new RTRelationGrid(rt_string);
            else
                throw "RT_fit(): Unknwon RT-relation.";

            result.detector         = "UNKNOWN";
            result.signal_velocity  = svel;

            if( V::read_data(cin,result.vdata) )
                RT_fit(result);
        }
        
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
