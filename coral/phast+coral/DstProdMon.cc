//
// in case of problems with this code, 
// contact  Paolo Pagano <ppagano@mail.cern.ch>
//
#include "CsGeom.h"
#include "CsOpt.h"
#include "CoralUser.h"
#include "CsEvent.h"
#include <TFile.h>
#include <TH2.h>
#include "CsOutput.h"
#include <TDirectory.h>

namespace Sasha {
bool CoralUserSetup(int argc, char* argv[]);
bool CoralUserInit();
bool CoralUserEvent();
bool CoralUserEnd();
}

#ifndef USE_Oracle // without Oracle

bool CoralUserSetup(int argc, char* argv[]) { return true; }
bool CoralUserInit ()                       { return true; }
bool CoralUserEvent()                       { return true; }
bool CoralUserEnd  ()                       { return true; }

#else             // with Oracle

#include "CsOraStore.h"
TDirectory *dGeneral;
TH1F* hMonit[12];
TH1F* prof[500];
TH2F* hCluMap;

int NT,NTM,NPV,NSV;

list<CsDetector*> dtt;

// DST Production User's Coral routines (ORACLE DB)

const double M_Pi   = 0.139567;
const double M_P    = 0.93827231;
const double M_N    = 0.93956563;
const double M_K    = 0.493677;
const double M_mu   = 0.105658357;
const double M_D0_  = 1.8645;
const double M_K0_  = 0.497672;
const double M_nucl = (M_P+M_N)/2.;

using namespace std;

bool GetMyExternalVars(double& Q2, double& Y, int& nTracksWithMomentum)
{

  const list<CsTrack*> &Trks = CsEvent::Instance()->getTracks();
  list<CsTrack*>::const_iterator it;
  for( it=Trks.begin(); it!=Trks.end(); it++ ) 
    {
      CsTrack *trk = (*it);
      
      const vector<CsHelix> v = trk->getHelices();
      
      if( v.size()!=0 && v[0].getCop()!=0 ) nTracksWithMomentum++;   // number of tracks with momentum
      
    }

  const vector<CsParticle* > &parts = CsEvent::Instance()->getParticles();
  
  double P_i = 0;
  double P_s = 0;
  double dxdz_i = 0;
  double dxdz_s = 0;
  double dydz_i = 0;
  double dydz_s = 0;
  bool isFound = false;
  for(unsigned i = 0; i < parts.size(); i++)
    {
      if(parts[i]->getName() != "mu") // not scattered muon
	continue;
      const CsTrack* tr = parts[i]->getTrack();
      if(tr)
	{
	  const list<CsVertex*> &v = tr->getVertices();
	  list<CsVertex*>::const_iterator it;
	  for(it = v.begin(); it != v.end(); it++)
	    {
	      if((*it)->isPrimary()) // it is primary vertex
		{
		  isFound = true;
		  const vector<CsHelix>& hlx = tr->getHelices();
		  for(unsigned j = 0; j < hlx.size(); j++)
		    {
		      if(hlx[j].getCop() != 0)
			{
			  P_s = 1./hlx[j].getCop();
			  dxdz_s = hlx[j].getDXDZ();
			  dydz_s = hlx[j].getDYDZ();
			  break;
			}
		    }
		  if(P_s <= 0.) return false;
		  // looking for beam track:
		  const list<CsTrack* > &trks = (*it)->getTracks();
		  list<CsTrack*>::const_iterator itr;
		  for( itr = trks.begin(); itr != trks.end(); itr++ )
		    {
		      if((*itr)->IsBeamTrack())
			{
			  const vector<CsHelix>& hlx = (*itr)->getHelices();
			  for(unsigned j = 0; j < hlx.size(); j++)
			    {
			      if(hlx[j].getCop() != 0)
				{
				  P_i = 1./hlx[j].getCop();
				  dxdz_i = hlx[j].getDXDZ();
				  dydz_i = hlx[j].getDYDZ();
				  break;
				}
			    }
			  if(P_i <= 0.)
			    return false;
			}
		    }
		  break;
		}
	    }
	}
    }

  if(!isFound) return false;

  if(P_i <= P_s)
    return false;

  double E_i = sqrt(P_i*P_i+M_mu*M_mu);
  double E_s = sqrt(P_s*P_s+M_mu*M_mu);

  double pz_i = P_i*sqrt(1+dxdz_i*dxdz_i+dydz_i*dydz_i);
  double px_i = dxdz_i*pz_i;
  double py_i = dydz_i*pz_i;
  double pz_s = P_s*sqrt(1+dxdz_s*dxdz_s+dydz_s*dydz_s);
  double px_s = dxdz_s*pz_s;
  double py_s = dydz_s*pz_s;

  double dE  = E_i-E_s;
  double dPx = px_i-px_s;
  double dPy = py_i-py_s;
  double dPz = pz_i-pz_s;

  Q2 = -(dE*dE-dPx*dPx-dPy*dPy-dPz*dPz);

  if(Q2 < 0) 
    return false;

  Y = dE/E_i;

   return true;
}

bool CoralUserSetup(int argc, char* argv[])
{
  Sasha::CoralUserSetup(argc,argv);

  CsOraStore::setExtVarName(0,string("Q_2"));      // Q square
  CsOraStore::setExtVarName(1,string("Y_bjork"));  // Y Bjorken
  CsOraStore::setExtVarName(2,string("Ntr_with_P"));  // Number of tracks with momentum
  
  return true;
}

void DST_PreuploadFunc()
{

  double Q2 = 0.;
  double Y  = 0.;
  int nTrWithMom  = 0;

  if(!GetMyExternalVars(Q2,Y,nTrWithMom))
    Q2 = Y = -1.;

  CsOraStore::setExtVariable(0, Q2);
  CsOraStore::setExtVariable(1, Y);
  CsOraStore::setExtVariable(2, nTrWithMom);

}


void State_Histos(){

  NT=NTM=NPV=NSV=0;

  
  TFile *f = gFile;
  
  f->cd();

  dGeneral= new TDirectory("dGeneral","General");
  dGeneral-> cd();
    
  hMonit[0] = new TH1F("hMonit_0","tracks per event"            , 300,0,30000 );
  hMonit[1] = new TH1F("hMonit_1","tracks (with mom) per event" , 300,0,30000 );
  hMonit[2] = new TH1F("hMonit_2","primary vertices per event"  , 300,0,30000 );
  hMonit[3] = new TH1F("hMonit_3","secondary vertices per event", 300,0,30000 );
  
  hMonit[4] = new TH1F("hMonit_4","Number of tracks " , 100,0,100 );
  hMonit[5] = new TH1F("hMonit_5","Number of tracks before the target" , 100,0,100 );
  hMonit[6] = new TH1F("hMonit_6","Number of tracks before SM1" , 100,0,100 );
  hMonit[7] = new TH1F("hMonit_7","Number of tracks between M1 and M2" , 100,0,100 );
  hMonit[8] = new TH1F("hMonit_8","Number of tracks between M2 and Muon Wal" , 100,0,100 );
  hMonit[9] = new TH1F("hMonit_9","Number of tracks after Muon Wall" , 100,0,100 );
  hMonit[10] = new TH1F("hMonit_10","Total number of clusters per event " , 1000,0,4000 );
  hMonit[11] = new TH1F("hMonit_11","Total number of clusters per plane" , 100,0,100 );
  
  hCluMap = new TH2F("hCluMap","ID vs. clusters size" , 100, 0, 100, 1300, 1, 1300 );

  dtt= CsGeom::Instance()->getDetectors(); 
  list<CsDetector*>::iterator id;

  unsigned int nbins;
  float minx, maxx;
  string det_name;
  string isto_title;
  unsigned int isto_number=0;
  char* isto_name=new char[10];


  for (id=dtt.begin(); id!=dtt.end(); id++){
    det_name = (*id)-> GetTBName();
    if (det_name[4] == 'X' ||  det_name[4] == 'Y' ||
	det_name[4] == 'U' ||  det_name[4] == 'V'){

      sprintf(isto_name, "prof_%d", isto_number);
      isto_title = "Profile of " + det_name;
      double dimension, center;
      nbins = (*id)->getNWir();
      dimension = (nbins - 1) * (*id)->getWirP();
      center = (*id)-> getWirD() + dimension/2; 
      minx = center - dimension/2 - (*id)->getWirP()/2;
      maxx = center + dimension/2 + (*id)->getWirP()/2;
      cout << isto_name << " " << isto_title << endl;
      prof[isto_number] = new TH1F( isto_name,isto_title.c_str(), nbins, minx, maxx );
      isto_number++;
    }
  }

  f->cd();
}

bool CoralUserInit() {

  Sasha::CoralUserInit();

  //Monitoring stuff

  CsOpt *opt= CsOpt::Instance();
  string tag ="Monitoring";
  string key="Directory";
  string yes_or_no;
  if (opt->getOpt(tag, key, yes_or_no) && (yes_or_no == "ON")) State_Histos();


  // Volodia's stuff
  CsOraStore* store = CsOraStore::Instance();
  if(store)
    store->setPreuploadFunc(DST_PreuploadFunc); // the function for filling DST4 ext.vars
  else
    return false;
  return true;
}


// --------------------------------------------------
bool CoralUserEvent() {

    Sasha::CoralUserEvent();

  CsOpt *opt= CsOpt::Instance();
  string tag ="Monitoring";
  string key="Directory";
  string yes_or_no;

  if (!opt->getOpt(tag, key, yes_or_no) || (yes_or_no != "ON")) return true;

  unsigned int NEV = CsEvent::Instance()->getNumberOfEvents();
  
  const list<CsVertex*> &vrts = CsEvent::Instance()->getVertices();
  const list<CsTrack* > &Trks = CsEvent::Instance()->getTracks();
  
  
  ///////////////// LOOP OVER VERTICES ////////////////
  
  list<CsVertex*>::const_iterator iv;
  for( iv=vrts.begin(); iv!=vrts.end(); iv++ ) {
    CsVertex *vrt = (*iv);
    
    if( vrt->isPrimary() ) { NPV++; continue; }
    else                     NSV++;
  }   
  
    
  ///////////////// LOOP OVER TRACKS //////////////// 
  
  int NZ1(0),NZ2(0),NZ3(0),NZ4(0),NZ5(0);

  list<CsTrack*>::const_iterator it;
  for( it=Trks.begin(); it!=Trks.end(); it++ ) {
    CsTrack *trk = (*it);
    
    const vector<CsHelix> v = trk->getHelices();
    
    if( v.size()!=0 && v[0].getCop()!=0 ) NTM++;   // number of tracks with momentum
    
    ////////// number of tracks per zone ////////
    
    const list<CsZone*> zones = trk->getZones ();
    list<CsZone*>::const_iterator iz;
    for( iz=zones.begin(); iz!=zones.end(); iz++ ) {
      const string &name = (*iz)->getName();
      if( name == "before the target"        ) NZ1++;
      if( name == "before M1"                ) NZ2++;
      if( name == "between M1 and M2"        ) NZ3++;
      if( name == "between M2 and Muon Wall" ) NZ4++;
      if( name == "after Muon Wall"          ) NZ5++;
    }
        
  }
  
  
  /////////////// MONITORING //////////////////
  
  hMonit[4]->Fill( Trks.size() );
  hMonit[5]->Fill( NZ1 );
  hMonit[6]->Fill( NZ2 );
  hMonit[7]->Fill( NZ3 );
  hMonit[8]->Fill( NZ4 );
  hMonit[9]->Fill( NZ5 );
 
  const int NE = 100;
  NT += Trks.size();
  
  if( NEV%NE == 0 ) {
    hMonit[0]->Fill( NEV-0.1, NT /float(NE) );
    hMonit[1]->Fill( NEV-0.1, NTM/float(NE) );
    hMonit[2]->Fill( NEV-0.1, NPV/float(NE) );
    hMonit[3]->Fill( NEV-0.1, NSV/float(NE) );
    NT=NTM=NPV=NSV=0;
  }


  if( NEV%25 == 0 ) {
    unsigned int total_number_of_cluster_in_event=0;
    unsigned int total_number_of_cluster_per_plane=0;
    
    unsigned int isto_number=0;
    for( list<CsDetector*>::iterator k=dtt.begin();
	 k!=dtt.end(); k++ ) {
     
      list<CsCluster*> clus = (*k)->getMyClusters();
      list<CsCluster*>::iterator clu;
      string det_name = (*k)-> GetTBName();
      if (det_name[4] == 'X' ||  det_name[4] == 'Y' ||
	  det_name[4] == 'U' ||  det_name[4] == 'V'){
	
	for (clu=clus.begin(); clu!=clus.end(); clu++){
	  prof[isto_number]->Fill((*clu)->getU());
	}
	
	isto_number++;
      }
      total_number_of_cluster_in_event += clus.size();
      total_number_of_cluster_per_plane = clus.size();
      hMonit[11]->Fill( total_number_of_cluster_per_plane );
      hCluMap->Fill( total_number_of_cluster_per_plane, (*k)->GetID().GetNumber() );
    }
    hMonit[10]->Fill( total_number_of_cluster_in_event );
  }
 
  return true;  
  
  
}
  


// --------------------------------------------------
// This fuction is called bevore Coral end. Put here your
// final lines of code...
bool CoralUserEnd() {

  Sasha::CoralUserEnd();

  return true;  

}

#endif // USE_Oracle









