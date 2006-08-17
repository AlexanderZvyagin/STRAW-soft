#include <map>
#include <vector>

float my_atof(char *s)
{
  return s==NULL ? -999999 : atof(s);
}

void set_errors(TH1 *h,float error)
{
    printf("Setting errors to %g\n",error);
    for( int b=1; b<=h->GetXaxis()->GetNbins(); b++ )
        if( h->GetBinError(b)>0 )
            h->SetBinError(b,error);
}

vector<string> __fields_name;
vector<long>   __fields_num;
int fields(char *name,int f=-1)
{
    if( f>=0 )
    {
        __fields_name.push_back(name);
        __fields_num.push_back(f);
    }
    else
    {
        for( unsigned i=0; i<__fields_name.size(); i++ )
            if( __fields_name[i]==name )
                return __fields_num[i];
        printf("fields: not found: %s\n",name);
        exit(1);
    }
}

void t0ud(char *TABLE,float t0_delta_print=1)
{
    float dT=2;
    char q[222];
    int nc = 1;
    TH1F* h_dT  = new TH1F("dT","T_{0}^{u}-T_{0}^{d} dist",100,-5,5);
    h_dT -> GetXaxis() -> SetTitle("Time difference, [ns]");
    h_dT -> GetYaxis() -> SetTitle("Entries/0.1[ns]");
    int cards_max=200;
    TH1F* h_dT_card  = new TH1F("dT_card","T_{0}^{u}-T_{0}^{d} vs geoID",cards_max,0,cards_max);
    //h_dT_card -> GetXaxis() -> SetTitle("Card geographical address.");
    h_dT_card -> GetYaxis() -> SetTitle("Time difference, [ns]");
    h_dT_card->SetMinimum(-dT);
    h_dT_card->SetMaximum( dT);
    
    // Try to connect to the database.
    TSQLServer *server = TSQLServer::Connect("mysql://na58pc052.cern.ch/STDC","","");
    if( server==NULL )
    {
        printf("Cannot connect to the server!\n");
        return;
    }

    for( int i_geo_id = 0; i_geo_id<1000; i_geo_id++ )
    {
        float T0_u =0, T0_d = 0;

        //creating the command line
        sprintf(q,"select * from %s where comment='geoID %d'",TABLE,i_geo_id);

        // OK, the DB connection was successful
        TSQLResult *result = server->Query(q);
        if( result==NULL )
        {
            printf("Bad query!\n");
            return;
        }

        // Now we read the field names
        //map<string,int> fields;
        for( int i=0; i<result->GetFieldCount(); i++ )
            fields(result->GetFieldName(i),i);

        TSQLRow *record=NULL;
        while( NULL!=(record=result->Next()) )
        {
            // Read fields!
            float data = my_atof(record->GetField(fields("data")));
            
            if(data>0))//condition for the card to exist or not dead
            {
                float T0            = my_atof(record->GetField(fields("T0")));
                char *s_Detector	= record->GetField(fields("detector"));
        
                char s_lab[222];
                sprintf(s_lab,"%d %s",i_geo_id,s_Detector);
                h_dT_card->GetXaxis()-> SetBinLabel(nc,s_lab);
 
                if(s_Detector[6]=='u')
                {
                    T0_u = T0;
                    h_dT_card->Fill(nc,T0);
                }
                else if(s_Detector[6]=='d')
                {
                    T0_d = T0;
                    h_dT_card->Fill(nc,-T0);
                    h_dT_card->SetBinError(nc,1);       // temporary
                    nc++;
                   
                } 
            }//End of if(data>0)
    
        }//end of while over record
        
        delete result;

        if(T0_u != 0 && T0_d != 0)
            h_dT->Fill(T0_u-T0_d);
//        h_1->Fill(i_geo_id,T0_u-T0_d);

        if( fabs(T0_u-T0_d)>t0_delta_print )
            printf("\"%s\",%d,%f:%f  diff=%g\n",s_Detector,i_geo_id,T0_u,T0_d,fabs(T0_u-T0_d));
    }//end of for over geo_id	

    char c_2_title[222];
    sprintf(c_2_title,"c_2_%s",TABLE);
    TCanvas *c_2 = new TCanvas("c_2",c_2_title);
    h_dT->Draw();
    TF1 *f=new TF1("fit","gaus");
    f->SetParameters(h_dT->GetEntries(),0,0.1);
    f->FixParameter(1,0);
    h_dT->Fit(f,"","",-dT,dT);
    
    set_errors(h_dT_card,f->GetParameter(2)/sqrt(2.));
    
    char c_3_title[222];
    sprintf(c_3_title,"c_3_%s",TABLE);

    TCanvas *c_3 = new TCanvas("c_3",c_3_title);
    title = new TPaveLabel(0.1,0.955,0.9,0.99,"Cardwise T_{0} diff for u and d Layer");
    title->SetFillColor(16);
    title->Draw();
    TPad *pad = new TPad("pad","pad",0.01,0.01,0.99,0.95,0);
    pad->Draw();
    pad->cd();
    h_dT_card->GetXaxis()->SetRange(1,nc+1);
    h_dT_card->Draw();


    TCanvas *c=new TCanvas;
    c->Divide(2,1);
    c->cd(1);
    h_dT_card->Draw();
    c->cd(2);
    h_dT->Draw();
    c->cd(0);


    if(0)
    {
        c_2->Print("",".eps");

        TFile fff("dT.root","RECREATE","",9);
        c->Write();
        fff.Write();
    }

}
