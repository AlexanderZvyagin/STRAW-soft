#include <stdexcept>

#include "TMinuit.h"
#include "TTree.h"
#include "TEventList.h"
#include "TChain.h"
#include "TFile.h"
#include "TH2.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TPavesText.h"
#include "TSQLServer.h"

#include "VS.h"
#include "RTRelationGrid.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

extern int minuit_printout;

namespace {

V::VFitResult *_res_=NULL;

void f_space(Int_t &np, Double_t *g, Double_t &fr, Double_t *x, Int_t flag)
{
    if( _res_==NULL )
        throw "There is no fit data!";

    const double w0=x[0], t0=x[1], xe=0.02, b=0;
    float n=0;

    fr=0;
    _res_->rt->SetT0(t0);

    for( vector<V::VData>::const_iterator it=_res_->vdata.begin(); it!=_res_->vdata.end(); it++ )
    {
        if( it->w<=b )
            continue;   // background!
        
        if( fabs(it->x-w0)<_res_->V_center_coridor )
            continue;
        
        try
        {
        
            double d = fabs(_res_->rt->GetR(it->t)-fabs(it->x-w0));

            if( d < _res_->V_leg_max_dist )
            {
                fr += pow(d/xe,2)*(it->w-b);
                n += it->w-b;
                
                if(0)
                    printf("[r=%g,x-r=%g] V(x=%g,t=%g,w=%g) =>  (w0=%g,t0=%g)  =>  df=%g\n",
                            _res_->rt->GetR(it->t),_res_->rt->GetR(it->t)+w0 - it->x,
                            it->x,it->t,it->w,w0,x[1],d*d*(it->w-b));
            }
        }
        catch(...)
        {
        }
    }
    
    fr /= n;
    
    if( _res_->V_fit_max_points )
        fr *= 1 +
              _res_->V_fit_max_points * fabs(_res_->vdata.size()-n)/float(_res_->vdata.size()+n);
}

void f_time(Int_t &np, Double_t *g, Double_t &fr, Double_t *x, Int_t flag)
{
    if( _res_==NULL )
        throw "There is no fit data!";

    double w0=x[0], t0=x[1], te=1, b=0;
    float n=0;

    fr=0;
    _res_->rt->SetT0(t0);

    for( vector<V::VData>::const_iterator it=_res_->vdata.begin(); it!=_res_->vdata.end(); it++ )
    {
        if( it->w<=b )
            continue;   // background!
        
        if( fabs(it->x-w0)<_res_->V_center_coridor )
            continue;
        
        try
        {
            double d = fabs(_res_->rt->GetT(fabs(it->x-w0)) - it->t);

            if( fabs(d) < _res_->V_leg_max_dist )
            {
                fr += pow(d/te,2) * (it->w-b);
                n += it->w-b;
                
                if(0)
                    printf("[r=%g,x-r=%g] V(x=%g,t=%g,w=%g) =>  (w0=%g,t0=%g)  =>  df=%g\n",
                            _res_->rt->GetR(it->t),_res_->rt->GetR(it->t)+w0 - it->x,
                            it->x,it->t,it->w,w0,x[1],d*d*(it->w-b));
            }
        }
        catch(...)
        {
        }
    }
    
    fr /= n;
}

void fcn_t0(Int_t &np, Double_t *g, Double_t &fr, Double_t *x, Int_t flag)
{
    if( _res_==NULL )
        throw "There is no fit data!";

    const double w0=x[0], t0=x[1], slope=x[2];

    fr=0;

    for( vector<V::VData>::const_iterator it=_res_->vdata.begin(); it!=_res_->vdata.end(); it++ )
        fr += pow( fabs(it->x-w0)*slope - (it->t-t0), 2 );
    
    fr /= _res_->vdata.size();
}

void prefit_T0_w0(const V::VFitResult &result,double &t0,double &w0)
{
    int ierflg=0;
    Double_t arglist[10];
    TMinuit minuit(3);

    float t0_start=t0;
    if( t0_start==0 )
    {
        // -- Simple calculation of a start point for T0 fit.
        for( std::vector<V::VData>::const_iterator d=result.vdata.begin();
             d!=result.vdata.end(); d++ )
        {
            t0_start += d->t;
        }
        t0_start /= result.vdata.size();
    }

    // -- Set the debug printout level by the fitting code.
    arglist[0]=minuit_printout;
    minuit.mnexcm("SET PRINTOUT", arglist ,1,ierflg);
    
    if( minuit_printout==-1 )
        minuit.mnexcm("SET NOWARNINGS", arglist ,0,ierflg);

    // -- Declare variables for the fit.
    minuit.DefineParameter(0,"w0",w0,0.1,-0.15,0.15);
    minuit.DefineParameter(1,"T0",t0_start,1,t0_start-_res_->rt->GetTMax(),t0_start+_res_->rt->GetTMax());
    minuit.DefineParameter(2,"slope",1,1,-1e9,1e9);

    // -- Set the fit function.
    minuit.SetFCN(fcn_t0);

    minuit.mnexcm("MINIMIZE", arglist ,0,ierflg);
    //minuit.mnmnos();

    // -- Read fit results.

    double f_min,par, err;

    minuit.mnstat(f_min,par,par,ierflg,ierflg,ierflg);

    minuit.GetParameter(0,w0,err);
    minuit.GetParameter(1,t0,err);
}

}

////////////////////////////////////////////////////////////////////////////////

CS::RTRelationGrid *CreateFixedRT(const CS::RTRelation &rt,float rmax,vector<float> r=vector<float>())
{
    if( rmax==0 )
    {
        printf("CreateFixedRT(): maximum distance for RT is taken from the fit.\n");
        rmax = rt.GetRMax();
        r.push_back(rmax);
    }
    else
    {
        if( rmax>rt.GetRMax() )
            printf("CreateFixedRT(): maximum distance will be enlarged by %g:  %g=>%g\n",
                    rmax-rt.GetRMax(),rt.GetRMax(),rmax);
        if( !r.empty() && rmax!=r.back() )
            throw invalid_argument("CreateFixedRT(): rmax must be zero or equal to the last argument of 'r' list.");
    }
    
    if( r.size()<=1 )
    {
        r.clear();
        r.push_back(0.0*rmax);
        r.push_back(0.1*rmax);
        r.push_back(0.2*rmax);
        r.push_back(0.5*rmax);
        r.push_back(0.8*rmax);
        r.push_back(0.9*rmax);
        r.push_back(1.0*rmax);
    }

    char buf[222];
    
    sprintf(buf,"RT-Grid T0=%g",rt.GetT0());
    
    for( unsigned int i=0; i<r.size(); i++ )
    {
        float
            x = r[i],
            t = 0,
            e = 0;
        try
        {
            // Try to calculate time from the RT.
            t = rt.GetT(x);
            e = rt.GetE(x);
        }
        catch(const std::exception &eee)
        {
            printf("Workaround: %s:   x=%g   max=%g\n",eee.what(),x,rt.GetRMax());
            
            // We failed to calculate RT point because the original
            // 'rt' does not cover [0,rmax] range. Make an extrapolation then.

            float
                x1  = rt.GetRMax()*0.99,
                x2  = rt.GetRMax(),
                t1  = rt.GetT(x1),
                t2  = rt.GetT(x2),
                e1  = rt.GetE(x1),
                e2  = rt.GetE(x2);
            t = t2 + (x-x2)/(x2-x1)*(t2-t1);
            e = e2 + (x-x2)/(x2-x1)*(e2-e1);
        }

        sprintf(buf+strlen(buf)," %g:%g:%g",x,t-rt.GetT0(),e);
    }
    
    return new CS::RTRelationGrid(buf);
}

////////////////////////////////////////////////////////////////////////////////

float VS::GetT0(TH1 &h_p)
{
    TCanvas *tmp=new TCanvas;
    float peak=h_p.GetBinCenter(h_p.GetMaximumBin());

    TF1 *f=new TF1("f","[2]+x*[3]+(x>[0])*(x-[0])*[1]");
    f->SetParName(0,"Tmin");
    f->SetParName(1,"T slope");
    f->SetParName(2,"bkg const");
    f->SetParName(3,"bkg slope");
    f->SetParameters(peak-30,100,0,0.01);
    f->SetParLimits(1,1.5,500);
    f->SetParLimits(2,0,10000000);

    char opts[11]="QE";
    //if( !draw )
    //    sprintf(opts+strlen(opts),"N");
    h_p.Fit(f,opts,"",peak-150,peak-3);
    
    delete tmp;
        
    return f->GetParameter(0);
}

////////////////////////////////////////////////////////////////////////////////

class VV
{
  public:
    float w0, w, s, t, a, b, fit;
};

/*! Function used in V-plot fitting */
Double_t function_gg(Double_t *x, Double_t *p)
{
    Double_t a=p[0], w0=p[1], w=p[2], s=p[3], b=p[4], b2a=p[5], b2s=p[6];
    return a/sqrt(2*TMath::Pi())/s*(TMath::Gaus(*x-w0,-w,s)+TMath::Gaus(*x-w0,w,s))
           + b
           + b2a/sqrt(2*TMath::Pi())/b2s*TMath::Gaus(*x-w0,0,b2s);
}

TGraphErrors *VS::FitV(TH2 &h, V::VFitResult &result)
{
    if( result.dt<=0 )
        throw "VS::CalculateRT(): unknwon maximum drift time.";
    
    delete result.hT;
    result.hT=h.ProjectionY();
    result.hT->GetXaxis()->SetTitle("Time [ns]");
    result.hT->GetYaxis()->SetTitle("Events");

    result.t0 = VS::GetT0(*result.hT);
    float T0=result.t0;

    float
        t_start = T0+5,
        t_end   = T0+result.dt;
    
    TF1 f_bkg("bkg","pol0");
    TF1 f_g1("g1","gaus(0)+pol0(3)");
    TF1 f_gg("function_gg",function_gg,h.GetXaxis()->GetXmin(),h.GetXaxis()->GetXmax(),7);
    f_gg.SetParName(0,"leg events");
    f_gg.SetParName(1,"wire position");
    f_gg.SetParName(2,"leg position");
    f_gg.SetParName(3,"resolution");
    f_gg.SetParName(4,"constant bkg");
    f_gg.SetParName(5,"gaus bkg A");
    f_gg.SetParName(6,"gaus bkg S");
    f_gg.SetParLimits(0,1,100000);
    f_gg.SetParLimits(1,h.GetXaxis()->GetXmin(),h.GetXaxis()->GetXmax());
    f_gg.SetParLimits(2,0,h.GetXaxis()->GetXmax());
    f_gg.SetParLimits(3,h.GetXaxis()->GetBinWidth(1),(h.GetXaxis()->GetXmax()-h.GetXaxis()->GetXmin())/10);
    f_gg.SetParLimits(4,0,100000);
    f_gg.SetParLimits(5,0,100000);
    f_gg.SetParLimits(6,0.4,100);

    char name[111];
    sprintf(name,"%s_RT_fit_status",result.detector.c_str());
    TCanvas *canvas = new TCanvas(name,h.GetTitle(),700,900);
    canvas->Divide(1,2);
    canvas->cd(1);
    h.Draw("colz");
    canvas->cd(2);
    TVirtualPad *pad_lr = canvas->GetPad(2);

    float
        res = h.GetXaxis()->GetBinWidth(1)*h.GetXaxis()->GetNbins()*0.07,
        time_resolution = h.GetYaxis()->GetBinWidth(1)*1/sqrt(12.);
    VV *vvs=new VV[h.GetYaxis()->GetNbins()], *vvs_it=vvs;

    for( int i=1; i<=h.GetYaxis()->GetNbins(); i++ )
    {
        if( h.GetYaxis()->GetBinCenter(i)<t_start ||
            h.GetYaxis()->GetBinCenter(i)>t_end )
            continue;

        TH1D *proj = h.ProjectionX("vv",i,i);
        proj->GetXaxis()->SetTitle("Distance [cm]");
        proj->GetYaxis()->SetTitle("Events");

        // Get background
        proj->Fit("bkg","0Q");

        float peak_max[2]={0,0}, ww[2]={0,0};  // left and right peaks
        for( int k=1; k<=proj->GetXaxis()->GetNbins(); k++ )
        {
            float x = proj->GetXaxis()->GetBinCenter(k);
            int p = x>result.w0;  // left or right leg
            if( peak_max[p]>=proj->GetBinContent(k) )
                continue;
            peak_max[p] = proj->GetBinContent(k);
            ww[p] = x;
        }
        
        if( peak_max[0]==0 || peak_max[1]==0 )
        {
            printf("peak position was not found!\n");
            peak_max[0]=peak_max[1]= peak_max[0]+peak_max[1]+1;
            ww[0]=ww[1]=result.w0;
        }

        vvs_it->w  = (-ww[0]+ww[1])/2;
        vvs_it->w0 = ( ww[0]+ww[1])/2;
        
        vvs_it->t = h.GetYaxis()->GetBinLowEdge(i),
        // preliminary values
        vvs_it->s = res;
        vvs_it->b = f_bkg.GetParameter(0);

        pad_lr->cd();

        // Find last good fit
        VV *vgf=vvs_it;
        while( --vgf>=vvs )
            if( vgf->fit>0.1 && vgf->fit<2 )
                break;
        
        // For new fit we either use the previous fit point
        // or (if fit failed) we calculate the start point
        if( vgf>=vvs )
        {
            f_gg.SetParameters(vgf->a,vvs_it->w0,vvs_it->w,vgf->s,vgf->b);
            //printf("*** VALUES FROM A PREVIOUS FIT WILL BE USED! ***\n");
        }
        else
            f_gg.SetParameters(peak_max[0],vvs_it->w0,vvs_it->w,res,vvs_it->b,1,0.3);

        //for( int j=0; j<f_gg.GetNpar(); j++ )
        //    printf("par[%2d]  =  %g\n",j,f_gg.GetParameter(j));

        proj->Fit(&f_gg,"Q");
        
        // Get the fit results
        vvs_it->a  = f_gg.GetParameter(0);
        vvs_it->w0 = f_gg.GetParameter(1);
        vvs_it->w  = f_gg.GetParameter(2);
        vvs_it->s  = f_gg.GetParameter(3);
        vvs_it->b  = f_gg.GetParameter(4);
        if( f_gg.GetNDF()>0 )
            vvs_it->fit = f_gg.GetChisquare()/f_gg.GetNDF();
        else
            vvs_it->fit = 100;
        vvs_it++;
        
        canvas->cd(1);
        h.Draw("colz");

        // Draw the time range beeing fit on the histogram plot
        float
            w1 = h.GetXaxis()->GetXmin(),
            w2 = h.GetXaxis()->GetXmax(),
            t1 = h.GetYaxis()->GetBinLowEdge(i),
            t2 = h.GetYaxis()->GetBinUpEdge(i);
        TLine l_t1(w1,t1,w2,t1), l_t2(w1,t2,w2,t2);
        l_t1.Draw();
        l_t2.Draw();
        canvas->cd(0);
        canvas->Update();

        if(1)
        {
            char name[111];
            sprintf(name,"%s_V_tbin%d",result.detector.c_str(),i);
            canvas->Write(name);
        }

        // The work with the time slice has finished. We do not need it any more.
        delete proj;
    }
    
    delete canvas;

    //////////////////////////
    // Next we calculate w0 //
    //////////////////////////

    result.w0=0;   // obtained wire position
    
    int NVV=vvs_it-vvs;
    TGraphErrors *g_rec = new TGraphErrors;
    g_rec->SetName("RT_rec");
    g_rec->GetXaxis()->SetTitle("Distance to wire [cm]");
    g_rec->GetYaxis()->SetTitle("Time [ns]");

    g_rec->SetPoint(0,0,T0);  // The error will be added later! (another function)
    g_rec->SetPointError(0,0,time_resolution);  // The error will be added later! (another function)

    for( int i=0; i<NVV; i++ )
    {
        //printf("raw: i=%d   w0=%g  t=%g  fit=%g\n",i,vvs[i].w0, vvs[i].t,vvs[i].fit);
        
        if( vvs[i].fit>6 )
            continue;       // Bad fit
        
        // Stop if we reached the straw wall.
        if( result.r>0 && vvs[i].w>result.r )
            break;

        g_rec->SetPoint(g_rec->GetN(),vvs[i].w, vvs[i].t);
        g_rec->SetPointError(g_rec->GetN()-1,vvs[i].s,time_resolution);
        result.w0 += vvs[i].w0;
    }
    if( g_rec->GetN()>0 )
        result.w0 /= g_rec->GetN();

    delete [] vvs;
    
    g_rec->SetPoint(0,0,T0);

    // And finally set the error for point at T0.
    //if( g_rec->GetN()>=2 )
    //    g_rec->SetPointError(0,1.5*g_rec->GetErrorX(1),1.5*g_rec->GetErrorY(1));

    return g_rec;
}

////////////////////////////////////////////////////////////////////////////////

/*! Draw RT-fit result (two legs) on the top of V-plot,
    taking into account two legs and wire position
*/
void DrawFit(TGraphErrors &g_rec,float w0)
{
    TGraphErrors *g=new TGraphErrors;
    g->GetXaxis()->SetTitle("Distance to wire [cm]");
    g->GetYaxis()->SetTitle("Drift time [ns]");

    for( int i=0; i<g_rec.GetN(); i++ )
    {
        g->SetPoint(g->GetN(),w0-g_rec.GetX()[i],g_rec.GetY()[i]);
        g->SetPointError(g->GetN()-1,g_rec.GetErrorX(i),g_rec.GetErrorY(i));

        g->SetPoint(g->GetN(),w0+g_rec.GetX()[i],g_rec.GetY()[i]);
        g->SetPointError(g->GetN()-1,g_rec.GetErrorX(i),g_rec.GetErrorY(i));

    }
    g->Draw("S");
}

////////////////////////////////////////////////////////////////////////////////

TGraphErrors *merge_3points(TGraphErrors &g)
{
    if( g.GetN()<2 )
        throw "merge_3points(): empty graph";

    TGraphErrors *gg=new TGraphErrors();
    gg->SetName("g3_merge_3points");
    gg->GetXaxis()->SetTitle("Distance to wire [cm]");
    gg->GetYaxis()->SetTitle("Drift time [ns]");

    double w3, w3e, t3, t3e;
    // Point 0 we just copy.
    g.GetPoint(0,w3,t3);
    gg->SetPoint(0,w3,t3);
    gg->SetPointError(0,g.GetErrorX(0),g.GetErrorY(0));

    w3=0;
    t3=0;
    w3e=0;
    t3e=0;

    // Now we skip the very first (0,0) point.
    for( int i=1; i<g.GetN(); i++ )
    {
        double w,t;
        g.GetPoint(i,w,t);
        //printf(" (%d,%g,%g) ",i,w,t);
        w3  += w;
        t3  += t;
        w3e += g.GetErrorX(i);
        t3e += pow(g.GetErrorY(i),2);
        
        if( i%3==0 )
        {
            gg->SetPoint(gg->GetN(),w3/3,t3/3);
            gg->SetPointError(gg->GetN()-1,w3e/3,sqrt(t3e));
            //printf(" =>  (%d,%g,%g)\n",gg->GetN()-1,w3/3,t3/3);
            w3=0;
            t3=0;
            w3e=0;
            t3e=0;
        }
    }

    return gg;
}

////////////////////////////////////////////////////////////////////////////////

char *coral_old_RT(CS::RTRelationGrid &rt)
{
    assert( rt.GetPointsR().size()==rt.GetPointsT().size() );

    static char buf[1111];
    float res=0;
    for( unsigned int i=1; i<rt.GetPointsE().size(); i++ )
        res += rt.GetPointsE()[i];

    sprintf(buf,"T_{min}=%g\navr. spatial res. is %.0f #mu m\nRTGrid\n",
            rt.GetT0(),10000*res/(rt.GetPointsE().size()-1));

    for( unsigned int i=0; i<rt.GetPointsR().size(); i++ )
        sprintf(buf+strlen(buf),"%6.3f  %6.3f   %6.3f\n",
                rt.GetPointsT()[i],
                rt.GetPointsR()[i],
                rt.GetPointsE()[i]);

    return buf;
}

////////////////////////////////////////////////////////////////////////////////

void TPavesText_add_text(TPavesText &p,char *txt)
{
    char *buf = new char[strlen(txt)+1];
    strcpy(buf,txt);

    for( char *s1=buf; *s1!=0; )
    {
        char *s2 = strchr(s1,'\n');
        
        // replace '\n' with end of string
        if( s2!=NULL )
            *s2=0;

        p.AddText(s1);
        
        if( s2==NULL )
            break;
        s1=s2+1;
    }

    delete [] buf;
}

////////////////////////////////////////////////////////////////////////////////

void VS::CalculateRT(V::VFitResult &result,float dx,const vector<float> &rrr)
{
    delete result.rt;
    result.rt = NULL;

    char name[111], title[111];
    sprintf(name,"%s_V",result.detector.c_str());
    sprintf(title,"V-plot for %s chf=%d chl=%d   region=[%d-%d,%d+%d]",
            result.detector.c_str(),result.channel_first,result.channel_last,
            result.pos,result.delta,result.pos,result.delta);

    delete result.hV;   // remove the previous histogram
    result.hV = MakeHistogram(name,title,result.vdata,dx*1.1,result.dt*1.4);

    sprintf(name,"%s_chf%d_chl%d_delta%d_report",
            result.detector.c_str(),
            result.channel_first,
            result.channel_last,
            int(result.delta));
    TCanvas *canvas = new TCanvas(name,title,600,900);
    canvas->Divide(2,4);

    result.hV->Write();

    result.w0 = 0;  // starting point of the fit

    // Now we make fit in every time bin
    TGraphErrors *gr_rec = FitV(*result.hV,result);
    sprintf(name,"RT_raw_%s_chf%d_chl%d_delta%d",
            result.detector.c_str(),
            result.channel_first,
            result.channel_last,
            int(result.delta));
    gr_rec->SetName(name);
    gr_rec->Write();
    
    TGraphErrors *gr_smooth = merge_3points(*gr_rec);
    sprintf(name,"RT_smooth_%s_chf%d_chl%d_delta%d",
            result.detector.c_str(),
            result.channel_first,
            result.channel_last,
            int(result.delta));
    gr_smooth->SetName(name);
    gr_smooth->Write();
    
    for( int i=1; i<gr_smooth->GetN(); i++ )
    {
        double w[2], t[2];
        gr_smooth->GetPoint(i-1,w[0],t[0]);
        gr_smooth->GetPoint(i  ,w[1],t[1]);
        //if( w[0]>=w[1] || t[0]>=t[1] )
        //    printf("smoothing problem? i=%d=(%g,%g)  i=%d=(%g,%g)\n",i-1,w[0],t[0],i,w[1],t[1]);
        //printf("Smooth:  i=%d  w=%g  t=%g\n",i,w[0],t[0]);
    }
    
    
    //TGraphErrors *rt_grid = cg(*gr_rec,T0,RT_grid_points);

    // V-plot and fit result
    canvas->cd(1);
    result.hV->Draw("colz");
    DrawFit(*gr_rec,result.w0);        // draw result of the fit

    // Time projection and T0
    canvas->cd(2);
    result.hT->Draw();

    canvas->cd(3);
    gr_smooth->Draw("ACP");
    gr_smooth -> GetHistogram() -> SetName(gr_smooth->GetName());
    gr_smooth -> GetHistogram() -> SetTitle(gr_smooth->GetTitle());
    gr_smooth -> GetHistogram() -> GetXaxis() -> SetTitle("Distance to wire [cm]");
    gr_smooth -> GetHistogram() -> GetYaxis() -> SetTitle("Drift time [ns]");
    gr_smooth -> GetHistogram() -> GetYaxis() -> SetTitleOffset(1.5);

    // Resolution versus distance
    canvas->cd(4);
    delete result.gResolution;
    sprintf(name,"%s_res",result.detector.c_str());
    sprintf(title,"%s resolution versus distance to wire",result.detector.c_str());
    result.gResolution = new TGraph;
    result.gResolution -> SetName(name);
    result.gResolution -> SetTitle(title);
    char rt_grid[2000]="RT-Grid"; // string to be used in RT creation.

    for( int i=0; i<gr_smooth->GetN(); i++ )
    {
        // Load the next point.
        double x,y;
        gr_smooth->GetPoint(i,x,y);

        float t=y-result.t0;
        // For the very first point the distance must be 0, time must be 0.
        if( i==0 )
            x=t=0;

        if( t<result.dt )
        {
            result.gResolution->SetPoint(result.gResolution->GetN(),x,gr_smooth->GetErrorX(i)*10000);
            sprintf(rt_grid+strlen(rt_grid)," %g:%g:%g",x,t,gr_smooth->GetErrorX(i));
        }
    }
    result.gResolution -> SetMarkerStyle(8);
    result.gResolution -> SetMinimum(0);
    result.gResolution -> Draw("AP");
    result.gResolution -> GetHistogram() -> SetName(result.gResolution->GetName());
    result.gResolution -> GetHistogram() -> SetTitle(result.gResolution->GetTitle());
    result.gResolution -> GetHistogram() -> GetXaxis() -> SetTitle("Distance to wire [cm]");
    result.gResolution -> GetHistogram() -> GetYaxis() -> SetTitle("Resolution [#mu m]");
    result.gResolution -> GetHistogram() -> SetMaximum(400);
    result.gResolution -> GetHistogram() -> GetYaxis() -> SetTitleOffset(1.5);


    canvas->cd(5);
    sprintf(name,"%s_V_tbin45",result.detector.c_str());
    TObject *o=gDirectory->Get(name);
    if( o!=NULL )
    {
        TCanvas *ccc=dynamic_cast<TCanvas*>(o);
        if( ccc!=NULL )
        {
            TIter next(ccc->GetListOfPrimitives());
            while( TObject *obj=next() )
            {
                canvas->cd(5);
                obj->Draw();
            }
        }
        else
            printf("Not a TCanvas??\n");
    }

    try
    {
        canvas->cd(7);

        // OK, create RT-grid with the smoothed points.
        CS::RTRelationGrid _rt(rt_grid);
        _rt.SetT0(result.t0);

        // Now we convert this RT to the 'fixed format'.
        CS::RTRelationGrid *rt_fixed = CreateFixedRT(_rt,result.r,rrr);
        result.rt = rt_fixed;
        TGraph *gr_final = result.rt->MakeGraph(result.w0);
        sprintf(name,"RT_final_%s_chf%d_chl%d_delta%d",
                result.detector.c_str(),
                result.channel_first,
                result.channel_last,
                int(result.delta));
        gr_final->SetName(name);
        gr_final->SetMarkerStyle(8);
        result.hV->Draw("colz");

        result.hV->Draw("colz");

        gr_final->Draw("PC");
        gr_final->Write();

        // Create the status report
        canvas->cd(8);
        TPavesText *rt_print = new TPavesText(0.01,0.01,0.99,0.99);
        rt_print->SetTextSize(0.05);
        TPavesText_add_text(*rt_print,coral_old_RT(*rt_fixed));
        rt_print->Draw();
        printf("%s\n",coral_old_RT(*rt_fixed));

        // Residual plot
        canvas->cd(6);
        delete result.residuals_corr[0].h;
        sprintf(name,"%s_residual",result.detector.c_str());
        sprintf(title,"V-plot residual for %s chf=%d chl=%d   region=[%d-%d,%d+%d]",
                result.detector.c_str(),result.channel_first,result.channel_last,
                result.pos,result.delta,result.pos,result.delta);
        result.residuals_corr[0].h = new TH1F(name,title,200,-dx/5,dx/5);
        result.residuals_corr[0].h -> GetXaxis() -> SetTitle("Distance [cm]");
        result.residuals_corr[0].h -> GetYaxis() -> SetTitle("Events");
        FillResidualPlot("",&result.residuals_corr[0],NULL,NULL,result.vdata,*result.rt,result.t0,result.w0);
        result.residuals_corr[0].Fit();
    }
    catch(const char *e)
    {
        printf("%s\n",e);
    }
    catch(const std::exception &e)
    {
        printf("%s\n",e.what());
    }

    // Write the report.
    canvas->Write();
    canvas->Print("","eps");
    
    delete canvas;  // release the memory.
}

////////////////////////////////////////////////////////////////////////////////

void VS::VFit(VFitResult &result)
{
    // -- A simple check to speed-up fit failer for empty data sets.
    if( result.vdata.size()<10 )
    {
        result.t0       = 0;
        result.t0_err   = 0;
        result.w0       = 0;
        result.w0_err   = 0;
        result.success  = -1;
        return;
    }

    // -- Set data to be used
    _res_ = &result;

    // -- Depending on the 'result.rt' we either fit with the fixed RT
    // -- or calculate RT from the V-plot.

    if( result.rt==NULL )
    {
        CalculateRT(result,result.r*2,result.rt_r);
        
        printf("*******************************************************\n");
        if( result.rt!=NULL )
        {
            printf("The RT is:\n");
            printf("%s\n",string(*result.rt).c_str());
        }
        else
            printf("RT calculation has failed!\n");
        printf("*******************************************************\n");
        

        return;
    }

    // -- Declare variables for the fit code.
    Double_t arglist[10];
    Int_t ierflg;
    TMinuit minuit(2);

    double t0_start=result.t0_start, w0_start=result.w0_start;
    if( result.t0_start==0 )
    {
        prefit_T0_w0(result,t0_start,w0_start);
        printf("After pre-fit: T0=%g w0=%g\n",t0_start,w0_start);
    }

    // -- Set the debug printout level by the fitting code.
    arglist[0]=minuit_printout;
    minuit.mnexcm("SET PRINTOUT", arglist ,1,ierflg);
    
    if( minuit_printout==-1 )
        minuit.mnexcm("SET NOWARNINGS", arglist ,0,ierflg);

    // -- Declare variables for the fit.
    minuit.DefineParameter(0,"w0",w0_start,0.1,-0.15,0.15);
    minuit.DefineParameter(1,"t0",t0_start,1,t0_start-_res_->rt->GetTMax(),t0_start+_res_->rt->GetTMax());
    //minuit.DefineParameter(2,"xe",0.1,0.1,0.01,0.05);
    //minuit.DefineParameter(3,"b",1,1,0,15);

    // -- Set the fit function.
    if( result.fit_function=="time" )
        minuit.SetFCN(f_time);
    else
        minuit.SetFCN(f_space);

    // -- Fit! We make a few iterations here until fit succeeds.
    ierflg=4;   // Starting value for the loop
    for( int iter=0; iter<3 && ierflg==4; iter++ )
    {
        if( iter>0 && minuit_printout>=0 )
        {
            printf("It seems that the minimization has failed... But we try again! Iteration %d\n",iter+1);
            //minuit.mnexcm("SEEK", arglist ,0,ierflg);
        }

        arglist[0] = 1000; // Call limit
        minuit.mnexcm("MINIMIZE", arglist ,1,ierflg);
    }
    //minuit.mnmnos();
    arglist[0]=1000;
    minuit.mnexcm("MINOs", arglist, 1,ierflg);

    // -- Read fit results.

    result.success = ierflg;
      
    double f_min,par, err;

    minuit.mnstat(f_min,par,par,ierflg,ierflg,ierflg);
    result.xi2 = f_min;

    minuit.GetParameter(1,par,err);
    _res_->rt->SetT0(par);
    result.t0=par;
    result.t0_err=err;

    minuit.GetParameter(0,par,err);
    result.w0=par;
    result.w0_err=err;
}

////////////////////////////////////////////////////////////////////////////////

void VS::VCreate(const char *src,VFitResult &result)
{
    auto_ptr<TFile> f(TFile::Open(src));
    if( f.get()==NULL || !f->IsOpen() )
        throw "VS::VCreate(): Can not open file.";
    
    char s[222];
    sprintf(s,"%s_CORAL",result.detector.c_str());

    TObject *o=f->Get(s);
    if( o==NULL )
        throw "Can not find ntuple.";

    TTree *nt=dynamic_cast<TTree*>(o);
    if( nt==NULL )
        throw "Object is not TTree!";

    // Create cuts
    if( result.channel_first==result.channel_last )
        sprintf(s,"ch==%d",result.channel_first);
    else
        sprintf(s,"ch>=%d&&ch<=%d",result.channel_first,result.channel_last);
    if( result.delta>0 )
        sprintf(s+strlen(s),"&&abs(wy-%d)<%d",result.pos,result.delta);
    if( result.cuts.size()>0 )
        sprintf(s+strlen(s),"&&%s",result.cuts.c_str());
    
    // Now make the events selection
    nt->Draw(">>elist",s);

    // Load events selection
    TEventList *elist = (TEventList*)gDirectory->Get("elist");
    
    // Tell to the TTree to use it
    nt->SetEventList(elist);

    // Set addresses of variables we need to analyse.
    float ch,chp,d,wy,t;
    nt->SetBranchAddress("ch",&ch);
    nt->SetBranchAddress("chp",&chp);
    nt->SetBranchAddress("wy",&wy);
    nt->SetBranchAddress("d",&d);
    nt->SetBranchAddress("t",&t);

    // Loop only over selected events.
    for( int i=0; i<elist->GetN(); i++ )
    {
        if( nt->GetEvent(elist->GetEntry(i))<=0 )
            printf("VS::VCreate(): Can not read all events from the ntuple!\n");

        result.vdata.push_back( VData() );
        result.vdata.back().x = d;
        result.vdata.back().t = t;
        result.vdata.back().w = 1;
        
        if( result.signal_velocity>0 )
        {
            float dt = -wy/result.signal_velocity;
            if( chp>0.5 )
                dt = -dt;
            result.vdata.back().t += dt;
        }
    }

    // Free memory.
    delete elist;
}

////////////////////////////////////////////////////////////////////////////////

extern "C" {

V *VConstruct(void)
{
    return new VS;
}

void VDestroy(V *v)
{
    delete v;
}

}

////////////////////////////////////////////////////////////////////////////////
