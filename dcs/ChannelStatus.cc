/*! \file ChannelStatus.cc
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include <string>
#include <cassert>

#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TGaxis.h"
#include "TStyle.h"

#include "ChannelStatus.h"
#include "dcs.h"

using namespace std;

/*! \brief A single measurement (float) at time moment (time_t)
    \warning Time precision is seconds.
*/
struct Measurement
{
    Measurement(time_t tt=0,float vv=0) : t(tt), v(vv) {}
    time_t t;
    float v;
};

int ChannelStatus::bins=1000;
int ChannelStatus::bins_avail=10;

static const int
    color_HV            = kMagenta,
    color_avail         = kBlue,
    color_temperature   = kRed,
    color_current       = kGreen;

extern bool debug;

////////////////////////////////////////////////////////////////////////////////

ChannelStatus::ChannelStatus(const string &det_name,const string &chan_name,time_t _t1,time_t _t2,TSQLServer &_db)
:   detector(det_name),
    channel(chan_name),
    t1(_t1),
    t2(_t2),
    db(_db),
    voltage_ok(0),
    voltage_low(0),
    trips(0),
    g_Temperature(NULL),
    g_HV(NULL),
    h_avail(NULL),
    all_trips(NULL),
    all_not_available(NULL)
{
    nominal_HV = get_nominal_HV(db,det_name.c_str(),channel.c_str(),(t1+t2)/2);

    // book histograms
    char name[222], title[222];

    sprintf(name,"availability_%s_%s",detector.c_str(),channel.c_str());
    sprintf(title,"Status of %s:%s in %s",
            detector.c_str(),channel.c_str(),interval_s(t1,t2).c_str());
    h_avail = new TH1F(name,title,bins,t1,t2);
    h_avail->SetStats(false);
    sprintf(title,"Availability in last %g seconds",bins_avail*double(t2-t1)/bins);
    h_avail->GetYaxis()->SetTitle(title);
    h_avail->GetXaxis()->SetTitle("Time");
    h_avail->GetXaxis()->SetTimeDisplay(1);
    h_avail->GetXaxis()->SetTimeFormat("#splitline{%d/%m}{%H:%M}");
    h_avail->GetXaxis()->SetLabelSize(0.03);
    h_avail->SetLabelOffset(0.02);
    h_avail->SetMinimum(0);
    h_avail->SetMaximum(105);

    sprintf(name,"HV_%s_%s",detector.c_str(),channel.c_str());
    g_HV=new TGraph;
    g_HV->SetName(name);
    g_HV->SetTitle("High Voltage, V");
    g_HV->SetMarkerStyle(24);
    g_HV->SetMarkerSize(0.4);
    g_HV->SetMarkerColor(color_HV);

    g_HV->GetXaxis()->SetTimeDisplay(1);
    g_HV->GetXaxis()->SetLabelSize(0.03);
    g_HV->GetXaxis()->SetTimeFormat("#splitline{%d/%m}{%H:%M}");
    g_HV->GetYaxis()->SetTitle("High voltage, V");    
    
    SetDrawCurrent(true);
    SetDrawTemperature(true);
}

////////////////////////////////////////////////////////////////////////////////

void ChannelStatus::SetDrawCurrent(bool f)
{
    if( !f )
    {
        delete g_current;
        g_current=NULL;
    }
    else
    {
        if( g_current==NULL )
        {
            char name[111];
            sprintf(name,"I_%s_%s",detector.c_str(),channel.c_str());
            g_current=new TGraph;
            g_current->SetName(name);
            g_current->SetTitle("Current, #mu A");
            g_current->SetMarkerStyle(24);
            g_current->SetMarkerSize(0.4);
            g_current->SetMarkerColor(color_current);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ChannelStatus::SetDrawTemperature(bool f)
{
    if( !f )
    {
        delete g_Temperature;
        g_Temperature=NULL;
    }
    else
    {
        if( g_Temperature==NULL )
        {
            char name[111];
            sprintf(name,"T_%s_%s",detector.c_str(),channel.c_str());
            g_Temperature=new TGraph;
            g_Temperature->SetName(name);
            g_Temperature->SetTitle("Temperature, ^{0}C");
            g_Temperature->SetMarkerStyle(23);
            g_Temperature->SetMarkerSize(0.4);
            g_Temperature->SetMarkerColor(color_temperature);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ChannelStatus::CalculateStatus(void)
{
    if( debug )
        printf("%s %s\n",detector.c_str(),channel.c_str());

    // We can not do anything without a knowledge about nominal high voltage
    if( nominal_HV==0 )
    {
        printf("Can not get nominal HV for %s %s\n",detector.c_str(),channel.c_str());
        return;
    }

    // First we read the measurements.    
    vector<TSQLRow*> measurements;
    TSQLResult *table = get_measurements(db,detector.c_str(),channel.c_str(),t1,t2,"dcs.HV",measurements);

    if( measurements.empty() )
    {
        printf("No measurements at all!\n");
        return;
    }

    if( debug )
        printf("%d %s has %d measurements.\n",detector.c_str(),channel.c_str(),measurements.size());
    
    // Print debug information about the measurements we retrieved from DB
    if( debug )
    {
        // Get the very first and very last measurements
        time_t db_t1,db_t2,db_last;
        float voltage;
        decode_HV_measurement(measurements.front(),db_t1,voltage);
        decode_HV_measurement(measurements.back(),db_t2,voltage);

        printf("detector %s   channel %s\n",detector.c_str(),channel.c_str());
        printf("Request: %s time interval.\n",interval_s(t1,t2).c_str());
        printf("DB-delivered: %s time interval with %d measurements.\n",
                interval_s(db_t1,db_t2).c_str(), measurements.size());
        for( int i=0; i<measurements.size(); i++ )
            printf("%s  %s\n",measurements[i]->GetField(0),measurements[i]->GetField(6));
    }

    // convert all measurements to Measurement
    vector<Measurement> thv;
    for( vector<TSQLRow*>::const_iterator it=measurements.begin();
         it!=measurements.end(); it++ )
    {
        thv.push_back(Measurement());
        decode_HV_measurement(*it,thv.back().t,thv.back().v);
    }
    
    // Change the time of very first measurement to t1
    if( thv.front().t<t1 )
        thv.front().t=t1;

    // Add the time of very last measurement to t2
    assert(thv.back().t<t2);
    thv.push_back(thv.back());
    thv.back().t=t2;
    
    trips=0;
    
    double
        NHV=0,      // The sum of all time intervals with nominal HV
        LHV=0,      // The sum of all time intervals with lowered HV
        NHV_tot=0,
        LHV_tot=0;
    
    int bc_bin_last=0;
    int filled_tripped_last_bin=-1;

    for( int i=0; i<thv.size(); i++ )
    {
        const Measurement &old=thv[i-1];

        time_t now=thv[i].t;
        float voltage=thv[i].v;
        //decode_HV_measurement(measurements[i],now,voltage);

        // Add an extra graph point if a previous measurement was >10 seconds ago
        if( i>0 && (now-old.t)>=10 )
            g_HV->SetPoint(g_HV->GetN(),now-1,old.v);
        
        g_HV->SetPoint(g_HV->GetN(),now,voltage);
    
        if( i==0 )
            continue;

        assert(now<=t2);

        if( debug )
            printf("Next measuremnt!\n");

        bool
            is_nominal = HoldNominalHV(old.v),
            is_lowered = !is_nominal && IsOn(old.v);

        // This is a channel trip detection.
        if( IsTrip(old.v,voltage) )
        {
            trips++; // In the variable we count the very trip.

            // Do we need to fill a shared (among other channels) histogram?
            if( all_trips!=NULL )
            {
                // Get the histogram bin number which we are going to fill.
                int bin=all_trips->GetXaxis()->FindFixBin(now);
                
                if( debug )
                    printf("channel trip is detected for bin %d.\n",bin);

                // We don't want to fill several times the same time bin.
                if( bin!=filled_tripped_last_bin )
                {
                    // OK, this bin was not filled yet.
                    if( filled_tripped_last_bin>=bin )
                        throw "ChannelStatus::CalculateStatus(): not time-ordered values!";
                    filled_tripped_last_bin=bin;
                    all_trips->Fill(now);
                }
            }
        }

        if( (now-old.t)<0 )
        {
            printf("bad times: old=%s(%d) now=%s(%d)",
                    time_to_timestamp(old.t).c_str(),old.t,
                    time_to_timestamp(now).c_str(),now);
            throw "ChannelStatus::CalculateStatus(): Bad time ordering.";
        }

        int
            bin_old = h_avail->GetXaxis()->FindFixBin(old.t),
            bin_new = h_avail->GetXaxis()->FindFixBin(now);

        if( debug )
            printf("bin_old=%d bin_new=%d\n",bin_old,bin_new);

        if( bin_old!=bin_new )
        {
            // finish last bin
            if( debug )
                printf("Finishing bin %d\n",bin_old);

            // Get the start point
            double ts=h_avail->GetXaxis()->GetBinLowEdge(bin_old);
            if( old.t>ts )
                ts=old.t;

            // The end point is GetBinUpEdge(bin_old)

            double dt = h_avail->GetXaxis()->GetBinUpEdge(bin_old)-ts;

            if( is_nominal )
                NHV += dt;
            if( is_lowered )
                LHV += dt;

            if( debug )
                printf("bw=%g   dt=%g    old.t=%d   ts=%g\n",
                        h_avail->GetXaxis()->GetBinWidth(bin_old),dt,old.t,ts);

            h_avail->SetBinError( bin_old, NHV/h_avail->GetXaxis()->GetBinWidth(bin_old) );
            if( debug )
                printf("bin %d: NHV/LHV:  %g/%g\n",
                        bin_old,
                        NHV/h_avail->GetXaxis()->GetBinWidth(bin_old),
                        LHV/h_avail->GetXaxis()->GetBinWidth(bin_old));

            NHV_tot += NHV;
            LHV_tot += LHV;

            NHV=LHV=0;
        }

        // Because the HV state did not change for these bins, we set
        // availability to 0 or 1 depending on the last HV measurement
        if( debug )
            printf("Set a=%d for bins:",is_nominal);

        for( int b=bin_old+1; b<bin_new; b++ )
        {
            h_avail->SetBinError( b, is_nominal );

            if( is_nominal )
                NHV_tot += h_avail->GetXaxis()->GetBinWidth(b);
            if( is_lowered )
                LHV_tot += h_avail->GetXaxis()->GetBinWidth(b);

            if( debug )
                printf("%d ",b);
        }
        if( debug )
            printf("\n");

        // Start/continue the bin
        if( debug )
            printf("Start/conitnue bin %d\n",bin_new);

        // Get the start point
        double ts=h_avail->GetXaxis()->GetBinLowEdge(bin_new);
        if( old.t>ts )
            ts=old.t;

        assert(now>=h_avail->GetXaxis()->GetBinLowEdge(bin_new));
        assert(now<=h_avail->GetXaxis()->GetBinUpEdge(bin_new));

        double dt = now-ts;

        if( is_nominal )
            NHV += dt;
        if( is_lowered )
            LHV += dt;
        
        // Now we fill histogram with not-perect channels
        if( all_not_available!=NULL )
        {
            int bin_current=all_not_available->GetXaxis()->FindFixBin(now);

            if( bin_current!=bc_bin_last )
            {
                // OK, this is a next time bin.
                if( debug )
                    printf("Old bin is %d. The new one is %d.\n",bc_bin_last,bin_current);

                // But first of all we have to fill the 'previous' bin.
                // And we may skip some bins of the histogram!
                // That is why we have the loop.
                if( !is_nominal )
                {
                    if( debug )
                        printf("%s:%s   old_v=%g new_v=%g\n",detector.c_str(),channel.c_str(),old.v,voltage);

                    for( int b=bc_bin_last; b<bin_current; b++ )
                    {
                        //if( debug && b!=bc_bin_last )
                        //    printf("Wow! We have no measurement for the bin %d\n",b);

                        all_not_available->SetBinContent(b,all_not_available->GetBinContent(b)+1);
                    }
                }

                bc_bin_last=bin_current; // This is our new bin!
                if( debug )
                    printf("all_not_available: last filled bin is %d\n",bc_bin_last);
            }
        }
    }

    if( all_not_available!=NULL )
    {
        assert( bc_bin_last!=all_not_available->GetXaxis()->GetNbins() );
    }
    
    voltage_ok  = NHV_tot/(t2-t1);
    voltage_low = LHV_tot/(t2-t1);
    
    if( debug )
        printf("trips=%d\n",trips);
    
    for( int i=1; i<=h_avail->GetNbinsX(); i++ )
    {
        float a=0;
        int j;
        for( j=i; j>=1 && j>i-bins_avail; j-- )
            a += h_avail->GetBinError(j);
        h_avail->SetBinContent(i,100*a/(i-j));
        //printf("SET: %d %g\n",i,h_avail->GetBinContent(i));
    }

    for( int i=1; i<=h_avail->GetNbinsX(); i++ )
        h_avail->SetBinError(i,1e-7);
}

////////////////////////////////////////////////////////////////////////////////

void ChannelStatus::ReadTemperature(void)
{
    if( NULL==g_Temperature )
        throw "ChannelStatus::ReadTemperature(): tempreture container does not exist.";

    // First we read the measurements.    
    vector<TSQLRow*> measurements;
    TSQLResult *table = get_measurements(db,"dcs.Temperature",t1,t2,
                                         "AND detector='ST05U1' AND sensor='TMJ'",
                                         measurements);

    if( measurements.empty() )
    {
        printf("No temperature measurements at all!\n");
        return;
    }

    // Print debug information about the measurements we retrieved from DB
    if( debug )
    {
        // Get the very first and very last measurements
        time_t db_t1,db_t2,db_last;
        float temperature;

        db_t1=timestamp_to_time((char*)measurements.front()->GetField(0));
        db_t2=timestamp_to_time((char*)measurements.back()->GetField(0));

        printf("detector %s   channel %s\n",detector.c_str(),channel.c_str());
        printf("Request: %s time interval.\n",interval_s(t1,t2).c_str());
        printf("DB-delivered: %s time interval with %d measurements.\n",
                interval_s(db_t1,db_t2).c_str(), measurements.size());
        for( int i=0; i<measurements.size(); i++ )
            printf("%s  %s\n",measurements[i]->GetField(0),measurements[i]->GetField(1));
    }

    // convert all measurements to Measurement (but now we have TEMPERATURE instead of VOLTAGE).
    vector<Measurement> thv;
    for( vector<TSQLRow*>::const_iterator it=measurements.begin();
         it!=measurements.end(); it++ )
    {
        thv.push_back(Measurement());
        decode_temperature_measurement(*it,thv.back().t,thv.back().v);
        
        thv.back().t = timestamp_to_time((char*)(*it)->GetField(0));
        thv.back().v = atof((*it)->GetField(3));
        
        if( thv.back().v<-10 || thv.back().v>50 )
            thv.pop_back(); // Bad temperature measurement, remove it!
    }
    
    // Change very first and very last measurement time to t1
    if( thv.front().t<t1 )
        thv.front().t=t1;

    // Add very very last measurement with time t2
    assert(thv.back().t<t2);
    thv.push_back(thv.back());
    thv.back().t=t2;

    for( int i=0; i<thv.size(); i++ )
        g_Temperature->SetPoint(g_Temperature->GetN(),thv[i].t,thv[i].v);
}

////////////////////////////////////////////////////////////////////////////////

void ChannelStatus::ReadCurrent(void)
{
    if( NULL==g_current )
        throw "ChannelStatus::ReadTemperature(): current container does not exist.";

    // First we read the measurements.    
    vector<TSQLRow*> measurements;
    TSQLResult *table = get_measurements(db,detector.c_str(),channel.c_str(),t1,t2,
                                         "dcs.current",measurements);
    if( measurements.empty() )
    {
        printf("No current measurements at all!\n");
        return;
    }

    // Print debug information about the measurements we retrieved from DB
    if( debug )
    {
        // Get the very first and very last measurements
        time_t db_t1,db_t2,db_last;
        float current;
        decode_current_measurement(measurements.front(),db_t1,current);
        decode_current_measurement(measurements.back(),db_t2,current);

        printf("detector %s   channel %s\n",detector.c_str(),channel.c_str());
        printf("Request: %s time interval.\n",interval_s(t1,t2).c_str());
        printf("DB-delivered: %s time interval with %d measurements.\n",
                interval_s(db_t1,db_t2).c_str(), measurements.size());
        for( int i=0; i<measurements.size(); i++ )
            printf("%s  %s\n",measurements[i]->GetField(0),measurements[i]->GetField(6));
    }

    // convert all measurements to Measurement (but now we have CURRENT instead of VOLTAGE).
    vector<Measurement> thv;
    for( vector<TSQLRow*>::const_iterator it=measurements.begin();
         it!=measurements.end(); it++ )
    {
        thv.push_back(Measurement());
        decode_current_measurement(*it,thv.back().t,thv.back().v);
    }
    
    // Change very first and very last measurement time to t1
    if( thv.front().t<t1 )
        thv.front().t=t1;

    // Add very very last measurement with time t2
    assert(thv.back().t<t2);
    thv.push_back(thv.back());
    thv.back().t=t2;

    for( int i=0; i<thv.size(); i++ )
        g_current->SetPoint(g_current->GetN(),thv[i].t,thv[i].v*1e6);  // convert Amps to micro-Aamps
}

////////////////////////////////////////////////////////////////////////////////

void ChannelStatus::DrawInOneWindow(void)
{
    char name[111], title[111];
    sprintf(name,"hStatus_%s_%s",detector.c_str(),channel.c_str());
    sprintf(title,"HV status of %s %s for %s",
            detector.c_str(),channel.c_str(),interval_s(t1,t2).c_str());
    
    TCanvas * canvas = new TCanvas(name+1,title);

    TH1F *h = new TH1F(name,title,bins,t1,t2);
    h->SetStats(false);
    h->GetYaxis()->SetTitle(g_HV->GetTitle());
    h->GetXaxis()->SetTitle("Time");
    h->GetXaxis()->SetTimeDisplay(1);
    h->GetXaxis()->SetTimeFormat("#splitline{%d/%m}{%H:%M}");
    h->GetXaxis()->SetLabelSize(0.03);
    h->SetLabelOffset(0.02);
    h->SetMinimum(0);
    h->SetMaximum(1900);
    h->Draw();
    canvas->Update();

    if( NULL!=g_HV )
    {
        //draw_graph(g_HV,0,nominal_HV+250,color_HV,"High Voltage, V","PL");
        g_HV->Draw("PL");
        canvas->Update();
    }

    if( NULL!=g_Temperature )
    {
        if( g_Temperature->GetN()==0 )
            ReadTemperature();
        draw_graph(g_Temperature,1,60,color_temperature,"Temperature, ^{0}C","P");
        canvas->Update();
    }
}

////////////////////////////////////////////////////////////////////////////////

void ChannelStatus::Draw(const char *options)
{
    char name[111], title[111];
    sprintf(name,"Status_%s_%s",detector.c_str(),channel.c_str());
    sprintf(title,"Status of %s %s for %s",
            detector.c_str(),channel.c_str(),interval_s(t1,t2).c_str());
    
    TCanvas * canvas = new TCanvas(name,title);
    gStyle->SetPadBorderMode(0);
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    
    int next=0, ngraphs=strlen(options);
    if( ngraphs>1 )
        canvas->Divide(1,ngraphs,0,0);
    else
        next=-1;

    // Create a histogram which is used to draw axis

    if( NULL!=strchr(options,'A') && h_avail!=NULL )
    {
        canvas->cd(++next);
        if( next>1 )
            gPad->SetTopMargin(0);
        if( next!=ngraphs )
            gPad->SetBottomMargin(0);
        gPad->SetTickx();
        gPad->SetGridx();
        h_avail->Draw();
        canvas->Update();
    }

    if( NULL!=strchr(options,'V') && NULL!=g_HV )
    {
        TGraph *g=g_HV;
        if( g->GetN()==0 )
            ReadCurrent();
        canvas->cd(++next);
        if( next>1 )
            gPad->SetTopMargin(0);
        if( next!=ngraphs )
            gPad->SetBottomMargin(0);
        gPad->SetTickx();
        gPad->SetGridx();
        draw_graph(g,t1,t2,0,1900);
    }

    if( NULL!=strchr(options,'I') && NULL!=g_current )
    {
        TGraph *g=g_current;
        if( g->GetN()==0 )
            ReadCurrent();
        canvas->cd(++next);
        if( next>1 )
            gPad->SetTopMargin(0);
        if( next!=ngraphs )
            gPad->SetBottomMargin(0);
        gPad->SetTickx();
        gPad->SetGridx();
        draw_graph(g,t1,t2,0,20);
    }
    
    if( NULL!=strchr(options,'T') && NULL!=g_Temperature )
    {
        TGraph *g=g_Temperature;
        if( g->GetN()==0 )
            ReadTemperature();
        canvas->cd(++next);
        if( next>1 )
            gPad->SetTopMargin(0);
        if( next!=ngraphs )
            gPad->SetBottomMargin(0);
        gPad->SetTickx();
        gPad->SetGridx();
        draw_graph(g,t1,t2,10,50);
    }

    canvas->cd(0);
    //gStyle->SetPadBorderMode(1);
    //gStyle->SetOptStat(1);
    //gStyle->SetOptTitle(1);
}

////////////////////////////////////////////////////////////////////////////////
