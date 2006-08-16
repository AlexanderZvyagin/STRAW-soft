/*! \file split.C

    \code
    $ root ntuples.root report.C split.C
    \endcode
*/

/*! \brief Start the code
*/
void split_start(char *detector="",char *options="")
{
    // Get the ntuple
    char name[222];
    sprintf(name,"%s_CORAL",detector);
    TTree *nt=gDirectory->Get(name);
    if( nt==NULL )
        return;

    // Now we need to create the ntuple variables list.
    *name=0;
    TIterator *iter=nt->GetListOfBranches()->MakeIterator();
    TBranch *b;
    float vars[55];
    int vars_counter=0;
    while( (b=(TBranch*)iter->Next())!=NULL )
    {
        if( vars_counter>=sizeof(vars)/sizeof(*vars) )
        {
            printf("Wow!!! Too may vaiables!\n");
            return;
        }
        nt->SetBranchAddress(b->GetName(),&vars[vars_counter++]);
        sprintf(name+strlen(name),"%s:",b->GetName());
    }
    name[strlen(name)-1]=0;

    // Inform whe world about us.
    printf("Ntuple found for the detector %s\n",detector);
    
    // Try to preserve the directory structure.
    RS.dir_save();


    // Create detetor-specific file.
    char fname[22];
    sprintf(fname,"%s.root",detector);
    TFile f(fname,"RECREATE","",9);
    TNtuple *nt_new=new TNtuple(nt->GetName(),nt->GetTitle(),name);

    for( int i=0; i<nt->GetEntries(); i++ )
    {
        if( nt->GetEntry(i)<=0 )
        {
            printf("Can not get entry??\n");
            break;
        }

        nt_new->Fill(vars);
    }

    // Close the file.
    f.Write();
    f.Close();
    
    // Restore the directory.    
    RS.dir_restore();
}

/*! Initialization function.
*/
bool split_init(void)
{
    RS.reports[RS.reports_counter++]=split_start;
    printf("Registering 'split' code.\n");
}

/*! Initilize the report during its loading!
*/
bool init=split_init();

/*! \brief Default call from ROOT.
*/
void split(void)
{
    report();
}
