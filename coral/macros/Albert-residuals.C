#include "TStyle.h"
#include "TPad.h"
#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TH3.h"
#include "TH2.h"
#include "TText.h"
#include "TLatex.h"
#include "string.h"
#include <iostream>

bool residual_xray_print=true;

vector <string> naDet;

const int NPlanes=24;
TH1F *h_rms_diff          = new TH1F("rms_diff","rms",        NPlanes,0,NPlanes);
TH1F *h_rms_residuals     = new TH1F("rms_residuals","rms",   NPlanes,0,NPlanes);
TH1F *h_rms_xray          = new TH1F("rms_xray","rms",        NPlanes,0,NPlanes);
TH1F *h_rms_all_xray      = new TH1F("rms_all_xray","RMS of Xray, all chambers",100,0,300);
TH1F *h_rms_all_residuals = new TH1F("rms_all_residuals","RMS of residuals, all chambers",100,0,300);
TH1F *h_rms_all_diff      = new TH1F("rms_all","RMS of Xray-residuals difference, all chambers",100,0,300);
TH1F *h_rms_diff_xr       = new TH1F("rms_diff_xr","RMS diff for Xray-residuals difference {black-red}",200,-300,300);
TH1F *h_rms_diff_xd       = new TH1F("rms_diff_xd","RMS diff for Xray-[Xray-residuals] difference {black-green}",200,-300,300);
TH1F *h_rms_diff_rd       = new TH1F("rms_diff_rd","RMS diff for residuals-[Xray-residuals] difference {red-green}",200,-300,300);
int plane=1;

bool det_accept_6mm(char *det)
{
    if( det[7]!='b' )
        return false;

    if( 0==strcmp(det,"ST06V1db") )
        return false;

    if( 0==strcmp(det,"ST06V1ub") )
        return false;

    if( 0==strcmp(det,"ST06Y1db") )
        return false;

    if( 0==strcmp(det,"ST06Y1ub") )
        return false;

    if( 0==strcmp(det,"ST05X1db") )
        return false;

    return true;
}

bool det_accept_56(char *det)
{
    if( 0==strcmp(det,"ST06V1db") )
        return false;

    if( 0==strcmp(det,"ST06V1ub") )
        return false;

    if( 0==strcmp(det,"ST05X1db") )
        return false;

    //if( det[7]=='b' )
    //    return false;

    if( det[3]!='5' && det[3]!='6' )
        return false;

    if( det[4]=='Y' )
        return false;

    return true;
}

bool det_accept(char *det)
{
    return det_accept_6mm(det);
}


void plot_residuals_diff(TH1F *h1, TH1F *h2)
{
    gStyle->SetOptStat(1);
    gStyle->SetOptTitle(1);
    gStyle->SetStatW(0.4);
    gStyle->SetStatH(0.25);

    if( fabs(h1->GetXaxis()->GetXmin() - h2->GetXaxis()->GetXmin())>0.1   ||
        fabs(h1->GetXaxis()->GetXmax() - h2->GetXaxis()->GetXmax())>0.1   ||
        h1->GetXaxis()->GetNbins()!=h2->GetXaxis()->GetNbins() )
    {
        printf("Big difference!\n");
        return;
    }

    float center = (h1->GetXaxis()->GetXmin()+h1->GetXaxis()->GetXmax())/2;
    char name[44];
    sprintf(name,"%s_diff",h1->GetName());
    TH1F *hd=new TH1F(name,"Difference between Xray and residuals",
                      h1->GetXaxis()->GetNbins(),
                      h1->GetXaxis()->GetXmin(), h1->GetXaxis()->GetXmax());
    hd->GetXaxis()->SetTitle("Coordinate, [mm]");
    hd->GetYaxis()->SetTitle("Residual, [#mum]");

    #ifdef report_C
    RS.registration(hd);
    #endif

    for( int b=1; b<=h1->GetXaxis()->GetNbins(); b++ )
        hd->SetBinContent(b,h1->GetBinContent(b)-h2->GetBinContent(b));
    TCanvas *c=new TCanvas(name,name,800,600);
    c->Divide(2,1);
    c->cd(1);

    h1->SetLineColor(kBlack);
    h2->SetLineColor(kRed);
    hd->SetLineColor(kGreen);

    //char *fit_options="W LL F";
    char *fit_options="W F";

    //=====================

    h1->Fit("pol1",fit_options);
    h1->GetFunction("pol1")->SetLineColor(h1->GetLineColor());
    TVirtualFitter *fitter = TVirtualFitter::GetFitter();
    float xray_p0=fitter->GetParameter(0), xray_p1=fitter->GetParameter(1);

    //=====================

    hd->SetStats(0);
    hd->Fit("pol1",fit_options);
    hd->GetFunction("pol1")->SetLineColor(hd->GetLineColor());
    TVirtualFitter *fitter = TVirtualFitter::GetFitter();
    float cor_p0=fitter->GetParameter(0), cor_p1=fitter->GetParameter(1);

    //=====================

    h2->GetXaxis()->SetTitle("wire number, mm");
    h2->GetYaxis()->SetTitle("Residual, #mu m");
    h2->SetStats(0);
    h2->SetMinimum(-500);
    h2->SetMaximum( 500);
    h2->Fit("pol1",fit_options);
    h2->GetFunction("pol1")->SetLineColor(h2->GetLineColor());
    TVirtualFitter *fitter = TVirtualFitter::GetFitter();
    float orig_p0=fitter->GetParameter(0), orig_p1=fitter->GetParameter(1);
    printf("alignment of %s: %g %g\n",h1->GetName(),orig_p0,orig_p1);

    //=====================

    h1->SetStats(0);
    h1->Draw("Psame");

    hd->Draw("same");

    if(1)
    {
        char det[99];
        sprintf(det,"%s",h1->GetName()+6);
        det[8]=0;

        //c=new TCanvas(name,name,800,600);
        TPad *pad=c->cd(2);
        pad->Divide(1,3);
        
        // ---------        

        pad->cd(1);
        sprintf(name,"%s_proj_orig",h1->GetName());
        TH1F *hp_xray = new TH1F(name,"Xray spread",500,-1000,1000);
        hp_xray->GetXaxis()->SetTitle("RMS, #mu m");
        hp_xray->GetYaxis()->SetTitle("Events");

        for( int b=1; b<=h1->GetXaxis()->GetNbins(); b++ )
        {
            float x = h1->GetBinCenter(b);
            float r = h1->GetBinContent(b) - (xray_p0+xray_p1*x);
            hp_xray->Fill(r);
        }
        hp_xray->SetLineColor(h1->GetLineColor());
        hp_xray->Draw();

        if( det_accept(det) )
        {
            h_rms_xray->GetXaxis()->SetBinLabel(plane,det);
            h_rms_xray->SetBinContent(plane,hp_xray->GetRMS(1));
            h_rms_all_xray->Fill(hp_xray->GetRMS(1));
        }

        // ---------        
        
        pad->cd(2);
        sprintf(name,"%s_proj",h2->GetName());
        TH1F *hp_0 = new TH1F(name,"Residuals, no X-ray",500,-1000,1000);
        hp_0->GetXaxis()->SetTitle("RMS, #mu m");
        hp_0->GetYaxis()->SetTitle("Events");

        for( int b=1; b<=hd->GetXaxis()->GetNbins(); b++ )
        {
            float x = h2->GetBinCenter(b);
            float r = h2->GetBinContent(b) - (orig_p0+orig_p1*x);
            hp_0->Fill(r);
        }
        hp_0->SetLineColor(h2->GetLineColor());
        hp_0->Draw();

        if( det_accept(det) )
        {
            h_rms_residuals->GetXaxis()->SetBinLabel(plane,det);
            h_rms_residuals->SetBinContent(plane,hp_0->GetRMS(1));
            h_rms_all_residuals->Fill(hp_0->GetRMS(1));
        }
        
        // ---------        
        
        pad->cd(3);
        sprintf(name,"%s_proj",h1->GetName());
        TH1F *hp_cor = new TH1F(name,"Expected residuals",500,-1000,1000);
        hp_cor->GetXaxis()->SetTitle("RMS, #mu m");
        hp_cor->GetYaxis()->SetTitle("Events");

        for( int b=1; b<=hd->GetXaxis()->GetNbins(); b++ )
        {
            float x = hd->GetBinCenter(b);
            float r = hd->GetBinContent(b) - (cor_p0+cor_p1*x);
            hp_cor->Fill(r);
        }
        hp_cor->SetLineColor(hd->GetLineColor());
        hp_cor->Draw();
        
        if( det_accept(det) )
        {
            h_rms_diff->GetXaxis()->SetBinLabel(plane,det);
            h_rms_diff->SetBinContent(plane,hp_cor->GetRMS(1));
            h_rms_all_diff->Fill(hp_cor->GetRMS(1));
        }
        
        if( det_accept(det) )
        {
            h_rms_diff_xd->Fill(hp_xray->GetRMS(1)-hp_cor->GetRMS(1));
            h_rms_diff_xr->Fill(hp_xray->GetRMS(1)-hp_0->GetRMS(1));
            h_rms_diff_rd->Fill(hp_0->GetRMS(1)-hp_cor->GetRMS(1));
            plane++;
        }

        #ifdef report_C
        RS.registration(hp_xray);
        RS.registration(hp_0);
        RS.registration(hp_cor);
        #endif
    }

    c->cd(0);
    if( residual_xray_print )
    {
        c->Print("","gif");
        c->Print("","ps");
    }
    delete c;
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
}

void UMaps_forSasha( TString name = "" ) {

 if( name == "" ) name="trafdic.root";      // rootfile with 3D residuals histos

 gStyle->SetTitleH(0.1);
 gStyle->SetTitleW(0.7);
 gStyle->SetPalette(1);
 gStyle->SetOptStat(0);
 gStyle->SetOptTitle(0);

 TH2 *h0[7];
 TProfile *prof[7];
 char *pos[7] = { "m3", "m2", "m1", "p1", "p2", "p3", "0" };

 TFile *f=(TFile*)gROOT->GetListOfFiles()->FindObject(name);
 if( f==0) f = TFile::Open(name);
 f->cd("dPrivate");

 /* Determine the set of detectors for which there are histograms in
    the file.  */
 naDet.clear();
 TIter next(gDirectory->GetListOfKeys());
 TKey *key = 0;
 while ((key = (TKey*)next())) {
   // The interesting keys are those whose name starts with "Udist_",
   // followed by the detector name, since we are interested in
   // straws, we only care about detectors whose names begin in "ST".
   if (strncmp(key->GetName(), "Udist_ST", strlen("Udist_ST"))) {
     // Only straws.
     continue;
   }
   string s(key->GetName());
   naDet.push_back(s.substr(strlen("Udist_"))); // Store detector name.
 }
 int Ndet = naDet.size();

 TCanvas *c1;
 c1 = new TCanvas("c1","c1",10,10,700,1000);
 c1->SetFillColor(10);

 TFile *out = new TFile("Umap-STxray.root","RECREATE");         // output file with Umaps, to be read from CompStrawXray3D.C

 TPostScript *ps = new TPostScript("Umap-31971-STxray.ps",111);

 /////////////////////////////////////

 int pad = 0;
 gPad->Update();
 for ( int i=0; i < Ndet; i++) {

   if ((pad % 6) == 0) {
     ps->NewPage();
     c1->Clear(); 
     c1->Divide(2,3);
     pad = 0;
   }

   f->cd("dPrivate");
   const float EntryLimit = 10;
   TString dd = naDet[i];
   cout << dd << endl;
   //sprintf(hist, "Udist_%s", naDet[i].c_str());
   TH3F *a0 = (TH3F*)gDirectory->Get(("Udist_" + naDet[i]).c_str());
   if( a0==NULL )
   {
    printf("Can not find histogram %s\n",hist);
    continue;
   }
   if (a0->GetEntries() < EntryLimit) continue;

   for ( int kk=0; kk <= 6; kk++) {
   
     // The histogram 'a0' may have 6 (original code) or 12 (Sasha's modification)
     // divisions along straw. Take into account this automatically.
     if( a0->GetYaxis()->GetNbins()==12 )
       if ( kk != 6 )
         a0->GetYaxis()->SetRange(kk*2+1,kk*2+2);
       else
         a0->GetYaxis()->SetRange(6,7);
     else
       if ( kk != 6 )
         a0->GetYaxis()->SetRange(kk+1,kk+1);
       else
         a0->GetYaxis()->SetRange(1,6);

     h0[kk] = (TH2*)a0->Project3D("zx");
     h0[kk]->GetXaxis()->SetTitle("u in mm");
     h0[kk]->GetYaxis()->SetTitle("v in mm");
     h0[kk]->GetZaxis()->SetTitle("du in mm");
     // Cint doesn't like it if I do the "+" thing as in the line
     // below, therefore this ugly piece of crap.
     h0[kk]->SetTitle((string(naDet[i]).append(" Residual u vs v at ")
		       .append(pos[kk])).c_str());
     h0[kk]->SetName(("uv_Residual_" + naDet[i] + pos[kk]).c_str());

     prof[kk] = h0[kk]->ProfileX();
     prof[kk]->GetXaxis()->SetTitle("u in mm");
     prof[kk]->GetYaxis()->SetTitle("du in #mum");
     prof[kk]->GetYaxis()->SetTitleOffset(1.5);
     prof[kk]->SetName(("du_Map_" + naDet[i] + pos[kk]).c_str());
     prof[kk]->SetTitle(("du of " + naDet[i] + pos[kk]).c_str());
     prof[kk]->Scale(1000);
     prof[kk]->SetMinimum(-500);
     prof[kk]->SetMaximum( 500);

     if ( kk != 6 ) {     // plot only the subranges !
       pad++;
       c1->cd(pad);
       gPad->SetLeftMargin(0.125);

       prof[kk]->Draw();

       TLatex *tex3 = new TLatex(0.5, 0.93,
				 ("du Map of " + naDet[i] + pos[kk]).c_str());
       tex3->SetNDC(kTRUE);
       tex3->SetTextAlign(22);
       tex3->SetTextSize(0.07);
       tex3->SetLineWidth(2);
       tex3->SetTextColor(kBlue);
       tex3->Draw();
       c1->Update();
     }

     out->cd();
     prof[kk]->Write();
   }

 }

 ps->Close();
 out->Close();
}
#include "TStyle.h"
#include "TPad.h"
#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "TH3.h"
#include "TH2.h"
#include "TText.h"
#include "TLatex.h"
#include "TString.h"

void CompStrawXray3D_forSasha() {

  UMaps_forSasha("richard_withXray.root");
  nDet = naDet.size();

  char hist[100];

  char *pos = { "0" };        // xray along spacer

  TH1F *h0 = new TH1F[nDet];
  TH1F *h1 = new TH1F[nDet];
  TH1F *h0new = new TH1F[nDet];
  TH1F *h1new = new TH1F[nDet];
  TProfile *hp1 = new TH1F[nDet];

  gStyle->SetTitleH(0.1);
  gStyle->SetTitleW(0.7);
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);

  TString name0 = "xray.root";         // Sasha's x-ray maps
  TFile *f0=(TFile*)gROOT->GetListOfFiles()->FindObject(name0);
  if( f0==0) f0 = new TFile(name0);
  f0->cd();

  TString name1 = "Umap-STxray.root";              // produced with UMaps_forSasha.C
  TFile *f1=(TFile*)gROOT->GetListOfFiles()->FindObject(name1);
  if( f1==0) f1 = new TFile(name1);
  f1->cd();

  TCanvas *c1;
  c1 = new TCanvas("c1","c1",10,10,1200,750);
  c1->SetFillColor(10);
  c1->SetBorderMode(0);

  sprintf(hist, "StrawXray-2003-Xray-%s.ps", pos);
  TPostScript *ps = new TPostScript(hist,112);

  char *detector = "ST06X1";
  int pad = -1;
  for ( int l=0; l < nDet; l++) {

    TString dd = naDet[l];
//     if ( ! dd.Contains("ST06X1") ) continue;

    int t=f1->cd();
    //printf("cd=%d\n");
    sprintf(hist, "du_Map_%s_%s", naDet[l], pos);
    hp1[l] = (TProfile*)gDirectory->Get(hist);           // Histo from Data 2003 (all fitted)
    if( hp1[l]==NULL )
    {
        printf("The histograms was not found: %s\n",hist);
        continue;
    }

    int   xBins = hp1[l]->GetXaxis()->GetNbins();
    int   xMin  = hp1[l]->GetXaxis()->GetXmin();
    int   xMax  = hp1[l]->GetXaxis()->GetXmax();

    sprintf(hist, "Udist_%s_xray", naDet[l]);
    h0new[l] = new TH1F(hist,hist,xBins,xMin,xMax);
    sprintf(hist, "Udist_%s_tracking_2003", naDet[l]);
    h1new[l] = new TH1F(hist,hist,xBins,xMin,xMax);

    f0->cd();
    sprintf(hist, "%s_xray_%s", naDet[l], pos);
    h0[l] = (TH1F*)gDirectory->Get(hist);             // Histo from x-ray

    #ifdef report_C
    RS.registration(hp1[l]);
    RS.registration(h0[l]);
    RS.registration(h0new[l]);
    RS.registration(h1new[l]);
    #endif


    int   xxBins = h0[l]->GetXaxis()->GetNbins();
    cout << "Detector = " << naDet[l] << "    hist = " << hist << endl;

    for ( int ii=1; ii <= xBins; ii++) {
      if ( xBins != xxBins ) {
        cout << hist << ":   Number of xBins different !!" << endl;
        continue;
      }
      h0new[l]->SetBinContent(ii, -h0[l]->GetBinContent(ii));
      h1new[l]->SetBinContent(ii, hp1[l]->GetBinContent(ii));
    }

    cout << "Detector = " << naDet[l] << endl;

    if (pad == -1 || pad == 6) {
//printf("New page!\n");
      ps->NewPage();
      c1->Clear(); 
      c1->Divide(3,2);
      pad = 0;
    }

    pad++;
    c1->cd(pad);
//printf("1\n");
    float h0max = h0new[l]->GetMaximum();
    float h1max = h1new[l]->GetMaximum();
    if ( h1max > h0max ) h0max = h1max;
    float h0min = h0new[l]->GetMinimum();
    float h1min = h1new[l]->GetMinimum();
    if ( h1min < h0min ) h0min = h1min;
//printf("2\n");

    h0new[l]->SetMaximum(500);
    h0new[l]->SetMinimum(-500);
//printf("3\n");
    h0new[l]->GetXaxis()->SetTitleSize(0.05);
    h0new[l]->GetXaxis()->SetTitle("u in mm");
    h0new[l]->GetYaxis()->SetTitleOffset(1.2);
    h0new[l]->GetYaxis()->SetTitleSize(0.05);
    h0new[l]->GetYaxis()->SetTitle("du in #mum");
    h0new[l]->SetLineColor(1);
    h0new[l]->SetLineWidth(2);
    h0new[l]->Draw("P");
    //    h0new[l]->Draw("hist");

    h1new[l]->SetLineColor(2);
    h1new[l]->SetLineWidth(1);
    h1new[l]->Draw("histsame");

    h0new[l]->SetMarkerSize(0.4);
    h0new[l]->SetMarkerColor(1);
    h0new[l]->SetMarkerStyle(20);
    h0new[l]->Draw("Psame");

    char txt[100];
    sprintf(txt,"%s", naDet[l]);
    TLatex *tex3 = new TLatex(0.5, 0.93, txt);
    tex3->SetNDC(kTRUE);
    tex3->SetTextAlign(22);
    tex3->SetTextSize(0.065);
    tex3->SetLineWidth(2);
    tex3->SetTextColor(kBlue);
    tex3->Draw();

//printf("4\n");
    if (pad == 1) {
      char txt1[100];
      sprintf(txt1, "x-ray correction");
//printf("4a\n");
      TLatex *tex4 = new TLatex(0.15, 0.85, txt1);
//printf("4b\n");
      tex4->SetNDC(kTRUE);
      tex4->SetTextAlign(12);
      tex4->SetTextSize(0.05);
      tex4->SetTextColor(1);
      tex4->Draw();
      sprintf(txt1, "Run #37059 x-ray corr.");
//printf("4c\n");
      TLatex *tex5 = new TLatex(0.15, 0.775, txt1);
      tex5->SetNDC(kTRUE);
      tex5->SetTextAlign(12);
      tex5->SetTextSize(0.05);
      tex5->SetTextColor(2);
      tex5->Draw();
//printf("4d\n");
    }
//printf("4e\n");
    c1->Update();
//printf("4f\n");

    ps->Off();
    plot_residuals_diff(h0new[l],h1new[l]);
    ps->On();
  }
//printf("5\n");

//   sprintf(hist, "StrawXray_%s_%s.eps", detector, pos);
//   c1->Print(hist);
//   sprintf(hist, "StrawXray_%s_%s.gif", detector, pos);
//   c1->Print(hist);
//   sprintf(hist, "StrawXray_%s_%s.root", detector, pos);
//   c1->Print(hist);

  ps->Close();

  if(1)
  {
      h_rms_diff->GetXaxis()->SetTitle("wire coordinate");
      h_rms_diff->GetYaxis()->SetTitle("residual, #mu m");

      h_rms_diff_rd->GetXaxis()->SetTitle("#mu m");
      h_rms_diff_rd->GetYaxis()->SetTitle("events");
      
      h_rms_diff_xr->GetXaxis()->SetTitle("RMS, #mu m");
      h_rms_diff_xd->GetXaxis()->SetTitle("RMS, #mu m");
      
      h_rms_all_xray->GetXaxis()->SetTitle("RMS, #mu m");
      h_rms_all_residuals->GetXaxis()->SetTitle("RMS, #mu m");
      h_rms_all_diff->GetXaxis()->SetTitle("RMS, #mu m");
      
      h_rms_xray->GetXaxis()->SetTitle("RMS, #mu m");


      TCanvas *c = new TCanvas("rms");
      c->Divide(2,3);
      
      c->cd(1);
      
      h_rms_diff->SetStats(0);
      h_rms_diff->SetMinimum(0);
      h_rms_diff->SetMaximum(300);
      h_rms_diff->SetLineColor(kGreen);
      h_rms_diff->SetMarkerColor(kGreen);
      h_rms_diff->SetMarkerSize(1);
      h_rms_diff->SetMarkerStyle(20);
      h_rms_diff->GetYaxis()->SetTitle("#mu m");
      h_rms_diff->Draw("P");
      

      h_rms_xray->SetStats(0);
      h_rms_xray->SetLineColor(kBlack);
      h_rms_xray->SetMarkerColor(kBlack);
      h_rms_xray->SetMarkerSize(1);
      h_rms_xray->SetMarkerStyle(21);
      h_rms_xray->Draw("PSAME");

      h_rms_residuals->SetStats(0);
      h_rms_residuals->SetLineColor(kRed);
      h_rms_residuals->SetMarkerColor(kRed);
      h_rms_residuals->SetMarkerSize(1);
      h_rms_residuals->SetMarkerStyle(22);
      h_rms_residuals->Draw("PSAME");
      
      c->Update();

      gStyle->SetOptStat(1);
      gStyle->SetOptTitle(1);
            
      c->cd(3);
      h_rms_all_xray->SetStats(0);
      h_rms_all_xray->SetLineColor(kBlack);
      h_rms_all_xray->Draw();
      h_rms_all_residuals->SetStats(0);
      h_rms_all_residuals->SetLineColor(kRed);
      h_rms_all_residuals->Draw("SAME");
      h_rms_all_diff->SetStats(0);
      h_rms_all_diff->SetLineColor(kGreen);
      h_rms_all_diff->Draw("SAME");
      
      c->cd(2);
      h_rms_diff_xd->Draw();

      c->cd(4);
      h_rms_diff_xr->Draw();

      c->cd(6);
      h_rms_diff_rd->Draw();
      
      TPad *p=c->cd(5);
      p->Divide(3,1);
      p->cd(1);
      h_rms_all_xray->SetStats(1);
      h_rms_all_xray->Draw();
      p->cd(2);
      h_rms_all_residuals->SetStats(1);
      h_rms_all_residuals->Draw();
      p->cd(3);
      h_rms_all_diff->SetStats(1);
      h_rms_all_diff->Draw();
      
      c->cd(0);
      c->Print("","ps");
      
      
      // --------
      
      TCanvas *cc=new TCanvas("q1");
      h_rms_diff_rd->Draw();
      cc->Print("gain.gif");
      
      cc=new TCanvas("q2");
      h_rms_diff->Draw("HIST");
      cc->Print("example.gif");

      cc=new TCanvas("q3");
      gStyle->SetOptTitle(kFALSE);
      h_rms_diff->SetStats(0);
      h_rms_diff->SetMinimum(0);
      h_rms_diff->SetMaximum(300);
      h_rms_diff->SetLineColor(kGreen);
      h_rms_diff->SetMarkerColor(kGreen);
      h_rms_diff->SetMarkerSize(1);
      h_rms_diff->SetMarkerStyle(20);
      h_rms_diff->Draw("P");

      h_rms_xray->SetStats(0);
      h_rms_xray->SetLineColor(kBlack);
      h_rms_xray->SetMarkerColor(kBlack);
      h_rms_xray->SetMarkerSize(1);
      h_rms_xray->SetMarkerStyle(21);
      h_rms_xray->Draw("PSAME");

      h_rms_residuals->SetStats(0);
      h_rms_residuals->SetLineColor(kRed);
      h_rms_residuals->SetMarkerColor(kRed);
      h_rms_residuals->SetMarkerSize(1);
      h_rms_residuals->SetMarkerStyle(22);
      h_rms_residuals->Draw("PSAME");
      
      cc->Print("rms_all.gif");
      gStyle->SetOptTitle(kTRUE);
      
      cc=new TCanvas("q4");
      cc->Divide(3,1);
      cc->cd(1);
      h_rms_all_xray->SetStats(1);
      h_rms_all_xray->Draw();
      cc->cd(2);
      h_rms_all_residuals->SetStats(1);
      h_rms_all_residuals->Draw();
      cc->cd(3);
      h_rms_all_diff->SetStats(1);
      h_rms_all_diff->Draw();
      cc->Print("all-rms.gif");
  }

}


Albert_residuals(char *detector="",char *options="")
{
    CompStrawXray3D_forSasha();
    #ifdef report_C
    RS.Close();
    #endif
}

#ifdef report_C

/*! Initialization function.
*/
bool Albert_residuals_init(void)
{
    RS.reports[RS.reports_counter++]=Albert_residuals;
    printf("Registering 'Albert_residuals' report.\n");
}

/*! Initilize the report during its loading!
*/
bool init=Albert_residuals_init();

#endif
