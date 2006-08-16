void pictures(void)
{
    gStyle->SetOptDate(0);
    TH1F *h;
    TCanvas *c=new TCanvas;

    
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    h = (TH1F*) gDirectory->Get("ST03X1db_CORAL_sp_eff_Xi100_nh40");
    h->GetXaxis()->SetRangeUser(26,28.5);
    h->Draw();
    c->Print("spacer_inefficiency.eps");


        
}
