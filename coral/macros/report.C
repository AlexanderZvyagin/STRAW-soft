/*! \file report.C
   \author Alexander Zvyagin
   \brief This file contains a collection of common functions used in the report creation.

    Here is an example of usage:
    \verbatim
    $ root data.root 'report.C("ST05X1db","","out.root")'
    \endverbatim

*/

#define report_loaded
#include <vector>

#ifndef report_C
#define report_C
#endif

/*! The set of data to be used in external macros.
*/
struct ForReportCreation
{
    ~ForReportCreation(void)
    {
        printf("End of the report creation.\n");
        Close();
    }

    ForReportCreation(void) :
        events_max(1e99),
        dir_data(NULL),
        dir_out(NULL),
        reports_counter(0)
    {}

    void Close(void)
    {
        if( RS.dir_out!=NULL && RS.dir_out->IsOpen() )
        {
            printf("Saving the file \"%s\"\n",dir_out->GetName());
            RS.dir_out->Write();
            RS.dir_out->Close();
        }
    }

    /*! Maximum number of events to analyze.
    */
    double events_max;

    /*! Top data directory.
    */
    TDirectory *dir_data;

    /*! A directory where all output histograms/ntuples/canvases
        are to be produced.
    */
    TFile *dir_out;

    /*! Used by dir_save(void) and dir_restore(void)
    */
    vector<TDirectory*> dirs_saved;

    /*! \brief Register an output root object.
    */
    void registration(TObject *object)
    {
        TH1 *h=dynamic_cast<TH1*>(object);
        if( h!=NULL )
        {
            if( dir_out!=NULL )
                h->SetDirectory(dir_out);
            return;
        }
    }

    /*! \brief Go to a directory where all output histograms should be put.
    */
    void cd_out(void)
    {
        if( dir_out!=NULL )
            dir_out->cd();
    }

    /*! \brief Go to the directory with data.
    */
    void cd_data(void)
    {
        if( dir_data!=NULL )
            dir_data->cd();
    }

    /*! \brief Remember the current ROOT directory.

        Save the current root directory for later
        restoration with the help of dir_restore().
    */
    void dir_save(void)
    {
        dirs_saved.push_back(gDirectory);
    }

    /*! \brief Restore the previously saved (by dir_save(void)) ROOT directory.
    */
    void dir_restore(void)
    {
        if( dirs_saved.empty() )
        {
            printf("dir_restore(): no saved directories.\n");
            return;
        }

        TDirectory *d=dirs_saved.back();
        dirs_saved.pop_back();
        d->cd();
    }
    
    void create_out(char *out_file)
    {
        if( *out_file!=0 )
            dir_out = new TFile(out_file,"UPDATE","",9);
    }

    /*! List of reports. */
    void (*reports[100])(char *det="",char *options="");

    /*! The reports counter. \sa reports */
    int reports_counter;

    /*! \brief Generate STDC COMPASS names

        The first call should be with the argument \b true, all later ones are without
        argument. Once the end of list is reached, the function returns NULL.

        A few examples: "ST03X1ua", "ST06U1db", "ST06V1db".
    */
    char *STDC_iterator(bool init=false)
    {
        static int abc, ud, station, plane, xy;
        static char STDC[9];

        if( init )
        {
            station = 1;
            plane   = 1;
            abc     = 0;
            ud      = 0;
            xy      = 0;
        }

        for( ; station<=6; station++ )
        {
            for( ; xy<4; xy++ )
            {
                for( ; plane<=2; plane++ )
                {
                    for( ; ud<2; ud++ )
                    {
                        while( abc<3 )
                        {
                            char *_abc="abc", *_ud="ud", *_xy="XUVY";
                            sprintf(STDC,"ST%2.2d%c%d%c%c",station,_xy[xy],plane,_ud[ud],_abc[abc]);
                            abc++;
                            return STDC;
                        }
                        abc=0;
                    }
                    ud=0;
                }
                plane=1;
            }
            xy=0;
        }

        return NULL;
    }
} RS;


/*! \brief Load macros for a default report.
*/
bool load_default_reports(void)
{
    //gROOT->LoadMacro("dummy_report.C");
    //gROOT->LoadMacro("eff.C");
    //gROOT->LoadMacro("V.C");
}

bool init=load_default_reports();

/*! \brief Create a detector report.
    INPUT: the root file must be opend with ST??????_CORAL ntuple(s).
    With det="" argument report for all straw detectorss will be created.
*/
void report(char *det="",char *options="",char *out_file="")
{
    printf("Maximum number of events to analyze: %g\n",RS.events_max);

    // Remember the data location.
    RS.dir_data = gDirectory;

    // Create file with output ROOT objects.
    RS.create_out(out_file);

    printf("There are %d reports to produce.\n",RS.reports_counter);

    // Iterate over all reports which we want to create.
    for( int i=0; i<RS.reports_counter; i++ )
    {
        // Think about badly written macros.
        // In principle all of them shoud use cd_data() and cd_out() functions.
        RS.cd_data();
        
        if( *det!=0 )
            (*RS.reports[i])(det,options);
        else
            for( char *detector=RS.STDC_iterator(true);
                 detector!=NULL; detector=RS.STDC_iterator() )
                (*RS.reports[i])(detector,options);
    }
}

    
// void report2(char *det="",char *options="r ")
// {
//     gROOT->LoadMacro("eff.C");
//     
//     bool albert_residual=strstr(options,"r ")!=NULL;
//     
//     if( albert_residual )
//     {
//         gROOT->LoadMacro("Albert-residuals.C");
//         UMaps_forSasha(gDirectory->GetName());
//         CompStrawXray3D_forSasha();
//     }
//     
//     if( det==NULL || *det==0 )
//         for( det=STDC_iterator(true); det!=NULL; det=STDC_iterator() )
//             report_detector(det);
//     else
//         report_detector(det);
// }
