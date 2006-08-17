#include <map>
#include <vector>
#include <string>

float my_atof(char *s)
{
  return s==NULL ? -999999 : atof(s);
}

map<string,void*> det_geoID;

void set_histogram_bin(TH1F *h,char *detector,char *geo_id,int chf,int pos,float T0,float T0_err)
{
    int
        ud      = detector[6]=='u'?0:1,
        cards_b = detector[4]=='Y'?6:7, // number of 6mm cards
        cards_c = detector[4]=='Y'?2:3, // number of 10mm cards
        abc     = detector[7]=='a'? 0 : (detector[7]=='b'?cards_c:cards_b+1+cards_c);
    int card=-1, ph=false;
    
    switch(chf)
    {
        case   0:   card=0;             break;
        case  32:   card=1;             break;
        case  64:   card=2;             break;
        case  80:   card=3; ph=true;    break;  // Y-chamber PH
        case  95:                               // X-chamber PH region
                if( pos==-1 )
                {
                    card=3;             break;  // normal X-card
                }
                else
                {
                    card=4; ph=true;    break;   // PH card
                }
                                        
        case  96:   card=4;             break;  // Y-chamber card after a PH
        case 127:   card=5;             break;  // X-card
        case 128:   card=5;             break;  // Y-card
        case 158:   card=6;             break;  // X-card
        case 160:   card=6;             break;  // Y-card
        case 190:   card=7;             break;  // X-card
        default:
            printf("Unknown chf=%d for %s\n",chf,detector);
    }
    
    char
        * cardX_names[] = {},
        * cardY_names[] = {};
    
    int bin = 1+(card+abc)*2 + ud;
    
    h->SetBinContent(bin,T0);
    h->SetBinError(bin,T0_err);
    
    char label[66];
    sprintf(label,"%s%s-%c %s",ph?"PH ":"",detector[7]=='b'?"6mm":"10mm",ud?'d':'u',geo_id);
    
    //printf("bin(%s %s chf=%d pos=%d) = %d\n",detector,geo_id,chf,pos,bin);
    
    h->GetXaxis()->SetBinLabel(bin,label);
}

void det_t0_geo(char *detector,char *table,float error=0.2)
{
    char q[222];
    sprintf(q,"%6.6s",detector);
    TH1F *h = det_geoID[q];
    if( h==NULL )
    {
        int nbins = 2*(detector[4]=='Y'?11:14);  // factor 2 - for u/d layers.
        h = new TH1F(q,q,nbins,0,nbins);
        h->GetXaxis()->SetTitle("T_0 time, [ns]");
        det_geoID[q] = h;
    }

    sprintf(q,"select * from %s where detector='%s' and chf>=0 and chf<=221",table,detector);
    
    // Try to connect to the database.
    TSQLServer *server = TSQLServer::Connect("mysql://na58pc052.cern.ch/STDC","","");
    if( server==NULL )
    {
        printf("Cannot connect to the server!\n");
        return;
    }

    // OK, the DB connection was successful
    TSQLResult *result = server->Query(q);
    if( result==NULL )
    {
        printf("Bad query!\n");
        delete server;
        return;
    }

    if(result->GetRowCount()==0)
    {
        //printf("No entries for detector %s on table %s \n",detector,table);
        delete server;
        return;
    }
    //printf("%d entries were selected.\n",result->GetRowCount());


    // Now we read the field names
    map<string,int> fields;
    for( int i=0; i<result->GetFieldCount(); i++ )
        fields[result->GetFieldName(i)] = i;
    
    





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
                T0      = my_atof(record->GetField(fields["T0"])),
                Xi2     = my_atof(record->GetField(fields["Xi2"])),
                W       = my_atof(record->GetField(fields["W"])),
                chf     = my_atof(record->GetField(fields["chf"])),
                chl     = my_atof(record->GetField(fields["chl"])),
                pos     = my_atof(record->GetField(fields["pos"]));
        
            char *geo_id = record->GetField(fields["comment"]);

            set_histogram_bin(h,detector,geo_id,int(chf),int(pos),T0,error);
//             
//             int cd_ID = F_CARD_ID(chf,pos,chamber);
//             printf("T0(%.0f)= %f \n",cd_ID,T0);
//             h_t0_card->Fill(cd_ID,T0);
//             //Setting the name of the bin corresponding to the channel
//             char s_bin_label[222];
//             sprintf(s_bin_label,"card %d (%s)",cd_ID+1,geo_id);
//             h_t0_card->GetXaxis()->SetBinLabel(cd_ID+1,s_bin_label);
// //                A_T0[cd_ID] = T0;
// 

        }//End of if(data>0)
     
     }//end of while over record
    	
    delete server;
}
