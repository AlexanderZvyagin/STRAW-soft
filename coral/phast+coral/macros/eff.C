/*! \brief List of Cuts to be applied.
*/
struct Cuts
{
    float Xi2;
    float nhits;
};


void eff_det(char *det,Cuts &cuts)
{
    gStyle->SetOptStat(0);
    
    char name[44], title[222];
    sprintf(name,"%s_CORAL",det);

    RS.cd_data();

    TNtuple *nt=gDirectory->Get(name);
    if( nt==NULL )
    {
        printf("NTuple %s was not found in %s.\n",name,gDirectory->GetName());
        return;
    }
    
    int nx=200, ny=300;
    float rx=120, ry=300;

    sprintf(title,"%s efficiency with tracks selection: #chi^{2}<%g N_{hits}>%d",det,cuts.Xi2,cuts.nhits);
    
    RS.cd_out();
    
    TH2F *h1 = new TH2F("tr_all","all tracks",nx,-rx/2,rx/2,ny,-ry/2,ry/2);
    TH2F *h2 = new TH2F("tr_st","tracks in straw",nx,-rx/2,rx/2,ny,-ry/2,ry/2);
    TH2F *h3 = new TH2F(det,title,nx,-rx/2,rx/2,ny,-ry/2,ry/2);
    h3->GetXaxis()->SetTitle("detector coordinate, [cm]");
    h3->GetYaxis()->SetTitle("detector coordinate, [cm]");

    sprintf(name,"%s_sp_eff_Xi%d_nh%d",name,int(cuts.Xi2),int(cuts.nhits));
    TH1F *h1_spacer_eff = new TH1F("sp_eff_1","Efficiency at spacer",2400,-120,120);
    TH1F *h2_spacer_eff = new TH1F("sp_eff_2","Efficiency at spacer",2400,-120,120);
    TH1F *h3_spacer_eff = new TH1F(name,"Efficiency at spacer",2400,-120,120);
    h3_spacer_eff->GetXaxis()->SetTitle("Coordinate, [cm]");
    h3_spacer_eff->GetYaxis()->SetTitle("Efficiency");

    float d,ch, x,y,z, wx,wy, tr_q, tr_nh,tr_Xi2, tr_z1,tr_z2;
    nt->SetBranchAddress("d",&d);
    nt->SetBranchAddress("x",&x);
    nt->SetBranchAddress("y",&y);
    nt->SetBranchAddress("z",&z);
    nt->SetBranchAddress("wx",&wx);
    nt->SetBranchAddress("wy",&wy);
    nt->SetBranchAddress("ch",&ch);
    nt->SetBranchAddress("wy",&wy);
    nt->SetBranchAddress("tr_q",&tr_q);
    nt->SetBranchAddress("tr_nh",&tr_nh);
    nt->SetBranchAddress("tr_Xi2",&tr_Xi2);
    //nt->SetBranchAddress("tr_z1",&tr_z1);
    //nt->SetBranchAddress("tr_z2",&tr_z2);
    
    for( int i=0; i<nt->GetEntries() && i<RS.events_max; i++ )
    {
        if( 0>=nt->GetEntry(i) )
            break;
        
        // Only tracks with charge!
        if( tr_q==0 )
            continue;

        // Only tracks with last helix after the detector!
        //if( tr_z2/10<z )
        //    continue;
        
        // check the cuts
        if( tr_Xi2>cuts.Xi2 || tr_nh<=cuts.nhits )
            continue;
        
        h1->Fill(wx,wy);
        h1_spacer_eff->Fill(wy);

        if( fabs(d)<1000 )
        {
            h2->Fill(wx,wy);
            h2_spacer_eff->Fill(wy);
        }
    }
    
    h3->Divide(h2,h1,1,1);
    h3_spacer_eff->Divide(h2_spacer_eff,h1_spacer_eff,1,1);
    
    int colors[]={kYellow,kYellow,kYellow,kYellow,kYellow,kYellow,kYellow,kYellow,kYellow,kYellow,
                  kYellow,kYellow,kYellow,kYellow,kYellow,kYellow,kGreen,kBlue,kRed,kBlack};
    gStyle->SetPalette(sizeof(colors)/sizeof(*colors),colors);
    
    RS.cd_out();
    TCanvas *c=new TCanvas("efficiency");
    RS.registration(c);
    h3->Draw("colz");
    
    TPaveText *text_eff=new TPaveText(-40,100,40,145);
    text_eff->SetLabel(det);
    if(1)
    {
        double eff_avr=0, bins=0, tracks_all=0, tracks_accepted=0;
        for( int bx=1; bx<=nx; bx++ )
            for( int by=1; by<=ny; by++ )
            {
                // Ignore bins with number of tracks less then ...
                if( h1->GetBinContent(bx,by)<5 )
                    continue;

                eff_avr += h2->GetBinContent(bx,by)/h1->GetBinContent(bx,by);
                bins ++;

                tracks_all      += h1->GetBinContent(bx,by);
                tracks_accepted += h2->GetBinContent(bx,by);
            }
        eff_avr/=bins;
        char s[55];
        text_eff->AddText(0,0,"");
        sprintf(s,"efficiency (per bin) average: %g",eff_avr);
        text_eff->AddText(0,0,s);
        sprintf(s,"efficiency weighted with tracks: %g",tracks_all==0?0:tracks_accepted/tracks_all);
        text_eff->AddText(0,0,s);
    }
    text_eff->Draw();
    c->Write();
}

void test2(TCanvas *c,char *det1,char *det2,Cuts &cuts)
{
    c->cd(1);
    eff_det(det1,cuts);
    
    c->cd(2);
    eff_det(det2,cuts);
    
    c->cd(0);

    char s[333];
    sprintf(s,"%s_%s_Xi%d_nh%d.ps",det1,det2,int(cuts.Xi2),int(cuts.nhits));
    c->Print(s,"ps");
    sprintf(s,"%s_%s_Xi%d_nh%d.gif",det1,det2,int(cuts.Xi2),int(cuts.nhits));
    c->Print(s,"gif");
}

void eff(char *det="",char *options="")
{
    printf("This is 'eff' report for %s.\n",det);
    
    RS.dir_save();

    Cuts cuts;
    cuts.Xi2=100;  cuts.nhits=40;
    eff_det(det,cuts);
    
    RS.dir_restore();
}

bool eff_init(void)
{
    RS.reports[RS.reports_counter++]=eff;
    printf("Registering 'eff' report.\n");
}
bool init=eff_init();


// void eff1(void)
// {
//     cd_out();
//     TCanvas *c = new TCanvas("c","c",600,800);
//     c->Divide(1,2);
// 
//     char *det1="ST03X1db", *det2="ST05X1db";
// 
//     Cuts cuts;
// 
//     cuts.Xi2=200;  cuts.nhits=10;   test2(c,det1,det2,cuts);
//     cuts.Xi2=200;  cuts.nhits=20;   test2(c,det1,det2,cuts);
//     cuts.Xi2=200;  cuts.nhits=30;   test2(c,det1,det2,cuts);
//     cuts.Xi2=200;  cuts.nhits=40;   test2(c,det1,det2,cuts);
// 
//     cuts.Xi2=100;  cuts.nhits=10;   test2(c,det1,det2,cuts);
//     cuts.Xi2=100;  cuts.nhits=20;   test2(c,det1,det2,cuts);
//     cuts.Xi2=100;  cuts.nhits=30;   test2(c,det1,det2,cuts);
//     cuts.Xi2=100;  cuts.nhits=40;   test2(c,det1,det2,cuts);
// 
//     cuts.Xi2= 50;  cuts.nhits=10;   test2(c,det1,det2,cuts);
//     cuts.Xi2= 50;  cuts.nhits=20;   test2(c,det1,det2,cuts);
//     cuts.Xi2= 50;  cuts.nhits=30;   test2(c,det1,det2,cuts);
//     cuts.Xi2= 50;  cuts.nhits=40;   test2(c,det1,det2,cuts);
// }
