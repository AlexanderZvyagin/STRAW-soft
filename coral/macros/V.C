void V(void)
{
    gStyle->SetOptDate(0);
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    TH2F *h = new TH2F("h","h",200,13.45,14.2,100,-1613,-1560);
    TAxis *a;
    a=h->GetXaxis();
    a->SetTitle("Coordinate, [cm]");
    a=h->GetYaxis();
    a->SetTitle("Time, [ns]");
    a->SetTitleOffset(1.3);
    TNtuple *ST03U1db_CORAL=gDirectory->Get("ST03U1db_CORAL");
    ST03U1db_CORAL->Draw("t:wx>>h","ch==133&&tr_Xi2<200&&tr_q>20");
    
    TCanvas *c=new TCanvas;
    h->Draw();
    c->Print("V.eps");
    
    // ------
    
    TH1F *hr=new TH1F("ST03U1db_residual","Residual plot",100,-0.1,0.1);
    ST03U1db_CORAL->Draw("r-abs(d)>>ST03U1db_residual","ch==133&&tr_Xi2<200&&tr_q>20&&r>0");
    a=hr->GetXaxis();
    a->SetTitle("#Delta x, [mm]");
    a=hr->GetYaxis();
    a->SetTitle("Entries");
    hr->SetOpt(111);
    hr->Draw();
}


void V_make(char *detector)
{
    // All output objects shoud be created in the 'output' directory.
    RS.cd_out();

    char name[222];

    sprintf(name,"%s_V",detector);
    TH2F *hV=new TH2F(name,"V-plot",1000,-100.4,110.1,100,-1610,-1560);
    RS.registration(hV);

    sprintf(name,"%s_residual",detector);
    TH1F *hresidual=new TH1F(name,"Residual plot",100,-0.2,0.2);
    RS.registration(hresidual);

    // Go to the data directory.    
    RS.cd_data();
    
    sprintf(name,"%s_CORAL",detector);
    TNtuple *nt=gDirectory->Get(name);

    sprintf(name,"t:wx>>%s_V",detector);
    nt->Draw(name,"ch==128&&tr_Xi2<200","",RS.events_max);
    
    sprintf(name,"r-abs(d)>>%s_residual",detector);
    nt->Draw(name,"ch==128&&tr_Xi2<200&&r>0","",RS.events_max);
    
}

/*! \brief The main report call.
*/
void V(char *detector="",char *options="")
{
    // Inform whe world about us.
    char *my_det="ST03U1db";
    detector=my_det;
    printf("The 'V' report is created only for %s detector!\n");
    printf("This is 'V' report for %s.\n",detector);
    
    // Try to preserve the directory structure.
    RS.dir_save();

    // The main call is here.
    V_make(detector);

    // Restore the directory.    
    RS.dir_restore();
}

/*! Initialization function.
*/
bool V_init(void)
{
    RS.reports[RS.reports_counter++]=V;
    printf("Registering 'V' report.\n");
}

/*! Initilize the report during its loading!
*/
bool init=V_init();
