#include <map>
#include <memory>
#include <cassert>
#include <popt.h>

#include "TROOT.h"
#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TH1.h"
#include "TNtuple.h"
#include "TFile.h"
#include "TMinuit.h"
#include "TCanvas.h"

using namespace std;

TROOT root("","");

float kwerr=1;
map<string,TCanvas *> pages;

////////////////////////////////////////////////////////////////////////////////

unsigned int get_channels_number(const char *det)
{
    if( det==NULL || strlen(det)!=8 )
    {
        printf("%s\n",det);
        throw "get_channels_number(): bad detector name";
    }
    
    const unsigned int
        Channels6mmStrawsXY [2] = {222,192},
        Channels10mmStrawsXY[2] = {96,64},
        xy = det[4]=='Y';
    
    return det[7]=='b' ? Channels6mmStrawsXY[xy] : Channels10mmStrawsXY[xy];
}

////////////////////////////////////////////////////////////////////////////////

float read_xray(TSQLServer &db,const string &det,int ch,int spacer)
{
    if( spacer==0 )
    {
        // spacer 0 is the center of detector where we do not have X-ray measurements.
        // So we interpolate measurements from two closes spacers.
        return (read_xray(db,det,ch,-1)+read_xray(db,det,ch,1))/2;
    }

    char q[222];
    
    sprintf(q,"SELECT W FROM STDC.xray WHERE detector='%s' AND channel=%d AND spacer=%d",det.c_str(),ch,spacer);
    
    auto_ptr<TSQLResult> r(db.Query(q));
    if( r.get()==NULL )
    {
        printf("Failed querry: %s\n",q);
        throw "DB request failed.";
    }
    
    auto_ptr<TSQLRow> m(r->Next());
    if( m.get()==NULL )
    {
        printf("No measurements: %s\n",q);
        throw "No measurements.";
    }
    float w=atof(m->GetField(0));

    if( r->Next()!=NULL )
    {
        printf("Second mesurement?\n");
    }
    
    return w;
}

////////////////////////////////////////////////////////////////////////////////

/*! Read from DB.
*/
void read(TSQLServer &db, const string &table,const string &det,int ch,int spacer,float &x,float &x_err)
{
    if( table=="xray" )
    {
        x=read_xray(db,det,ch,spacer);
        x_err=0;
        return;
    }

    char q[222];
    
    sprintf(q,"SELECT W,W_err FROM STDC.%s WHERE detector='%s' AND chf=%d AND chl=%d AND spacer=%d",
                table.c_str(),det.c_str(),ch,ch,spacer);
    
    auto_ptr<TSQLResult> r(db.Query(q));
    if( r.get()==NULL )
    {
        printf("Failed querry: %s\n",q);
        throw "DB request failed.";
    }

    auto_ptr<TSQLRow> m(r->Next());
    if( m.get()==NULL )
    {
        printf("No measurements: %s\n",q);
        throw "No measurements.";
    }
    x=atof(m->GetField(0));
    x_err=atof(m->GetField(1));

    if( r->Next()!=NULL )
    {
        printf("Second mesurement?\n");
    }
}

////////////////////////////////////////////////////////////////////////////////

string spacer_name(int spacer)
{
    char s[33];
    sprintf(s,"%s%d",spacer==0?"":spacer<0?"m":"p",abs(spacer));
    return s;
}

////////////////////////////////////////////////////////////////////////////////

class DetCor
{
  private:
                        DetCor                  (const DetCor &);
  public:
                        DetCor                  (TSQLServer &db, const string &table, const string &detector,int spacer);
                       ~DetCor                  (void) {}

    void                Align                   (void);

    string              detector;
    int                 spacer;
    string              table;
    TH1F *              h;
};

////////////////////////////////////////////////////////////////////////////////

DetCor::DetCor(TSQLServer &db, const string &t, const string &det,int sp) :
    detector(det),
    spacer(sp),
    table(t)
{
    unsigned int n_channels=get_channels_number(detector.c_str());
    char name[33];
    sprintf(name,"%s_%s_%s",detector.c_str(),table.c_str(),spacer_name(spacer).c_str());
    h = new TH1F(name,"",n_channels,0,n_channels);
    h->GetXaxis()->SetTitle("Channel number");
    h->GetYaxis()->SetTitle("Wire position correction [#mu m]");
    h->SetMaximum( 300);
    h->SetMinimum(-300);

    for( unsigned int ch=0; ch<n_channels; ch++ )
    {
        float w, w_err;
        try
        {
            read(db,table,detector,ch,spacer,w,w_err);
            // Apply alignment and convert also [cm] to [micro meter]
            h->SetBinContent(ch+1,w*10000);
            h->SetBinError(ch+1,w_err*10000*kwerr);
        }
        catch(const char *s)
        {
            printf("%s\n",s);
        }
    }

    Align();
}

////////////////////////////////////////////////////////////////////////////////

/*! Get rid of pitch-size effect and put all corrections around 0 (align corrections).
*/
void DetCor::Align(void)
{
    const char *func="pol1";
    h->Fit(func,"N");
    double a,b,e;
    gMinuit->GetParameter(0,a,e);
    gMinuit->GetParameter(1,b,e);
    
    for( int bin=1; bin<=h->GetXaxis()->GetNbins(); bin++ )
    {
        float x=h->GetXaxis()->GetBinCenter(bin);
        h->SetBinContent( bin, h->GetBinContent(bin)-a-b*x );
    }
    //h->GetListOfFunctions()->Remove(h->GetListOfFunctions()->FindObject(func) );
}

////////////////////////////////////////////////////////////////////////////////

void make_report(DetCor &d1, DetCor &d2)
{
    if( d1.detector!=d2.detector )
        throw "make_report(): different detectors";

    if( d1.spacer!=d2.spacer )
        throw "make_report(): bad spacers";

    if( abs(d1.spacer)>1 )
        throw "make_report(): bad spacer number";

    char q[222];
    sprintf(q,"%s_%s_%s",d1.detector.c_str(),
            d1.table.c_str(),d2.table.c_str());
    TCanvas *&c = pages[q];
    if( c==NULL )
    {
        c = new TCanvas(q);
        c->Divide(1,3);
    }

    c->cd(-d1.spacer+2);

    d1.h->SetLineColor(kBlue);
    d1.h->Draw();

    d2.h->SetLineColor(kRed);
    d2.h->Draw("SAME");

    //h1->SetLineStyle(kSolid);
    //h2->SetLineStyle(kDashed);
    
    //c->Write();
    //d1.h->Write();
    //d2.h->Write();
    
    c->cd(0);
}

////////////////////////////////////////////////////////////////////////////////

void straw_names_create(vector<string> &st_dets)
{
    const char *sts[]=
    {
        "ST03X1","ST03X2","ST03Y1","ST03Y2","ST03V1","ST03U1",
        "ST04X1","ST04Y1","ST04V1",
        "ST05X1","ST05Y1","ST05U1",
        "ST06X1","ST06Y1","ST06V1",NULL
    };

    st_dets.clear();

    for( const char **s=sts; *s!=NULL; s++ )
        for( int ud=0; ud<2; ud++ )
            for( char abc='a'; abc<='c'; abc++ )
                st_dets.push_back(string(*s)+"ud"[ud]+abc);
}

////////////////////////////////////////////////////////////////////////////////

void compare(TSQLServer &db,const char *table1,const char *table2,const char *detector,int spacer)
{
    DetCor
        d1(db,table1,detector,spacer),
        d2(db,table2,detector,spacer);

    try
    {
        make_report(d1,d2);
    }
    catch(const char *s)
    {
        printf("%s\n",s);
    }
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc,const char **argv)
{
    char status=0;
    
    try
    {
        const char
            *detector   = "",
            *table[2]   = {"",""},
            *root_file  = "";

        struct poptOption options[] =
        {
            { "detector",   '\0', POPT_ARG_STRING,  &detector,         0,
                                          "Make comparision only for this detector "
                                          "(default: all detectors found in both tables)", "NAME" },
            { "kwerr",      '\0', POPT_ARG_FLOAT|POPT_ARGFLAG_SHOW_DEFAULT,  &kwerr,         0,
                                          "Wire correction error suppresion factor", "NUMBER" },
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        poptContext poptcont=poptGetContext(NULL,argc,argv,options,0);
        poptSetOtherOptionHelp(poptcont,
            "<options...> <table1> <table2> <root-file>\n"
            "  Compare V-fit results. For X-ray table use name 'xray'.\n"
            "  Author: Alexander Zvyagin <Alexander.Zviagine@cern.ch>\n"
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

        table[0]  = poptGetArg(poptcont);
        table[1]  = poptGetArg(poptcont);
        root_file = poptGetArg(poptcont);
        
        if( root_file==NULL || poptPeekArg(poptcont)!=NULL )
        {
            poptPrintHelp(poptcont,stdout,0);
            return 1;
        }

        TSQLServer *db=TSQLServer::Connect("mysql://na58pc052.cern.ch","anonymous","");
        if( db==NULL )
            throw "Can not open the DB.";

        // ---------------------------------------------------------------------

        // Create straw detectors list
        vector<string> st_dets;
        straw_names_create(st_dets);

        // ---------------------------------------------------------------------

        TFile
            f(root_file,"UPDATE","",9);

        for( int spacer=-3; spacer<=3; spacer++ )
            if( *detector!=0 )
                compare(*db,table[0],table[1],detector,spacer);
            else
                for( vector<string>::const_iterator st=st_dets.begin(); st!=st_dets.end(); st++ )
                    printf("%s\n",st->c_str()),compare(*db,table[0],table[1],st->c_str(),spacer);
        
        for( map<string,TCanvas *>::iterator p=pages.begin(); p!=pages.end(); p++ )
            p->second->Write();
        
        f.Write();
        f.Close();
    }
    catch(const std::exception &e)
    {
        printf("%s\n",e.what());
        status=1;
    }
    catch(const char *e)
    {
        printf("%s\n",e);
        status=1;
    }
    catch(...)
    {
        printf("Unknown exception\n");
        status=1;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
