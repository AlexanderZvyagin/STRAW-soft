#include <vector>
#include <map>
#include <string>

void draw_mapping(TH1 *h,int ctch,int where=0);
char *channel_axis_label="CATCH: card_channel+port*64";
TH1F *draw_detector(string &name,map<string,void*> &card_channel);
map<string,void*> card_channel;

void draw(char *session1,
          char *dir1="/afs/cern.ch/user/o/onl/catch/fe-straws/data/",
          char *session2=NULL,
          char *dir2=NULL )
{
    vector<int> catches;
    catches.push_back(328);
    catches.push_back(329);
    catches.push_back(330);
    catches.push_back(331);
    catches.push_back(332);
    
    for( int i=0; i<catches.size(); i++ )
    {
        char buf[999];
        sprintf(buf,"%s/threshold_%d.dat_%s",dir1,catches[i],session1);
        
        TCanvas *c=NULL;
        TH1F *h[2]={NULL,NULL};
        
        h[0]=draw_catch(buf,catches[i],session1,c);
        
        if( session2!=NULL && dir2!=NULL )
        {
            sprintf(buf,"%s/threshold_%d.dat_%s",dir2,catches[i],session2);
            h[1]=draw_catch(buf,catches[i],session2,c);
        }

        if( h[0]!=NULL && h[1]!=NULL )
        {
            c->cd(4);
            gPad->SetGrid(0,1);

            char buf[99];
            sprintf(buf,"h_thr_diff_%d_%s_%s",catches[i],session1,session2);
            TH1F *h_thr_diff = new TH1F(buf,"Thresholds difference",1024,0,1024);
            h_thr_diff->SetStats(0);
            h_thr_diff->GetXaxis()->SetTitle(channel_axis_label);
            h_thr_diff->GetYaxis()->SetTitle("Thresholds difference, mV");
            //h_thr->SetDirectory(NULL);

            h_thr_diff->Add(h[0],h[1],1,-1);
            h_thr_diff->SetMinimum(-800);
            h_thr_diff->SetMaximum( 800);
            h_thr_diff->Draw();
            draw_mapping(h_thr_diff,catches[i]);
        }

        c->cd(0);
    }
    
    if(0)
    {
        new TCanvas("c4X");
        draw_detector("4X",card_channel);
        new TCanvas("c4U");
        draw_detector("4U",card_channel);
        new TCanvas("c4Y");
        draw_detector("4Y",card_channel);
        new TCanvas("c6X");
        draw_detector("6X",card_channel);
        new TCanvas("c6V");
        draw_detector("6V",card_channel);
        new TCanvas("c6Y");
        draw_detector("6Y",card_channel);
    }
}

TH1F* draw_catch(char *file,int catch_,char *session,TCanvas *&canvas)
{
    char buf[99];

    TNtuple nt("n","nn","port:ch:tr:rate");
    nt.ReadFile(file);
    
    if( strlen(file)<13 )
    {
        printf("Bad file name.\n");
        return NULL;
    }

    if( int(nt.GetEntries())!=1024 )
    {
        printf("Bad number of entries: %d\n",int(nt.GetEntries()));
        printf("In the file %s\n",file);
        return NULL;
    }

    const int rates_lim=30;
    
    sprintf(buf,"h_rate_%d_%s",catch_,session);
    TH1F *h_rate = new TH1F(buf,"Rates",1024,0,1024);
    h_rate->GetXaxis()->SetTitle(channel_axis_label);
    h_rate->GetYaxis()->SetTitle("Noise rate per channel, kHz");
    //h_rate->SetDirectory(NULL);
    h_rate->SetMinimum(0);
    h_rate->SetMaximum(rates_lim);
    h_rate->SetStats(0);

    sprintf(buf,"h_rate_prof_%d_%s",catch_,session);
    TH1F *h_rate_proj = new TH1F(buf,"Rates projection",100,0,rates_lim);
    //h_rate_proj->SetDirectory(NULL);
    h_rate_proj->GetXaxis()->SetTitle("Rate, kHz");
    h_rate_proj->GetYaxis()->SetTitle("Entries");

    sprintf(buf,"h_thr_%d_%s",catch_,session);
    TH1F *h_thr = new TH1F(buf,"Thresholds",1024,0,1024);
    h_thr->GetXaxis()->SetTitle(channel_axis_label);
    h_thr->GetYaxis()->SetTitle("Threshold, mV");
    //h_thr->SetDirectory(NULL);
    h_thr->SetMinimum(-2000);
    h_thr->SetMaximum(-300);
    h_thr->SetStats(0);

    for( int i=0; i<int(nt.GetEntries()); i++ )
    {
        if( !nt.GetEntry(i) )
        {
            printf("Can not read an entry??\n");
            return NULL;
        }
        
        int
            port = int(nt.GetArgs()[0]),
            ch   = int(nt.GetArgs()[1]),
            bin  = 1+ch+port*64;
        
        float
            rate = nt.GetArgs()[3],
            thr  = nt.GetArgs()[2];
        h_rate->SetBinContent(bin,rate);
        if( rate>0 )
            h_rate_proj->Fill(rate);
        h_thr ->SetBinContent(bin,(thr-128)*2040/128);
    }
    
    
    char opts[22]="";
    
    if( canvas==NULL )
    {
        sprintf(buf,"%d_%s",catch_,session);
        canvas = new TCanvas(buf,buf);
        canvas->Divide(2,2);
    }
    else
    {
        strcpy(opts,"SAME");
        h_rate      ->SetLineColor(kRed);
        h_thr       ->SetLineColor(kRed);
        h_rate_proj ->SetLineColor(kRed);
    }
    canvas->cd(1);
    //gPad->SetLogy();
    h_rate->Draw(opts);
    draw_mapping(h_rate,catch_,1);
    canvas->cd(2);
    h_rate_proj->Draw(opts);
    canvas->cd(3);
    gPad->SetGrid(0,1);
    h_thr->Draw(opts);
    draw_mapping(h_thr,catch_);
    canvas->cd(0);
    
    
    return h_thr;
}

void draw_mapping(TH1 *h,int ctch,int where)
{
    // Cards naming scheme:
    // SALEVE   10-S1 10-S2 10-S3   6-S1 6-S2 .... 6-S7  10-J3 10-J2 10-J1   JURA

    vector<string> cards;
    switch( ctch )
    {
        case 328:
            cards.push_back("4X-10-J1");    //  0
            cards.push_back("4X-10-J2");    //  1
            cards.push_back("4X-10-J3");    //  2
            cards.push_back("4X- 6-S7");    //  3
            cards.push_back("4X- 6-S6");    //  4
            cards.push_back("4X- 6-S5");    //  5
            cards.push_back("4X- 6-S4");    //  6
            cards.push_back("4X- 6-S3");    //  7
            cards.push_back("4X- 6-S2");    //  8
            cards.push_back("4X- 6-S1");    //  9
            cards.push_back("4X-10-S3");    // 10
            cards.push_back("4X-10-S2");    // 11
            cards.push_back("4X-10-S1");    // 12
            cards.push_back("4X- 6-ph");    // 13
            cards.push_back("4Y-10-B1");    // 14
            cards.push_back("4Y-10-B2");    // 15
            break;
        case 329:
            cards.push_back("4Y- 6-B1");    //  0
            cards.push_back("4Y- 6-B2");    //  1
            cards.push_back("4Y- 6-B3");    //  2
            cards.push_back("4Y- 6-B4");    //  3
            cards.push_back("4Y- 6-B5");    //  4
            cards.push_back("4Y- 6-B6");    //  5
            cards.push_back("4Y-10-T2");    //  6
            cards.push_back("4Y-10-T1");    //  7
            cards.push_back("4Y- 6-ph");    //  8
            cards.push_back("4U-10-S1");    //  9
            cards.push_back("4U-10-S2");    // 10
            cards.push_back("4U-10-S3");    // 11
            cards.push_back("4U- 6-S1");    // 12
            cards.push_back("4U- 6-S2");    // 13
            cards.push_back("4U- 6-S3");    // 14
            cards.push_back("4U- 6-S4");    // 15
            break;
        case 330:
            cards.push_back("4U- 6-S5");    //  0
            cards.push_back("4U- 6-S6");    //  1
            cards.push_back("4U- 6-S7");    //  2
            cards.push_back("4U-10-J3");    //  3
            cards.push_back("4U-10-J2");    //  4
            cards.push_back("4U-10-J1");    //  5
            cards.push_back("4U- 6-ph");    //  6
            cards.push_back("        ");    //  7
            cards.push_back("6V-10-J1");    //  8
            cards.push_back("6V-10-J2");    //  9
            cards.push_back("6V-10-J3");    // 10
            cards.push_back("6V- 6-S7");    // 11
            cards.push_back("6V- 6-S6");    // 12
            cards.push_back("6V- 6-S5");    // 13
            cards.push_back("6V- 6-S4");    // 14
            cards.push_back("6V- 6-S3");    // 15
            break;
        case 331:
            cards.push_back("6V- 6-S2");    //  0
            cards.push_back("6V- 6-S1");    //  1
            cards.push_back("6V-10-S3");    //  2
            cards.push_back("6V-10-S2");    //  3
            cards.push_back("6V-10-S1");    //  4
            cards.push_back("6V- 6-ph");    //  5
            cards.push_back("6Y-10-T1");    //  6
            cards.push_back("6Y-10-T2");    //  7
            cards.push_back("6Y- 6-B6");    //  8
            cards.push_back("6Y- 6-B5");    //  9
            cards.push_back("6Y- 6-B4");    // 10
            cards.push_back("6Y- 6-B3");    // 11
            cards.push_back("6Y- 6-B2");    // 12
            cards.push_back("6Y- 6-B1");    // 13
            cards.push_back("6Y-10-B2");    // 14
            cards.push_back("6Y-10-B1");    // 15
            break;
        case 332:
            cards.push_back("6Y- 6-ph");    //  0
            cards.push_back("6X-10-S1");    //  1
            cards.push_back("6X-10-S2");    //  2
            cards.push_back("6X-10-S3");    //  3
            cards.push_back("6X- 6-S1");    //  4
            cards.push_back("6X- 6-S2");    //  5
            cards.push_back("6X- 6-S3");    //  6
            cards.push_back("6X- 6-S4");    //  7
            cards.push_back("6X- 6-S5");    //  8
            cards.push_back("6X- 6-S6");    //  9
            cards.push_back("6X- 6-S7");    // 10
            cards.push_back("6X-10-J3");    // 11
            cards.push_back("6X-10-J2");    // 12
            cards.push_back("6X-10-J1");    // 13
            cards.push_back("6X- 6-ph");    // 14
            cards.push_back("        ");    // 15
            break;
        default:
            printf("No mapping found for catch %d!\n",ctch);
    }


    for( int port=0; port<16; port++ )
    {
        int pos_port=0, pos_det=0;
        if( where==0 )
        {
            // histogram bottom
            pos_port = h->GetMinimum();
            pos_det  = h->GetMinimum()+170;
        }
        else
        {
            // histogram top
            pos_port = h->GetMaximum()-2;
            pos_det  = h->GetMaximum()-11;
        }
    
        TLine *l = new TLine(port*64,h->GetMinimum(),port*64,h->GetMaximum());
        l->SetLineStyle(2);
        l->Draw();
        
        char buf[99];
        sprintf(buf,"%d",port);

        (new TText(10+port*64,pos_port,buf))->Draw();

        if( cards.size()>=port+1 )
        {
            if( 1 )
            {
                vector<int> *v = new vector<int>;
                card_channel[cards[port]] = v;
                for( int echan=0; echan<64; echan++ )
                    v->push_back(h->GetBinContent(port*64+echan+1));
            }

            sprintf(buf,cards[port].c_str());
            int offset=40;
            TText *m = new TText(offset+port*64,pos_det,buf);
            m->SetTextAngle(90);
            m->Draw();
        }
    }
}

TH1F *draw_detector(string &name,map<string,void*> &card_channel)
{
    TH1F *h=new TH1F(name.c_str(),name.c_str(),13*64,0,13*64);
    h->SetMinimum(-2000);
    h->SetMaximum(-300);
    h->Draw();
    gPad->Update();

    if(1)
    {
        // Draw line between cards
        TLine *l = new TLine(h->GetXaxis()->GetXmin(),-800,h->GetXaxis()->GetXmax(),-800);
        //printf("(new TLine(%d,%d,%d,%d))->Draw();\n",i*64,h->GetMinimum(),i*64,h->GetMaximum());
        l->SetLineStyle(1);
        l->SetLineWidth(3);
        l->SetLineColor(kRed);
        l->Draw();
    }
    
    char
        *cards_x[]={"-10-S1","-10-S2","-10-S3",
                    "- 6-S1","- 6-S2","- 6-S3","- 6-S4","- 6-S5","- 6-S6",
                    "-10-J3","-10-J2","-10-J1",
                    "- 6-ph"},
        *cards_y[]={"-10-B1","-10-B2",
                    "- 6-B1","- 6-B2","- 6-B3","- 6-B4","- 6-B5","- 6-B6",
                    "-10-T2","-10-T1"};
    char **cards = name[1]=='Y' ? cards_y : cards_x;
    int cards_amount = name[1]=='Y' ? sizeof(cards_y)/sizeof(*cards_y) : sizeof(cards_x)/sizeof(*cards_x);
    
    for( int i=0; i<cards_amount; i++ )
    {
        string s = name+string(cards[i]);

        if(1)
        {
            // Draw line between cards
            TLine *l = new TLine(i*64,h->GetMinimum(),i*64,h->GetMaximum());
            //printf("(new TLine(%d,%d,%d,%d))->Draw();\n",i*64,h->GetMinimum(),i*64,h->GetMaximum());
            l->SetLineStyle(2);
            l->Draw();
        }

        if(1)
        {
            // Put the card name
            TText *m = new TText(i*64+40,h->GetMinimum()+5,s.c_str());
            m->SetTextAngle(90);
            m->Draw();
        }
    
        vector<int> *v = card_channel[s];
        if( v==NULL )
        {
            printf("Card not found: %s\n",s.c_str());
            continue;
        }
        
        if( v->size()!=64 )
            printf("Bad number of channels!  %d\n",v->size());

        for( int b=0; b<v->size(); b++ )
            h->SetBinContent(i*64+b+1,(*v)[b]);
    }
    
    return h;
}

void det_canvas(int col,char *title,int cols_max=3)
{
    char *detectors[]={"4X","4U","4Y","6X","6V","6Y"};
    
    for( int i=0; i<sizeof(detectors)/sizeof(*detectors); i++ )
    {
        char buf[44];
        sprintf(buf,"%s_cmp",detectors[i]);
        TCanvas *c = gROOT->FindObject(buf);
        if( c==NULL )
        {
            c = new TCanvas(buf,buf);
            c -> Divide(cols_max,1);
        }
        
        c->cd(col);
        TH1F *h = draw_detector(detectors[i],card_channel);
        h->SetTitle(title);
    }
    
    c->cd(0);
    c->Draw();
}

void det_canvas_selected(int col,char *title,int cols_max=3)
{
    char *detectors[]={"4X","4U","6V"};
    
    TCanvas *cone = gROOT->FindObject("cone");
    if( cone==NULL )
    {
        cone = new TCanvas("cone","cone");
        cone->Divide(cols_max,sizeof(detectors)/sizeof(*detectors));
    }

    for( int i=0; i<sizeof(detectors)/sizeof(*detectors); i++ )
    {
        cone->cd(col+i*cols_max);
        TH1F *h = draw_detector(detectors[i],card_channel);
        h->SetTitle(title);
    }
    
    cone->cd(0);
    cone->Draw();
}

void compare_tests(char *session1,char *dir1,char *session2,char *dir2=NULL,char *session3="",char *dir3=NULL)
{
    int number_of_tests=1;
    if( dir2!=NULL )
        number_of_tests=2;
    if( dir3!=NULL )
        number_of_tests=3;

    draw(session1,dir1);
    det_canvas(1,dir1,number_of_tests);
    det_canvas_selected(1,dir1,number_of_tests);

    if( number_of_tests>=2 )
    {
        draw(session2,dir2);
        det_canvas(2,dir2,number_of_tests);
        det_canvas_selected(2,dir2,number_of_tests);
    }
    
    if( number_of_tests>=3 )
    {
        draw(session3,dir3);
        det_canvas(3,dir3,number_of_tests);
        det_canvas_selected(3,dir3,number_of_tests);
    }
}
