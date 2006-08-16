/*! \file dummy_report.C
    \brief Example of a minimal macro report.
    
    You must load 'report.C' macro before loading the 'dummy_report.C':
    \code
    .L report.C
    .L dummy_report.C
    \endcode
    
    or
    
    \code
    gROOT->LoadMacro("report.C");
    gROOT->LoadMacro("dummy_report.C");
    \endcode
*/

/*! \brief This function is called to generate a report for a detector.
*/
void dummy_call(char *detector)
{
    // All output objects shoud be created in the 'output' directory.
    RS.cd_out();

    char name[222];
    sprintf(name,"%s_dummy",detector);
    TH1F *h=new TH1F(name,"The dummy title.",1,0,1);
    
    // Register the object.
    RS.registration(h);

    // Go to the data directory.    
    RS.cd_data();
    
    // Do something here!
}

/*! \brief The main report call.
*/
void dummy_report(char *detector="",char *options="")
{
    // Inform whe world about us.
    printf("This is 'dummmy' report for %s.\n",detector);
    
    // Try to preserve the directory structure.
    RS.dir_save();

    // The main call is here.
    dummy_call(detector);

    // Restore the directory.    
    RS.dir_restore();
}

/*! Initialization function.
*/
bool dummy_init(void)
{
    RS.reports[RS.reports_counter++]=dummy_report;
    printf("Registering 'dummy' report.\n");
}

/*! Initilize the report during its loading!
*/
bool init=dummy_init();
