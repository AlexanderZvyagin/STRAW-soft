TH1 *hist_extract(TPad *c)
{
    TIter nextobj(c->GetListOfPrimitives());
    for( TObject *ptr; NULL!=(ptr=nextobj.Next()); )
    {
        printf("==========>>> %s\n",ptr->GetName());
    }
}

void c_V(void)
{
    TIter nextobj(gDirectory->GetListOfKeys());
    TObject *ptr=nextobj.Next();
    if( ptr==NULL )
    {
        printf("No objects found!\n");
        return;
    }

    TCanvas *c=gDirectory->Get(ptr->GetName());
    c->Draw();
    
    TPad *p=c->cd(1);
    hist_extract(p);
}

void ud_cmp(char *name,int geoID)
{
    if( strlen(name)!=8 )
    {
        printf("Bad detector name!\n");
        return;
    }

    char buf[33];
    sprintf(buf,"T0_%s_geoID%d.root",name,geoID);

    for( int ud=0; ud<2; ud++ )
    {
        char cud[]="ud";
        buf[9]=cud[ud];
        TFile f(buf);
        if( !f.IsOpen() )
            return;
        
        c_V();
        
    }
}
