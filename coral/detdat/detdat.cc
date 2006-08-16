#include <cassert>
#include <exception>
#include <popt.h>
#include <sstream>
#include <map>

#include <sys/stat.h>
#include <sys/types.h>

#include "TMath.h"

//#include "DetDat.h"
//#include "DetDatLine.h"
#include "utils.h"
#include "DetDat.h"
#include "Detector.h"
#include "Exception2.h"

using namespace std;
using namespace CS;

void modify(const string &old_detdat,const string &new_det_dat,map<string,DetDatLine *> &ddls);

bool check(double a,double b,double eps=0.01)
{
    return fabs(a-b)<eps;
}

int main(int argc,const char *argv[])
{
    try
    {
        int year=0;

        struct poptOption options[] =
        {
            { "year",          '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &year,            0,
                                          "Year (2002,2003,2004,2006)", "YEAR" },
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        poptContext poptcont=poptGetContext(NULL,argc,argv,options,0);
        poptSetOtherOptionHelp(poptcont,
            "<options...> <detectors.dat>\n"
            "  Investigate detectors.dat file.\n"
            "  \n"
            "  Author: Alexander Zvyagin <Alexander.Zvyagin@cern.ch>\n"
        );

        if( argc<=1 )
        {
            poptPrintHelp(poptcont,stdout,0);
            return 1;
        }
        
        int rc;
        while( (rc=poptGetNextOpt(poptcont))>0 )
        {
            switch( rc )
            {
                default:
	                printf("bad argument %s: %s\n",
		                poptBadOption(poptcont, POPT_BADOPTION_NOALIAS),
		                poptStrerror(rc));
                    return 1;
            }
        }

        // The Y-chamber station number #:  ST0#Y***
        map<int,const TRotation *> rotations;
        
        TRotation r4a;
        r4a.RotateZ(TMath::Pi()/2);
        TRotation r4b(r4a);
        r4b.RotateY(TMath::Pi());

        switch( year )
        {
            case 2002:
                rotations[3] = rotations[4] = &r4a;
                break;

            case 2003:
            case 2004:
                rotations[3] = rotations[4] = &r4a;
                rotations[5] = rotations[6] = &r4b;
                break;

            case 2006:
                rotations[2] = rotations[3] = &r4a;
                rotations[5] = rotations[6] = &r4b;
                break;
                
            default:
                printf("Bad year %d.\n",year);
                poptPrintHelp(poptcont,stdout,0);
                return 2;
        }

        const char *detdat_path=NULL;

        while( NULL!=(detdat_path=poptGetArg(poptcont)) )
        {
            try
            {
                printf("%s\n",detdat_path);
                DetDat det_dat(detdat_path);
                map<string,DetDatLine *> ddls_changed;
                
                Detectors dets;
                // Now we read the geometry file
                dets.ReadDetectorsDAT(det_dat);

                // Find all straw detectors.
                for( size_t i=0; i<dets.size(); i++ )
                {
                    if( dets[i]->GetName().length()!=8 )
                        continue;
                
                    if( dets[i]->GetName().substr(0,2)!="ST" )
                        continue;
                    
                    if( dets[i]->GetName()[4]!='Y' )
                        continue;
                    
                    int station = atoi(dets[i]->GetName().substr(3,1).c_str());
                    const TRotation *rc = rotations[station];
                    if( rc==NULL )
                    {
                        printf("Unknown rotation for %s\n",dets[i]->GetName().c_str());
                        continue;
                    }
                    
                    const TRotation &r = dets[i]->GetPosition().GetRotation();
                    
                    if( !Compare(r,*rc) )
                    {
                        printf("DISCREPANCY for %s!\n",dets[i]->GetName().c_str());
                        if( !Compare(r,r4a) )
                        {
                            printf("Wow!!! I don't know what to do!\n");
                        }
                        else
                        {
                            DetDatLine *ddl = dets[i]->GetDetDatLine();
                            if( ddl==NULL )
                                throw "internal problem!";
    
                            ddl->rot_matrix_n       = det_dat.AddMatrixCG(r); // auto-detection of matrix number
                            ddl->rot_matrix         = r;                      // New rotation matrix
                            ddl->angle              = -ddl->angle;            // and change the angle!
                            ddl->pos_MRS[2]         = -ddl->pos_MRS[2];       // and change the sign of 'Y'

                            ddl->dead.rot_matrix_n  = ddl->rot_matrix_n;
                            ddl->dead.rot_matrix    = ddl->rot_matrix;
                            ddl->dead.pos_MRS[2]    = -ddl->dead.pos_MRS[2];  // and change the sign of 'Y'
                            
                            ddls_changed[ddl->TBname] = ddl;
                            //det_dat.Apply(*ddl);
                            //printf("I recommend to change the line!\n");
                        }
                    }
                }
                
                if( !ddls_changed.empty() )
                {
                    static int counter=1;
                    char dir[22],s[3333];
                    sprintf(dir,"%d",counter++);

                    printf("Writing a new file to directory \"%s\"\n",dir);

                    if( mkdir(dir,0666) )
                        throw Exception2("Can not create directory \"%s\"",dir);
                    sprintf(s,"ln -s %s %s/old.dat",detdat_path,dir);
                    system(s);
                    modify(detdat_path,string(dir)+"/new.dat",ddls_changed);
                }
            }
            catch(const char *e)
            {
                printf("%s\n",e);
            }
            catch(const std::exception &e)
            {
                printf("%s\n",e.what());
            }
            catch(...)
            {
                printf("Unknown exception.\n");
            }
        }
    }
    catch(const char *e)
    {
        printf("%s\n",e);
    }
    catch(const std::exception &e)
    {
        printf("%s\n",e.what());
    }
    catch(...)
    {
        printf("Unknown exception.\n");
    }

    return 1;
}

void modify(const string &old_detdat,const string &new_detdat,map<string,DetDatLine *> &ddls)
{
    DetDat detdat(old_detdat);
    
    ofstream fo(new_detdat.c_str());
    if( !fo.is_open() )
        throw Exception2("DetDat::Write(): Can not open file \"%s\"",new_detdat.c_str());

    bool rot_processing=false;

    for( vector<string>::const_iterator l=detdat.lines.begin(); l!=detdat.lines.end(); l++ )
    {
        bool copy=true;
        istringstream s(l->c_str());
        string what;
        s >> what;
        
        if( what=="rot" )
            rot_processing=true;
        else
        if( rot_processing )
        {
            rot_processing=false;
            
            for( map<string,DetDatLine *>::iterator it=ddls.begin(); it!=ddls.end(); it++ )
                if( detdat.rot_matrixes.end()==detdat.rot_matrixes.find(it->second->rot_matrix_n) )
                {
                    // Actualy the matrix is wrong!!! But we need to put something to fill ...
                    detdat.rot_matrixes[it->second->rot_matrix_n] = it->second->rot_matrix;
                    
                    //const TRotation &r = it->second->rot_matrix;
                    //
                    //char s[222];
                    //sprintf(s," rot     %2d    %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n",
                    //          it->second->rot_matrix_n,
                    //          r(2,0), r(2,1), r(2,2),
                    //          r(0,0), r(0,1), r(0,2),
                    //          r(1,0), r(1,1), r(1,2) );
                    //fo << s;
                    
                    fo << " rot     17     1.0000  0.0000  0.0000  0.0000  0.0000  1.0000  0.0000 -1.0000  0.0000\n";
                }
        }
        
        if( what=="det" )
        {
            DetDatLine d(*l,detdat.rot_matrixes);
            DetDatLine *l=ddls[d.TBname];
            if( l!=NULL )
            {
                copy = false;
                fo << l->DetString() << "\n";
            }
        }

        if( what=="dead" )
        {
            DetDatLine ddl;
            s >> ddl.dead.ID >> ddl.TBname >> ddl.dead.det >> ddl.dead.unit >> ddl.dead.shape
              >> ddl.dead.size[0] >> ddl.dead.size[1] >> ddl.dead.size[2]
              >> ddl.dead.pos_MRS[0] >> ddl.dead.pos_MRS[1] >> ddl.dead.pos_MRS[2]
              >> ddl.dead.rot_matrix_n;

            const DetDatLine *l = ddls[ddl.TBname];
            if( l!=NULL )
            {
                copy = false;
                fo << l->DeadString() << "\n";
            }
                
        }
        
        if( copy )
            fo << *l << "\n";
    }
}
