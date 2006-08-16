/*! \file draw.cc
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include <ctime>

#include "TText.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGaxis.h"
#include "TStyle.h"
#include "TH1.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

void draw_text(char *prefix,float p,float x,float y)
{
    char buf[555];
    sprintf(buf,"%s%.1f%%",prefix,100*p);

    TText *text=new TText(x,y,buf);
    text->SetTextSize(0.22);
    text->Draw();
}

////////////////////////////////////////////////////////////////////////////////

void fill_pad(TVirtualPad *pad, const char *channel, float p1, float p2, int trips)
{
    pad->cd();
    
    TText *txt=new TText(0.05,0.7,channel);
    txt->SetTextSize(0.22);
    txt->Draw();
    
    draw_text("V ",p1,0.05,0.4);
    draw_text("- ",p2,0.55,0.4);
    draw_text("x ",1-p1-p2,0.05,0.1);
    
    char buff[555];
    sprintf(buff," # %d",trips);
    
    TText *text=new TText(0.55,0.1,buff);
    text->SetTextSize(0.22);
    text->Draw();

    if(p1==1 && trips==0)
        pad->SetFillColor(3);    //Fill the box green
    else if(p2==1 && trips==0)
        pad->SetFillColor(7);    //Fill the box blue
    else if(p1>=0.97 && (p1+p2)>=0.99)
        pad->SetFillColor(5);   //Fill the box yellow
    else if(p1<=0.97 && (p1+p2)>=0.99)
        pad->SetFillColor(41);   //Fill the box (dirty) yellow
    else
        pad->SetFillColor(2);    //Fill the box red

    pad->Draw();
}

////////////////////////////////////////////////////////////////////////////////

void draw_graph(TGraph *g,float axis_pos,float ymax,
                int color,const char *title,const char *draw_opt)
{
    float pos=gPad->GetUxmin()+axis_pos*(gPad->GetUxmax()-gPad->GetUxmin());
    TGaxis *axis = new TGaxis(pos,gPad->GetUymin(),pos,gPad->GetUymax(), 0, ymax,510,"+L");
    axis->SetLineColor(color);
    axis->SetLabelColor(color);
    axis->SetTextColor(color);
    axis->SetTitle(title);
    axis->SetLabelOffset(0);
    axis->SetTitleOffset(0);
    axis->Draw();

    float scale = gPad->GetUymax()/ymax;
    TGraph *newgraph = new TGraph;
    for( int i=0; i<g->GetN(); i++ )
        newgraph->SetPoint(i,g->GetX()[i],g->GetY()[i]*scale);
    g->TAttLine::Copy(*newgraph);
    g->TAttFill::Copy(*newgraph);
    g->TAttMarker::Copy(*newgraph);
    newgraph->SetLineColor(color);
    newgraph->SetBit(kCanDelete);
    //newgraph->AppendPad(option);
    //gPad->RangeAxis(t1,0,t2,2000);
    //g_HV->Draw("P");
    newgraph->Draw("PL");
}

////////////////////////////////////////////////////////////////////////////////

TH1F *draw_graph(TGraph *g,time_t t1,time_t t2,float y_min,float y_max)
{
    char name[222]="h";
    strcpy(name+1,g->GetName());
    
    TH1F *h=new TH1F(name,name,1000,t1,t2);
    h->GetYaxis()->SetTitle(g->GetTitle());
    h->GetXaxis()->SetTitle("Time");
    h->GetXaxis()->SetTimeDisplay(1);
    h->GetXaxis()->SetTimeFormat("#splitline{%d/%m}{%H:%M}");
    h->GetXaxis()->SetLabelSize(0.03);
    h->GetXaxis()->SetLabelOffset(0.02);
    h->SetMinimum(y_min);
    h->SetMaximum(y_max);

    h->Draw();
    g->SetTitle("");
    g->Draw("PL");
    g->SetTitle(name+1);
    
    return h;
}

////////////////////////////////////////////////////////////////////////////////
