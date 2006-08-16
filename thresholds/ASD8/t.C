/* Read the ASD8 measurements to a ROOT file.
*/

float get_threshold(int srcID,int port,int channel);

TNtuple *t(void)
{
    TFile ff("asd8_calib.root","RECREATE","",9);
    TNtuple *tree=new TNtuple("asd8","ASD8 db","chip:chan:udc:enc:dhl:dml");

    for( int chip=0; chip<6666; chip++ )
    {
        char file_name[22];
        sprintf(file_name,"ASD8/ASD0%d.DAT",chip);
        ifstream f(file_name);
        if( !f )
            continue;
        printf("This is file %s\n",file_name);

        int n[8], ch[8];
        float Udc[8], ENC[8];
        for( int channel=0; channel<8; channel++ )
        {
            if( !(f>>n[channel]>>ch[channel]>>Udc[channel]>>ENC[channel]) )
            {
                printf("There is a problem with file %s!!\n",file_name);
                exit(1);
            }

            if( (channel<6 && n[channel]!=chip) || ch[channel]!=(channel+1) )
            {
                printf("Wrong file content in %s:  %d\n",file_name,n[channel]);
                exit(1);
            }
        }
        
        for( int channel=0; channel<8; channel++ )
            tree->Fill(chip,channel,Udc[channel],ENC[channel],n[6],n[7]);
    }
    
    ff.Write();
    ff.Close();
    
    return tree;
}

float get_threshold(int srcID,int port,int channel)
{
    char s[111];
    sprintf(s,"/afs/cern.ch/user/o/onl/catch/f1/straw/");
    ifstream f(file_name);
}
