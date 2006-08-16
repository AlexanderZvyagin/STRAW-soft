/*! \file dcs.cc
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include <cassert>
#include <ctime>
#include <string>
#include <cstring>
#include <vector>
#include <map>

#include "TROOT.h"
#include "TH1.h"
#include "TH2.h"
#include "TDatime.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TSQLServer.h"
#include "TGraph.h"
#include "TGaxis.h"

#include "version.h"
#include "ChannelStatus.h"
#include "dcs.h"
#include "MainCanvas.h"

TROOT root("","");
using namespace std;

#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////////////

bool debug=false;
TSQLServer *db = NULL;
int bins_number=2000;   // number of bins for 'all' channels histograms
MainCanvas *main_canvas=NULL;

////////////////////////////////////////////////////////////////////////////////

void dcs(const char *st1,const char *st2)
{
    TDatime
        t1 = timestamp_to_time(st1,true),
        t2 = timestamp_to_time(st2,true);

    char buf[99];
    sprintf(buf,"STRAW HV status for period %s",interval_s(t1.Convert(),t2.Convert()).c_str());

    main_canvas = new MainCanvas("canvas",buf,1200,900);
    TPad *many_boxes = new TPad("many_boxes"," ",0,0,1,0.92);
    many_boxes->Draw();
    TPaveText *title1 = new TPaveText(0.1,0.93,0.9,0.99);
    title1->AddText(buf);
    title1->Draw();
    
    dcs(many_boxes,st1,st2);
}

////////////////////////////////////////////////////////////////////////////////

map<string,ChannelStatus*> all_channels;

void dcs(TPad *canvas,const char *st1,const char *st2)
{
    TDatime
        t1 = timestamp_to_time(st1,true),
        t2 = timestamp_to_time(st2,true);
    char buf[99];
    sprintf(buf,"STRAW HV status for period %s",interval_s(t1.Convert(),t2.Convert()).c_str());

    double dt=(t2.Convert()-t1.Convert())/double(bins_number);
    sprintf(buf,"Channels availability in the last %g seconds",dt);
    TH1F *h_channels_not_available = new TH1F("channels_availability",buf,bins_number,t1.Convert(),t2.Convert());
    h_channels_not_available->SetStats(false);
    h_channels_not_available->GetYaxis()->SetTitle("Channels availability");
    h_channels_not_available->GetXaxis()->SetTitle("Time");
    h_channels_not_available->GetXaxis()->SetTimeDisplay(1);
    h_channels_not_available->GetXaxis()->SetTimeFormat("#splitline{%d/%m}{%H:%M}");
    h_channels_not_available->GetXaxis()->SetLabelSize(0.03);
    h_channels_not_available->SetLabelOffset(0.02);

    sprintf(buf,"Number of tripped channels in last %g seconds",dt);
    TH1F *h_all_trips = new TH1F("tripped_channels",buf,bins_number,t1.Convert(),t2.Convert());
    h_all_trips->SetStats(false);
    h_all_trips->GetYaxis()->SetTitle("Number of tripped channels");
    h_all_trips->GetXaxis()->SetTitle("Time");
    h_all_trips->GetXaxis()->SetTimeDisplay(1);
    h_all_trips->GetXaxis()->SetTimeFormat("#splitline{%d/%m}{%H:%M}");
    h_all_trips->GetXaxis()->SetLabelSize(0.03);
    h_all_trips->SetLabelOffset(0.02);

    map<string,set<string> > detectors;
    get_detectors(*db,t1.Convert(),t2.Convert(),detectors);
    int maximum_channels=0;
    for( map<string,set<string> >::const_iterator det=detectors.begin(); det!=detectors.end(); det++ )
        if( maximum_channels < det->second.size() )
            maximum_channels = det->second.size();

    if( maximum_channels==0 )
    {
        printf("No channels are found!\n");
        return;
    }

    canvas->Divide(maximum_channels+1,detectors.size(),0,0);

    int det=0;
    for( map<string,set<string> >::const_iterator dett=detectors.begin(); dett!=detectors.end(); dett++,det++ )
    {
        const char *detector_name=dett->first.c_str();
        
        if( debug )
            printf("Detector %s has %d number of channels.\n",detector_name,dett->second.size());

        int pad_number=1+(maximum_channels+1)*det;
        canvas->cd(pad_number);
        //canvas->GetPad(pad_number)->SetFillColor(7);

        TText *txt1=new TText(0.2,0.5,detector_name);  // Print name of the Detectors
        txt1->SetTextSize(0.25);
        txt1->Draw();

        int cha=0;
        for( set<string>::const_iterator it=dett->second.begin(); it!=dett->second.end(); it++,cha++ )
        {
            const char *channel_name=it->c_str();
            float v_ok=0, v_low=0;
            TVirtualPad *pad=canvas->GetPad(2+(maximum_channels+1)*det+cha);

            // Set the pad name and title
            char buf[99], buf2[44];
            sprintf(buf,"%s %s",detector_name,channel_name);
            pad->SetTitle(buf);
            sprintf(buf,"%s_%s",detector_name,channel_name);
            pad->SetName(buf);

            // Add mouse-click action
            sprintf(buf2,"dcs(1,%d,%d)",t1.Convert(),t2.Convert());
            pad->AddExec(buf,buf2);

            // Get channels status
            ChannelStatus * &channel_status = all_channels[pad->GetTitle()];
            delete channel_status;
            channel_status=new ChannelStatus(detector_name,channel_name,t1.Convert(),t2.Convert(),*db);
            channel_status->all_not_available = h_channels_not_available;
            channel_status->all_trips = h_all_trips;
            channel_status->CalculateStatus();

            // Fill the pad
            fill_pad(pad,channel_name,
                     channel_status->voltage_ok,channel_status->voltage_low,
                     channel_status->trips);

            // Update the main window with this pad.
            canvas->Update();
        }
    }
    canvas->cd();

    if(1)
    {
        new TCanvas;
        h_channels_not_available->Draw();
        new TCanvas;
        h_all_trips->Draw();
    }
}

////////////////////////////////////////////////////////////////////////////////

void dcs(int n,time_t t1,time_t t2)
{
    int event = gPad->GetEvent();
    if (event != 11)
        return;

    char detector[33],channel[33];
    sscanf(gPad->GetTitle(),"%s %s",detector,channel);

    string options;
    
    if( main_canvas->GetDrawAvailability() )
        options += 'A';

    if( main_canvas->GetDrawVoltage() )
        options += 'V';

    if( main_canvas->GetDrawCurrent() )
        options += 'I';

    if( main_canvas->GetDrawTemperature() )
        options += 'T';

    all_channels[gPad->GetTitle()]->DrawInOneWindow();
    //all_channels[gPad->GetTitle()]->Draw(options.c_str());
}

////////////////////////////////////////////////////////////////////////////////

void decode_temperature_measurement(TSQLRow *rr,time_t &time,float &temperature)
{
    if( rr==NULL )
    {
        printf("decode_HV_measurement(): it is NULL!\n");
        time=0;
        temperature=-1;
        return;
    }

    time=timestamp_to_time((char*)rr->GetField(0));
    temperature=atof(rr->GetField(1));
}

////////////////////////////////////////////////////////////////////////////////

void decode_HV_measurement(TSQLRow *rr,time_t &time,float &voltage)
{
    if( rr==NULL )
    {
        printf("decode_HV_measurement(): it is NULL!\n");
        time=0;
        voltage=-1;
        return;
    }

    time=timestamp_to_time((char*)rr->GetField(0));
    voltage=atof(rr->GetField(6));
}

////////////////////////////////////////////////////////////////////////////////

void decode_current_measurement(TSQLRow *rr,time_t &time,float &current)
{
    if( rr==NULL )
        throw "decode_current_measurement(): it is NULL!";

    time=timestamp_to_time((char*)rr->GetField(0));
    current=atof(rr->GetField(6));
}

////////////////////////////////////////////////////////////////////////////////

#include <popt.h>
#include "TRint.h"

int main(int argc,const char *argv[])
{
    char
        *dcs_files_dir      = "/dcs/home/data/xls_report",
        *user               = "anonymous",
        *password           = "",
        *st1                = "",
        *st2                = "",
        *dbhost             = "na58pc052.cern.ch",
        *db_write           = "VITG";

    struct poptOption options[] =
    {
        { "t1",         '\0', POPT_ARG_STRING,  &st1,           0,
                                      "Starting of a time unterval, YYYY-MM-DD HH:MM:SS", "time_start" },
        { "t2",         '\0', POPT_ARG_STRING,  &st2,           0,
                                      "Ending of a time interval, YYYY-MM-DD HH:MM:SS", "time_end" },
        { "debug",      '\0', POPT_ARG_INT,  &debug,        0,
                                      "Turn on debug messages.", "" },
        { "bins",       '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &bins_number,        0,
                                      "Number of bins for histograms tripped_channels "
                                      "and channels_availability", "bins" },
        { "db-user",    '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &user,           0,
                                      "User name", "user" },
        { "db-password",'\0', POPT_ARG_STRING,  &password,      0,
                                      "Password for 'user'", "user_password" },
        { "db-files",   '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &dcs_files_dir, 0,
                                      "Directory with the DB files", "dir" },
        { "db-host",    '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &dbhost,          0,
                                      "Host name of the DB", "hostname" },
        { "db-write",   '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &db_write,          0,
                                      "What to write to the DB. "
                                      "'V' - channel high voltage; "
                                      "'I' - channel current; "
                                      "'T' - channel temperature; "
                                      "'G' - general hall info (temperature, pressure, humidity)", "[V|I|T|G]" },
        { "version",    'v',  POPT_ARG_NONE,  NULL,           1,
                                      "Print the version number" },
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    poptContext poptcont=poptGetContext(NULL,argc,argv,options,0);
    poptSetOtherOptionHelp(poptcont,
        "<options...>\n"
        "  * To monitor DCS values:\n"
        "    $ dcs --t1=YYYYMMDD[HH|MM|SS] --t2=YYYYMMDD[HH|MM|SS]\n"
        "  * To put values into DB:\n"
        "    $ dcs --dbfiles=/path/to/files --user=name --password=COMPASS-PASSWORD\n"
        "    To create 'db-files' you have to login on 'pccompass04' under\n"
        "    'compassdcs' user name and run program\n"
        "    $ /afs/cern.ch/compass/detector/straw/straw_dcs_files\n"
        "    \n"
        "  Program documentation can be found at /afs/cern.ch/compass/detector/straw/doc/DCS-M"
        "  Author: Alexander Zvyagin <Alexander.Zviagine@cern.ch>"
        "    \n"
    );

    if( argc<=1 )
    {
        // No options/arguments were given
        poptPrintHelp(poptcont,stdout,0);
        return 1;
    }

    int rc=poptGetNextOpt(poptcont);
    switch( rc )
    {
        case -1:
            break;
        case 1:
            printf("%s\n",CS::version);
            break;
        default:
	        fprintf(stderr, "bad argument %s: %s\n",
		            poptBadOption(poptcont, POPT_BADOPTION_NOALIAS),
		            poptStrerror(rc));
            return 1;
    }
    
    //--------------------
    // Open DB

    string db_path=string("mysql://")+string(dbhost);
    db = TSQLServer::Connect(db_path.c_str(),user,password);
    if( db==NULL )
    {
        printf("Can not open DB.\n");
        return 1;
    }

    //--------------------

    // For the data base write access we must have a password.
    if( strlen(password)>0 )
    {
        string s=dcs_files_dir;

        if( NULL!=strchr(db_write,'V') )
            db_write_HV                         (db,(s+"/ISEG_VMon.txt" ).c_str());

        if( NULL!=strchr(db_write,'T') )
            db_write_temperature                (db,(s+"/ISEG_Temp.txt" ).c_str());

        if( NULL!=strchr(db_write,'I') )
            db_write_current                    (db,(s+"/ISEG_IMon.txt" ).c_str());

        if( NULL!=strchr(db_write,'G') )
            db_write_hall_pressure_temperature  (db,(s+"/general.txt"   ).c_str());
    }

    //--------------------

    if( strlen(st1)>0 && strlen(st2)>0 )
    {
        TRint theApp("App", &argc, const_cast<char**>(argv) );

        dcs(st1,st2);
        theApp.Run();
    }

    poptFreeContext(poptcont);
}

////////////////////////////////////////////////////////////////////////////////
