#include "TH2.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TSQLServer.h"
#include "TText.h"
#include "TPaveText.h"

#include "Detectors/RTRelation.h"

#include "V.h"

using namespace CS;

////////////////////////////////////////////////////////////////////////////////

V::VFitResult::~VFitResult(void)
{
    delete hV;
    delete hVf;
    delete hT;
    delete gResolution;
}

////////////////////////////////////////////////////////////////////////////////

const std::string V::Residual::GetName(void) const
{
    if( h==NULL )
        throw "V::Residual::GetName(): no name.";
    return h->GetName();
}

////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> V::VFitResult::MakeDescription(void) const
{
    char s[2222]="";
    std::vector<std::string> v;

    sprintf(s,"%s channels [%d,%d]\n",detector.c_str(),channel_first,channel_last);
    v.push_back(s);
    
    sprintf(s,"straw region [%d-%d,%d+%d] cm\n",pos,delta,pos,delta);
    v.push_back(s);
    
    sprintf(s,"cuts: %s\n",cuts.c_str());
    v.push_back(s);
    
    sprintf(s,"signal propagation speed: %g cm/ns\n",signal_velocity);
    v.push_back(s);
    
    sprintf(s,"V_center_coridor: %g cm\n",V_center_coridor);
    v.push_back(s);
    
    sprintf(s,"V_leg_max_dist: %g cm\n",V_leg_max_dist);
    v.push_back(s);
    
    sprintf(s,"T0 from fit is %g ns\n",t0);
    v.push_back(s);
    
    sprintf(s,"W0 from fit is %g #mu m\n",w0*10000);
    v.push_back(s);
    
    sprintf(s,"T0 of CORAL (--t0-ref) was %g ns",t0_ref);
    v.push_back(s);
    
    return v;
}

////////////////////////////////////////////////////////////////////////////////

void V::VStore(void *location, const std::string &table, const VFitResult &r)
{
    // OK, now we get the fit parameters names
    std::string residual_fit_names, residual_fit_results;
    if(1)
    {
        for( int i=0; i<3; i++ )
        {
            const V::Residual *residual=NULL;
            
            if( r.residuals_ref[i].GetFitFunc()!=NULL )
            {
                residual = &r.residuals_ref[i];
                for( int k=0; k<residual->GetFitFunc()->GetNpar(); k++ )
                {
                    char buf[222];
                    sprintf(buf,"`%s_%s` float, ",residual->GetName().c_str(),residual->GetFitFunc()->GetParName(k));
                    residual_fit_names += buf;
                }
            }

            if( r.residuals_corr[i].GetFitFunc()!=NULL )
            {
                residual = &r.residuals_corr[i];
                for( int k=0; k<residual->GetFitFunc()->GetNpar(); k++ )
                {
                    char buf[222];
                    sprintf(buf,"`%s_%s` float, ",residual->GetName().c_str(),residual->GetFitFunc()->GetParName(k));
                    residual_fit_names += buf;
                }
            }
        }
    }
    
    std::auto_ptr<char> buf(new char[residual_fit_names.length()+table.length()+r.program.length()+1000]);
    // Create function fit parameters
    sprintf(buf.get(),
                "CREATE TABLE IF NOT EXISTS `%s` ("
                "`detector`         varchar(100) NOT NULL default '',"
                "`chf`              int,    "
                "`chl`              int,    "
                "`pos`              int,    "
                "`delta`            int,    "
                "`comment`          varchar(55),"
                "`data`             int,    "
                "`Xi2`              float,  "
                "`T0`               float,  "
                "`T0_err`           float,  "
                "`W`                float,  "
                "`W_err`            float,  "
                " %s"
                " UNIQUE KEY `indx` (`detector`,`chf`,`chl`,`pos`,`delta`)) TYPE=MyISAM COMMENT='%s';",
                table.c_str(),residual_fit_names.c_str(),r.program.c_str());

    // Establish a communication with the DB.
    TSQLServer* db=static_cast<TSQLServer*>(location);
    if( !db->Query(buf.get()) )
    {
        printf("**** FAILED: %s\n",buf.get());
        throw "Error in communication with the DB.";
    }

    // We should check for a NaN values: MySQL does not understand insertion of them.
    float t0=r.t0, t0_err=r.t0_err, w0=r.w0, w0_err=r.w0_err;
    if( !finite(t0) || !finite(t0_err) )
    {
        t0=-1000;
        t0_err=0;
    }
    if( !finite(w0) || !finite(w0_err) )
    {
        w0=-1000;
        w0_err=0;
    }
    
    residual_fit_names=residual_fit_results="";
    for( int i=0; i<3; i++ )
    {
        if( r.residuals_ref[i].GetFitFunc()!=NULL )
        {
            const V::Residual *residual=&r.residuals_ref[i];
            
            for( int k=0; k<residual->GetFitFunc()->GetNpar(); k++ )
            {
                char buf[222];
                sprintf(buf,"%s_%s,",residual->GetName().c_str(),residual->GetFitFunc()->GetParName(k));
                residual_fit_names += buf;
                sprintf(buf,"%g,",residual->GetFitFunc()->GetParameter(k));
                residual_fit_results += buf;
            }
        }

        if( r.residuals_corr[i].GetFitFunc()!=NULL )
        {
            const V::Residual *residual=&r.residuals_corr[i];
            
            for( int k=0; k<residual->GetFitFunc()->GetNpar(); k++ )
            {
                char buf[222];
                sprintf(buf,"%s_%s,",residual->GetName().c_str(),residual->GetFitFunc()->GetParName(k));
                residual_fit_names += buf;
                sprintf(buf,"%g,",residual->GetFitFunc()->GetParameter(k));
                residual_fit_results += buf;
            }
        }
    }

    if( residual_fit_names!="" )
    {
        // remove the trailing ','
        residual_fit_names[residual_fit_names.length()-1]=' ';
        residual_fit_results[residual_fit_results.length()-1]=' ';
    }
    sprintf(buf.get(),"REPLACE INTO %s ("
                "detector,chf,chl,pos,delta,comment,data,Xi2,T0,T0_err,W,W_err,"
                "%s"
                ") VALUES('%s',%d,%d,%d,%d,'%s',%d,%g,%g,%g,%g,%g,%s);",
            table.c_str(),
            residual_fit_names.c_str(),
            r.detector.c_str(),
            r.channel_first,r.channel_last,r.pos,
            r.delta,r.comment.c_str(),
            r.vdata.size(),r.xi2,t0,t0_err,w0,w0_err,
            residual_fit_results.c_str());

    if( !db->Query(buf.get()) )
    {
        printf("**** FAILED: %s\n",buf.get());
        throw "Can not write to DB.";
    }
}

////////////////////////////////////////////////////////////////////////////////

TCanvas *V::MakeReport(const V::VFitResult &result)
{
    TH2 &hV = *const_cast<TH2*>(result.hV);

    printf("FIT: %s  points=%d  w=%g   T0=%g\n",hV.GetName(),result.vdata.size(),result.w0, result.t0 );

    char name[222];
    sprintf(name,"%s_canvas",hV.GetName());

    TCanvas *c=new TCanvas(name);

    if(1)
    {
        // Draw VFitResult
        TPaveText *label[2] = { new TPaveText(0.05,0.90,0.45,0.99),
                                new TPaveText(0.55,0.90,0.95,0.99) };
        const std::vector<std::string> v=result.MakeDescription();
        unsigned int l_split=(v.size()+1)/2;
        for( unsigned i=0; i<v.size(); i++ )
            label[i/l_split]->AddText(v[i].c_str());
        
        label[0]->Draw();
        label[1]->Draw();
    }

    TPad *pad_main = new TPad("pad_main","This is the main pad",0.02,0.62,0.98,0.88);
    pad_main->Draw();
    TPad *pad_1 = new TPad("pad_1","Uncorrected residuals",0.02,0.32,0.98,0.58);
    pad_1->Draw();
    TPad *pad_2   = new TPad("pad_2","Corrected residuals",  0.02,0.02,0.98,0.28);
    pad_2->Draw();

    pad_main->Divide(3,1);
    pad_main->cd(1);
    hV.Draw("COLZ");
    
    if( result.rt!=NULL )
    {
        result.rt->MakeGraph(result.w0)->Draw("SL*");
        
        if( result.hVf!=NULL )
        {
            pad_main->cd(2);
            result.hVf->Draw("COLZ");
            result.rt->MakeGraph(result.w0)->Draw("SL*");
        }
        
        // Draw the fit result on top of the histogram.
        if(1)
        {
            TH2 *h = (result.hVf==NULL?result.hV:result.hVf);
            float
                txt_x = h->GetXaxis()->GetBinCenter(1),
                txt_y = h->GetYaxis()->GetBinCenter(h->GetYaxis()->GetNbins()-5);

            TText *p = new TText();
            sprintf(name,"Tmin=%g +/- %g",result.t0,result.t0_err);
            p->DrawText(txt_x,txt_y,name);
            sprintf(name,"W0=%g +/- %g",result.w0,result.w0_err);
            p->DrawText(txt_x,txt_y-5,name);
        }

        pad_1->Divide(3,1);
        result.residuals_ref[0].MakeReport(pad_1->cd(1));
        result.residuals_ref[1].MakeReport(pad_1->cd(2));
        result.residuals_ref[2].MakeReport(pad_1->cd(3));
        
        pad_2->Divide(3,1);
        result.residuals_corr[0].MakeReport(pad_2->cd(1));
        result.residuals_corr[1].MakeReport(pad_2->cd(2));
        result.residuals_corr[2].MakeReport(pad_2->cd(3));
        
        //sprintf(name,"%s_time",hV.GetName());
        //TH1F *h2 = new TH1F(name,"time diff",100,-10,10);
        //FillTimeDiffPlot(*h2,result.vdata,*result.rt,result.w0);
        //h2->Draw();
        //FitTimeDiffPlot(*h2);
    }

    c->cd(0);

    return c;
}

////////////////////////////////////////////////////////////////////////////////

TH2 *V::MakeHistogram(const char *name, const char *title,const std::vector<V::VData> &vdata,float dx, float dt)
{
    float t_avr=0, x_avr=0;
    for( std::vector<V::VData>::const_iterator it=vdata.begin(); it!=vdata.end(); it++ )
    {
        x_avr += it->x;
        t_avr += it->t;
    }
    
    t_avr /= vdata.size();
    x_avr /= vdata.size();

    TH2F *h=new TH2F(name,title,100,x_avr-dx/2,x_avr+dx/2,100,t_avr-dt/2,t_avr+dt/2);
    h->SetDirectory(NULL);
    for( std::vector<V::VData>::const_iterator it=vdata.begin(); it!=vdata.end(); it++ )
        h->Fill(it->x,it->t);

    h->GetXaxis()->SetTitle("Distance [cm]");
    h->GetYaxis()->SetTitle("Time [ns]");
    
    return h;
}

////////////////////////////////////////////////////////////////////////////////

void V::FillTimeDiffPlot(TH1 &ht,const std::vector<VData> &vdata,const CS::RTRelation &rt,float w0)
{
    for( std::vector<VData>::const_iterator v=vdata.begin(); v!=vdata.end(); v++ )
    {
        try
        {
            ht.Fill(rt.GetT(v->x-w0)-v->t);
        }
        catch(...)
        {
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void V::FitTimeDiffPlot(TH1 &h)
{
    TF1 f("f","gaus(0)");

    f.SetParName(0,"Amplitude_{1}");
    f.SetParName(1,"Mean_{1}");
    f.SetParName(2,"Sigma_{1}");
    //f.SetParName(3,"Amplitude_{2}");
    //f.SetParName(4,"Mean_{2}");
    //f.SetParName(5,"Sigma_{2}");
    
    //f.SetParLimits(0,0,1e9);
    //f.SetParLimits(2,0.01,0.1);
    //f.SetParLimits(3,0,1e9);
    //f.SetParLimits(5,0.03,10);

    f.SetParameters(1,0,1,1,0,0.1);

    h.Fit("f","Q");
}

////////////////////////////////////////////////////////////////////////////////

V::Residual::~Residual(void)
{
    delete h;
    delete fit_func;
}

////////////////////////////////////////////////////////////////////////////////

void V::Residual::Book(const std::string &_name,const std::string &_title,unsigned int bins,float rmax)
{
    delete fit_func;
    fit_func=NULL;

    std::string name(_name);
    for( size_t i=0; i<name.length(); i++ )
        if( name[i]==' ' )
            name[i]='_';
        else
        if( name[i]=='_' )
            {}
        else
        if( !isalpha(name[i]) )
            throw "V::Residual::Book(): Bad name!";

    delete h;
    
    h = new TH1F(name.c_str(),_title.c_str(),bins,-rmax,rmax);
    h -> SetDirectory(NULL);
    h -> GetXaxis()->SetTitle("Distance [cm]");
    h -> GetYaxis()->SetTitle("Entries");
    h -> SetDirectory(NULL);
}

////////////////////////////////////////////////////////////////////////////////

void V::FillResidualPlots(VFitResult &result,float r_limit,unsigned int bins,float r_max)
{
    if( result.rt==NULL )
        throw "V::FillResidualPlots(): RT is unknown.";

    FillResidualPlot("resFIT",
                     &result.residuals_corr[0],&result.residuals_corr[1],&result.residuals_corr[2],
                     result.vdata,*result.rt,result.t0,result.w0,r_limit,bins,r_max);

    if( result.t0_ref!=0 )
    {
        FillResidualPlot("resCORAL",
                         &result.residuals_ref[0],&result.residuals_ref[1],&result.residuals_ref[2],
                         result.vdata,*result.rt,result.t0_ref,result.w0,r_limit,bins,r_max);
    }
}

////////////////////////////////////////////////////////////////////////////////

void V::FillResidualPlot(const std::string &name, Residual *res_all,Residual *res_left,Residual *res_right,
                         const std::vector<VData> &vdata,const CS::RTRelation &rt_orig,float t0,float w0,float r_limit,
                         unsigned int bins,float r_max)
{
    if( name!="" )
    {
        // We book the histograms here!
        if( res_all!=NULL )
            res_all->Book(name,name+" residuals",bins,r_max);
        if( res_left!=NULL )
            res_left->Book(name+"l",name+" residuals of the left leg",bins,r_max);
        if( res_right!=NULL )
            res_right->Book(name+"r",name+" residuals of the right leg",bins,r_max);
    }

    RTRelation *rt = rt_orig.Clone();
    rt->SetT0(t0);

    if( res_all!=NULL )
        res_all->points_taken = res_all->points_rejected = 0;
    if( res_left!=NULL )
        res_left->points_taken = res_left->points_rejected = 0;
    if( res_right!=NULL )
        res_right->points_taken = res_right->points_rejected = 0;

    for( std::vector<VData>::const_iterator v=vdata.begin(); v!=vdata.end(); v++ )
    {
        try
        {
            float
                r  = rt->GetR(v->t),
                rl = v->x-w0 + r,
                rr = v->x-w0 - r;
            r = fabs(rl)<fabs(rr) ? rl:rr;
            
            if( fabs(r)>r_limit )
                throw 1;
            
            if( fabs(rl)<fabs(rr) )
            {
                if( res_left!=NULL )
                {
                    res_left -> points_taken ++;
                    if( res_left->GetHist()!=NULL )
                        res_left->GetHist()->Fill(rl);
                }
            }
            else
            {
                if( res_right!=NULL )
                {
                    res_right -> points_taken ++;
                    if( res_right->GetHist()!=NULL )
                        res_right->GetHist()->Fill(rr);
                }
            }
            
            if( res_all!=NULL )
            {
                res_all -> points_taken ++;
                if( res_all->GetHist()!=NULL )
                    res_all->GetHist()->Fill(r);
            }
        }
        catch(...)
        {
            if( res_all!=NULL )
                res_all -> points_rejected ++;
            if( res_left!=NULL )
                res_left -> points_rejected ++;
            if( res_right!=NULL )
                res_right -> points_rejected ++;
        }
    }

    delete rt;

    if( res_all!=NULL && res_all->GetHist()!=NULL )
        res_all->Fit();
    if( res_left!=NULL && res_left->GetHist()!=NULL )
        res_left->Fit();
    if( res_right!=NULL && res_right->GetHist()!=NULL )
        res_right->Fit();
}

////////////////////////////////////////////////////////////////////////////////

void V::Residual::Fit(const std::string &func)
{
    if( h==NULL )
        throw "V::Residual::Fit() histogram does not exist.";

    delete fit_func;
    fit_func=NULL;

    char buf[222];
    sprintf(buf,"f_%s",h->GetName());

    if( func=="" )
    {
        // To be used for the root function creation.
        Double_t (*fptr)(const Double_t*, const Double_t*);
        fptr=NULL;

        // Create a holder (TF1) of the fit results.
        fit_func = new TF1(buf,fptr,0.,1.,10);

        // Parameter number.
        int np=0;

         //Resolution defined as RMS +-750micrometers
         h->GetXaxis()->SetRangeUser(-0.075,0.075);
         fit_func->SetParName(np,"RMS_750");   
         fit_func->SetParameter(np,h->GetRMS(1));    
         np++;     
         fit_func->SetParName(np,"MEAN");   
         fit_func->SetParameter(np,h->GetMean(1));    
         np++;     

         // "No" more axis restriction
         //h->GetXaxis()->SetRangeUser(h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax());
    }
    else
    if( func=="gaus" )
    {
        fit_func = new TF1(buf,"gaus",0,1);
        fit_func -> SetParameters(10,0,0.02);
        //fit_func -> FixParameter(1,0);
        fit_func -> SetParLimits(1,-1e-4,1e-4);
        h->Fit(fit_func,"Q");
    }
    else
        throw "V::Residual::Fit(): Unknown function.";
}

////////////////////////////////////////////////////////////////////////////////

void V::Residual::MakeReport(TVirtualPad *pad) const
{
    if( h==NULL )
        return;
    pad->cd();
    h->Draw();
}

////////////////////////////////////////////////////////////////////////////////

bool V::read_data(std::istream &is,std::vector<V::VData> &data)
{
    fflush(stdout);

    data.empty();

    std::string line;
    int n=-1;
    while( getline(is,line) )
    {
        if( line.size()==0 || line[0]=='#' )
            continue;

        if( n==-1 )
        {
            char tmp;
            if( 1!=sscanf(line.c_str(),"%d %c",&n,&tmp) )
                throw "V::read_data(): Bad format, a single number is expected!";
        }
        else
        {
            assert(n>0);
        
            V::VData v;
            if( 3!=sscanf(line.c_str(),"%g %g %g",&v.x,&v.t,&v.w) )
                throw "V::read_data(): Bad format! (x,t,w)  numbers are expected!";
            else
                data.push_back(v);
            n--;

            if( n==0 )
            {
                // Finish data reading!
                return true;
            }
        }
    }

    // End of data
    return false;
}

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
