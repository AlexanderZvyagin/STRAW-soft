void T0_geoID_pics(char *table="T0_geoID_i")
{
    TString ext(".eps");

    gROOT->LoadMacro("t0ud.C");
    //gROOT->LoadMacro("t0_CARD.C");
    gROOT->LoadMacro("det-t0-geo.C");
    gROOT->LoadMacro("report.C");

    t0ud(table);
    
    // ----------------
    TCanvas *c_dT = new TCanvas;
    TH1 *h_dT = gDirectory->Get("dT");

    gStyle->SetOptTitle(false);
    h_dT->Draw();
    c_dT->Print(TString("T0u-T0d") +ext,ext);
    // ----------------
    
    float T0_error = h_dT->GetFunction("fit")->GetParameter(2)/sqrt(2.);
    
    ForReportCreation R;
    for( char *st=R.STDC_iterator(true); st!=NULL; st=R.STDC_iterator(false) )
        det_t0_geo(st,table,T0_error);

    gStyle->SetOptTitle(true);
    gStyle->SetOptStat(false);
    for( map<string,void*>::iterator it=det_geoID.begin(); it!=det_geoID.end(); it++ )
    {
        char *det = it->first.c_str();
        TH1F *h   = it->second;

        if( h->GetEntries()==0 )
            continue;

        // Get average T0
        float T0_min=1e9, T0_max=-1e9, T_gap=5;
        for( int bin=1; bin<=h->GetXaxis()->GetNbins(); bin++ )
        {
            if( h->GetBinError(bin)==0 )
                continue;
            if( h->GetBinContent(bin)>T0_max )
                T0_max = h->GetBinContent(bin);
            if( h->GetBinContent(bin)<T0_min )
                T0_min = h->GetBinContent(bin);
        }

        h->SetMinimum(T0_min-T_gap);
        h->SetMaximum(T0_max+T_gap);

        TCanvas *c = new TCanvas(det,det);
        c->SetBottomMargin(0.28);
        h->LabelsOption("v","X");
        h->Draw();
        c->Print("",".eps");
    }
}
