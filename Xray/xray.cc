#include <exception>
#include <cstdio>
#include <fstream>
#include <popt.h>
#include <dirent.h>

#include "TROOT.h"
#include "TSQLServer.h"
#include "TMinuit.h"

#include "Detector.h"
#include "StrawTubes.h"
#include "RTRelationGrid.h"

#include "ObjectXML.h"

using namespace std;
using namespace CS;

TROOT root("","");

TSQLServer *db=TSQLServer::Connect("mysql://na58pc052.cern.ch","anonymous","");

void fit_grid(StrawTubes &st,double &pitch,double &angle,double &x,double &y);

////////////////////////////////////////////////////////////////////////////////

void write_calib(const StrawTubes &st,const string &calib_dir)
{
    ofstream f((string(calib_dir)+'/'+st.GetName()+".xml").c_str());
    ObjectXML *xml = st.CreateObjectXML();
    xml->Print(f);
    delete xml;
}

////////////////////////////////////////////////////////////////////////////////

void xray_store(TSQLServer *db, const std::string &det, int ch, int spacer, float w)
{
    char buf[555];

    if( NULL==db->Query("USE STDC;") )
        throw "Error in DB communication.";

    sprintf(buf,"CREATE TABLE IF NOT EXISTS `xray` ("
                "`detector`     varchar(100) NOT NULL default '',"
                "`channel`      int,    "
                "`spacer`       int,    "
                "`w`            float,  "
                " UNIQUE KEY `indx` (`detector`,`channel`,`spacer`)) TYPE=MyISAM;");
    if( !db->Query(buf) )
    {
        printf("**** FAILED: %s\n",buf);
        throw "Error in communication with the DB.";
    }

    sprintf(buf,"INSERT INTO STDC.xray (detector,channel,spacer,w) VALUES('%s',%d,%d,%g);",
            det.c_str(),ch,spacer,w);
    if( !db->Query(buf) )
    {
        printf("**** FAILED: %s\n",buf);
        throw "Can not write to DB.";
    }
}

////////////////////////////////////////////////////////////////////////////////

RTRelationGrid *load_RT(const string &path)
{
    ifstream f(path.c_str());
    if( !f.is_open() )
        throw "load_RT(): Can not load RT file.";
    
    float T0;
    if( !(f>>T0) )
        throw "load_RT(): can not get T0";

    string rtgrid;
    getline(f,rtgrid);
    getline(f,rtgrid);
    if( rtgrid!="RTGrid" )
        throw "load_RT(): bad RT-grid";

    char buf[2222]="RT-Grid";
    
    float t,r,e;
    while( f>>t>>r>>e )
    {
        sprintf(buf+strlen(buf)," %g:%g:%g",r,t,e);
    }
    
    RTRelationGrid *rt = new RTRelationGrid(buf);
    rt->SetT0(T0);

    return rt;
}

RTRelationGrid *find_RT(const string &det,const string &db_path)
{
    struct dirent **namelist;
    int n = scandir(db_path.c_str(),&namelist,0,0);
    RTRelationGrid *rt=NULL;

    while( n-- > 0 )
    {
        const char *name=namelist[n]->d_name;  // file name
        if( NULL!=strstr(name,det.c_str()) && NULL==strstr(name,"geom") )
        {
            if( rt!=NULL )
                printf("extra RT for %s\n",det.c_str());
            else
                rt = load_RT(db_path+'/'+name);
        }
        free(namelist[n]);
    }
    free(namelist);

    return rt;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc,const char *argv[])
{
    try
    {
        const char
            *calib_dir  = NULL,
            *xray_dir   = NULL,
            *cdb        = NULL,
            *rt_str[2]  = {NULL,NULL};   // RT: 6mm, 10mm
        int rotate=1;
        
        struct poptOption options[] =
        {
            { "RT6mm",     '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &rt_str[0], 0,
                                          "RT for 6mm straws",
                                          "NAME" },
            { "RT10mm",    '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &rt_str[1], 0,
                                          "RT for 6mm straws",
                                          "NAME" },
            { "cdb",    '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &cdb, 0,
                                          "cdb path. If --RTxxx option is applied, then we take from the cdb only T0.",
                                          "NAME" },
            { "xray",       '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &xray_dir, 0,
                                          "X-ray directory",
                                          "NAME" },
            { "rotate",    '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,  &rotate, 0,
                                          "Rotate Xray table? 0-no, 1-yes",
                                          "0/1" },
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        poptContext poptcont=poptGetContext(NULL,argc,argv,options,0);
        poptSetOtherOptionHelp(poptcont,
            "[options...] <calib_dir>\n"
            "  Author: Alexander Zvyagin <Alexander.Zviagine@cern.ch>\n"
        );

        poptGetNextOpt(poptcont);

        calib_dir   = poptGetArg(poptcont);
        if( calib_dir==NULL || poptPeekArg(poptcont)!=NULL )
        {
            poptPrintHelp(poptcont,stdout,0);
            return 1;
        }

        Detectors dets_xray;

        const char *xray[]=
        {
            "ST3U1d","ST3U1u", "ST3V1d","ST3V1u", "ST3X1d","ST3X1u",
            "ST3X2d","ST3X2u", "ST3Y1d","ST3Y1u", "ST3Y2d","ST3Y2u",
            "ST4V1d","ST4V1u", "ST4X1d","ST4X1u", "ST4Y1d","ST4Y1u",
            "ST5U1d","ST5U1u", "ST5X1d","ST5X1u", "ST5Y1d","ST5Y1u",
            "ST6V1d","ST6V1u", "ST6X1d","ST6X1u", "ST6Y1d","ST6Y1u",
            NULL
        };

        for( const char **name=xray; *name!=NULL; name++ )
        {
            // First create and write an ideal STDC.
            for( char abc='a'; abc<='c'; abc++ )
            {
                string s=*name;
                s.insert(2,1,'0');
                s += abc;

                StrawTubes *st = new StrawTubes(s);
                dets_xray.push_back(st);

                st -> SetResolution(0.02);

                // Set read-out position and channel signal speed.
                for( vector<DriftDetectorChannel>::iterator ch=st->GetChannels().begin();
                     ch!=st->GetChannels().end(); ch++ )
                {
                    const float signal_speed=28;
                    const float read_out=20;

                    ch -> SetReadoutPosY(ch->GetChannelPos()*read_out);
                    ch -> SetSignalSpeed(signal_speed);
                }

                const char *rt_string=rt_str[st->GetName()[7]!='b'];
                if( rt_string!=NULL )
                {
                    RTRelationGrid rt(rt_string);
                    st->SetRT(&rt);
                }

                if( cdb!=NULL && *cdb!=0 )
                {
                    RTRelationGrid *rt=find_RT(st->GetName(),cdb);
                    if( rt==NULL )
                        throw "RT not found!";
                    if( st->GetRT()==NULL )
                        st->SetRT(rt);
                    else
                        st->GetRT()->SetT0(rt->GetT0());
                    delete rt;
                }

                write_calib(*st,calib_dir);
            }

            const unsigned int nspacers =   6;
            static double
                spacer[nspacers],
                w[414][nspacers];
           
            if( xray_dir!=NULL )
            {
                ifstream f((string(xray_dir)+'/'+*name).c_str());
                if( !f.is_open() )
                {
                    printf("file %s\n",*name);
                    throw "Can not open X-ray file.";
                }

                string name2;
                f>>name2;
                if( name2!=*name )
                    throw "xray file is bad!";

                string y_coord, mm;

                if( !(f >> y_coord >> mm >> spacer[0] >> spacer[1] >> spacer[2] >>
                                            spacer[3] >> spacer[4] >> spacer[5]) )
                    throw "Can not read y-coord!";



                if( y_coord!="y-koord" || mm!="[mm]" )
                {
                    printf("%s %s\n",y_coord.c_str(),mm.c_str());
                    throw "Bad y-ccord format";
                }

                // Align coords...
                double avr=0;
                for( unsigned int i=0; i<nspacers; i++ )
                {
                    spacer[i]/=10;          // convert from [mm] to [cm]
                    avr += spacer[i];
                }
                avr /= nspacers;
                for( unsigned int i=0; i<nspacers; i++ )
                    spacer[i] -= avr;

                unsigned int nwires = (*name)[3]=='Y' ? 320 : 414;

                for( unsigned int i=0; i<nwires; i++ )
                {
                    unsigned int ch;

                    if( !(f>>ch>>w[i][0]>>w[i][1]>>w[i][2]>>w[i][3]>>w[i][4]>>w[i][5]) )
                    {
                        printf("%s ch=%d\n",*name,i+1);
                        throw "Can not read X-ray results!";
                    }

                    if( ch!=i+1 )
                        throw "Bad channel number!";

                    for( int j=0; j<6; j++ )
                        w[i][j]/=10000;  // from [microns] to [cm]
                }

                // We have read the X-ray results!

                int k=0;  // counter for w[k][] container

                for( char abc='a'; abc<='c'; abc++ )
                {
                    string s=*name;
                    s.insert(2,1,'0');
                    s += abc;

                    //StrawTubes *st = new StrawTubes(s); ????
                    //st -> SetResolution(0.02);
                    //dets_xray.push_back(st);
                    
                    Detector &det = dets_xray.Find(s);
                    StrawTubes *st=dynamic_cast<StrawTubes*>(&det);

                    if( st==NULL )
                        throw "Internal error.";

                    printf("%s\n",st->GetName().c_str());

                    int nchan=0;

                    if( st->GetName()[7]=='b' )
                        nchan = StrawTubes::Channels6mmStrawsXY[st->GetName()[4]=='Y'];
                    else
                        nchan = StrawTubes::Channels10mmStrawsXY[st->GetName()[4]=='Y'];

                    unsigned int mod_channels=0; // counter of modified channels
                    float angle_avr=0;

                    TVector3 pos_old(0,0,0), pos_new(0,0,0);

                    printf("nchan=%d\n",nchan);

                    for( int l=0; l<nchan; l++,k++ )
                    {
                        // Store X-ray results in the DB
                        if(0)
                        {
                            int spacers[6]={-3,-2,-1,1,2,3};
                            for( int sp=0; sp<6; sp++ )
                                xray_store(db,st->GetName(),l,spacers[sp],w[k][sp]);
                        }


                        DriftDetectorChannel *ch = st->FindChannel((short)l), *ch_u=NULL, *ch_d=NULL;

                        if( ch!=NULL && l>0 )
                            if( fabs(w[k][3]-w[k-1][3]-ch->GetPitch())>0.02 )
                                printf("Bad pitch size: %s l=%d k=%d   |%g-%g|=%g   ==>  d=%g   diff=%g\n",
                                        st->GetName().c_str(),l,k,w[k-1][3],w[k][3],fabs(w[k-1][3]-w[k][3]),
                                        ch->GetPitch(),fabs(w[k][3]-w[k-1][3]-ch->GetPitch()));

                        if( ch!=NULL )
                        {
                            if( ch->GetSpacers().size()<2 )
                                throw "Too little spacers!";

                            pos_old += ch->GetShapeDRS().GetPoint();

                            ch->GetSpacers().clear();
                            for( unsigned int m=0; m<nspacers; m++ )
                                ch->GetSpacers().push_back( TVector3(w[k][m],spacer[m],0) );

                            pos_new += ch->GetShapeDRS().GetPoint();

                            mod_channels += 1;
                        }
                        else
                        {
                            ch_d = st->FindChannel((short)l,-1);
                            ch_u = st->FindChannel((short)l, 1);
                            if( ch_d==NULL || ch_u==NULL )
                                throw "Can not find channel!";

                            if( ch_d->GetSpacers().size()<2 )
                                throw "Too little spacers!";

                            if( ch_u->GetSpacers().size()<2 )
                                throw "Too little spacers!";

                            pos_old += ch_d->GetShapeDRS().GetPoint()+ch_u->GetShapeDRS().GetPoint();

                            double y_d=ch_d->GetSpacers().back().Y(), y_u=ch_u->GetSpacers().front().Y();

                            ch_d->GetSpacers().clear();
                            for( unsigned int m=0; m<3; m++ )
                                ch_d->GetSpacers().push_back( TVector3(w[k][m],spacer[m],0) );
                            ch_d->GetSpacers().push_back( TVector3(w[k][2],y_d,0) );


                            ch_u->GetSpacers().clear();
                            ch_u->GetSpacers().push_back(TVector3(w[k][3],y_u,0));
                            for( unsigned int m=3; m<6; m++ )
                                ch_u->GetSpacers().push_back( TVector3(w[k][m],spacer[m],0) );

                            mod_channels += 2;

                            pos_new += ch_d->GetShapeDRS().GetPoint()+ch_u->GetShapeDRS().GetPoint();
                        }
                    
                        // Calculate the sum of all wire angles.
                        angle_avr += (w[k][5]-w[k][0])/(spacer[5]-spacer[0]);
                    }

                    if( mod_channels!=st->GetChannels().size() )
                        throw "Wrong number of channels!";

                    angle_avr /= nchan;
                    TRotation rotate_table;
                    rotate_table.RotateZ(-angle_avr);
                    printf("Average wire angle is %g\n",angle_avr);

                    pos_old *= 1./mod_channels;
                    pos_new *= 1./mod_channels;
                    //printf("pos_old: (%g,%g,%g)   pos_new: (%g,%g,%g)\n",
                    //        pos_old.X(),pos_old.Y(),pos_old.Z(),
                    //        pos_new.X(),pos_new.Y(),pos_new.Z());

                    double fit_angle=0, fit_pitch=st->CalculatePitchAvr(), fit_x=pos_new.X(), fit_y=pos_new.Y();
                    fit_grid(*st,fit_pitch,fit_angle,fit_x,fit_y);

                    // Now we can calculate the new pitch size
                    double new_pitch = (st->GetChannels().back ().GetShapeDRS().GetPoint().X() -
                                        st->GetChannels().front().GetShapeDRS().GetPoint().X() )
                                       / (nchan-1);
                    printf("old pitch: %g    new pitch: %g  diff=%g\n",
                            st->GetChannels().front().GetPitch(),new_pitch,new_pitch-st->GetChannels().front().GetPitch());
                    
                    printf("simple-fit=diff:  pitch=%g-%g=%g  angle=%g-%g=%g   x=%g-%g=%g  y=%g-%g=%g\n",
                            new_pitch,fit_pitch,new_pitch-fit_pitch,
                            angle_avr,fit_angle,angle_avr-fit_angle,
                            pos_new.X(),fit_x,fit_x-pos_new.X(),
                            pos_new.Y(),fit_y,fit_y-pos_new.Y());

                    // fix the shift of x-ray results
                    
                    st->SetPitchAvr(fit_pitch);

                    for( vector<DriftDetectorChannel>::iterator it=st->GetChannels().begin();
                         it!=st->GetChannels().end(); it++ )
                    {
                        it->SetPitch(fit_pitch);

                        for( vector<TVector3>::iterator sp=it->GetSpacers().begin();
                             sp!=it->GetSpacers().end(); sp++ )
                        {
                                *sp = *sp - TVector3(fit_x,fit_y,0);
                                if( rotate )
                                    *sp = rotate_table * (*sp);
                        }
                    }
                    
                    write_calib(*st,calib_dir);
                }
            }
        }
        
    }
    catch( const std::exception &e )
    {
        printf("%s\n",e.what());
    }
    catch( const char *s )
    {
        printf("%s\n",s);
    }
    catch(...)
    {
        printf("Unknonw exception!\n");
    }

    return 0;
}

CS::StrawTubes *straw=NULL;

void f(Int_t &np, Double_t *g, Double_t &ff, Double_t *x, Int_t flag)
{
    if( straw==NULL )
        throw "There is no any STRAW detector for fit!";

    double pitch=x[0], angle=x[1];
    TVector3 o(x[2],x[3],0);  // offset

    TRotation r;
    r.RotateZ(-angle);    

    double start = -pitch*(straw->GetChannelsN()-1)/2.;
    int n=0;
    ff=0;

    for( vector<DriftDetectorChannel>::iterator it=straw->GetChannels().begin();
         it!=straw->GetChannels().end(); it++ )
    {
        for( vector<TVector3>::iterator sp=it->GetSpacers().begin();
             sp!=it->GetSpacers().end(); sp++ )
        {
                TVector3 v = *sp - o;
                v = r * v;

                ff +=  pow(start + it->GetChannel()*pitch - v.X(),2);
                n ++;
        }
    }

    ff /= n;           
}

void fit_grid(StrawTubes &st,double &pitch,double &angle,double &x,double &y)
{
    TMinuit minuit(4);    
    Double_t arglist[10];
    Int_t ierflg;
    
    straw = &st;
    
    // -- Set the debug printout level by the fitting code.
    arglist[0]=0;
    minuit.mnexcm("SET PRINTOUT", arglist ,1,ierflg);
    
    //minuit.mnexcm("SET NOWARNINGS", arglist ,0,ierflg);

    // -- Declare variables for the fit.
    minuit.DefineParameter(0,"pitch",pitch,0.01,pitch*0.9,pitch*1.1);
    minuit.DefineParameter(1,"angle",angle,0.01,angle-0.1,angle+0.1);
    minuit.DefineParameter(2,"x",x,0.01,x-10,x+10);
    minuit.DefineParameter(3,"y",y,0.01,y-10,y+10);

    // -- Set the fit function.
    minuit.SetFCN(f);

    // -- Fit!
    minuit.mnexcm("MINIMIZE", arglist ,0,ierflg);

    // -- Fit!
    //minuit.mnexcm("MINOS", arglist ,0,ierflg);

    // -- Read fit results.

    double err;
    minuit.GetParameter(0,pitch,err);
    minuit.GetParameter(1,angle,err);
    minuit.GetParameter(2,x,err);
    minuit.GetParameter(3,y,err);
}
