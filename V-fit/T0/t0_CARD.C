
#include <map>


float my_atof(char *s)
{
  return s==NULL ? -999999 : atof(s);
}

void set_errors(TH1 *h,float error)
{
    printf("Setting errors to %g\n",error);
    for( int b=1; b<=h->GetXaxis()->GetNbins(); b++ )
        h->SetBinError(b,error);
}


TH1* t0_CARD(char *DETECTOR,char *TABLE,float T0_ERR = 0.2)
//void t0_CARD(char *DETECTOR,char *TABLE = "ST03_4", float *A_T0)
{
    //gROOT->Reset();


     //COMMENT THE FOLLOWING IF CALLED
//     gROOT->LoadMacro("F_get_t0_ref.C");
     gROOT->LoadMacro("F_CARD_ID.C");

    gStyle->SetOptStat(0);//No stat box

    char q[222];
    //creating the command line
    sprintf(q,"select * from %s where detector='%s' and chf>=0 and chf<=221",TABLE,DETECTOR);
    
    char chamber = DETECTOR[4];
    printf("Detector = %s, ",DETECTOR);
    if(chamber == 'X' ||chamber == 'U' || chamber == 'V')int card_number = 8;
    if(chamber == 'Y')
    {
        int card_number = 7;
//        A_T0[7] = A_T0[8];
        
    }
    printf(" card num = %d\n",card_number);

    
    // Try to connect to the database.
    TSQLServer *server = TSQLServer::Connect("mysql://na58pc052.cern.ch/STDC","","");
    if( server==NULL )
    {
        printf("Cannot connect to the server!\n");
        return NULL;
    }

    // OK, the DB connection was successful
    TSQLResult *result = server->Query(q);
    if( result==NULL )
    {
        printf("Bad query!\n");
        return NULL;
    }

    if(result->GetRowCount()==0)
    {
        printf("No entries for detector %s on table %s \n",DETECTOR,TABLE);
        return NULL;
    }
    printf("%d entries were selected.\n",result->GetRowCount());


    // Now we read the field names
    map<string,int> fields;
    for( int i=0; i<result->GetFieldCount(); i++ )
        fields[result->GetFieldName(i)] = i;
    
    

    //creating hist name
    char s_h_t0_card[222];
    sprintf(s_h_t0_card,"h_t0_card_%s",DETECTOR);

//With T0 ref
//    TH1F* h_t0_card  = new TH1F(s_h_t0_card,"Fitted T_{0}-T_{0}(ref)",card_number,0,card_number);
//    h_t0_card->GetYaxis()->SetTitle("T_{0}^{fit} - T_{0}^{ref}[ns]");

//Without T0 ref
    TH1F* h_t0_card  = new TH1F(s_h_t0_card,"Fitted T_{0}",card_number,0,card_number);
    h_t0_card->GetYaxis()->SetTitle("T_{0}^{fit}[ns]");






    TSQLRow *record=NULL;
    while( NULL!=(record=result->Next()) )
    {
        // Read fields!
        float data = my_atof(record->GetField(fields["data"]));
        
       
        char *s_Detector	= record->GetField(fields["detector"]);
            
//        float T0_ref = F_get_t0_ref(s_Detector);
     
        if(data>0))//condition for the straw not to be dead
        {
            float
//                T0                  = my_atof(record->GetField(fields["T0"])) - T0_ref,//With T0 ref
                T0                  = my_atof(record->GetField(fields["T0"])),
                Xi2                  = my_atof(record->GetField(fields["Xi2"])),
                W                  = my_atof(record->GetField(fields["W"])),
                chf                 = my_atof(record->GetField(fields["chf"])),
                chl                 = my_atof(record->GetField(fields["chl"])),
                pos                 = my_atof(record->GetField(fields["pos"]));
        
            char *geo_id = record->GetField(fields["comment"]);
            
            int cd_ID = F_CARD_ID(chf,pos,chamber);
            printf("T0(%.0f)= %f \n",cd_ID,T0);
            h_t0_card->Fill(cd_ID,T0);
            //Setting the name of the bin corresponding to the channel
            char s_bin_label[222];
            sprintf(s_bin_label,"card %d (%s)",cd_ID+1,geo_id);
            h_t0_card->GetXaxis()->SetBinLabel(cd_ID+1,s_bin_label);
//                A_T0[cd_ID] = T0;


        }//End of if(data>0)
     
     }//end of while over record
    	
    if(chamber == 'Y')
    {
//        A_T0[7] = -9999;
    }
// cout <<"in t0_card\n";
// for(int j =0;j<8;j++)printf("T0(%d) = %f\n",j,A_T0[j]);
 	
    
    char c_name[222];
    sprintf(c_name,"c_%s",s_Detector);
    TCanvas *c_666 = new TCanvas(c_name,c_name);
    
    char s_title[555];
//    sprintf(s_title,"%s  T_{0}(ref) = %.2f, Table : %s",s_Detector,T0_ref,TABLE);//With T0 ref
    sprintf(s_title,"%s, Table : %s",s_Detector,TABLE);
    title = new TPaveLabel(0.1,0.955,0.9,0.99,s_title);
    title->SetFillColor(16);
    title->Draw();
     
     TPad *pad = new TPad("pad","pad",0.01,0.01,0.99,0.95,0);
     pad->Draw();
     pad->cd(0);
     set_errors(h_t0_card,T0_ERR);
     h_t0_card->Draw();

    //c_666->Print(TString(s_Detector)+".ps");

    delete server;
    return h_t0_card;
}






