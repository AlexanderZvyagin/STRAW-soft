#include "TFile.h"
#include "TDirectory.h"
#include "TNtuple.h"
#include "TVector3.h"
#include "TRotation.h"
#include "TMatrix.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#undef Check
#include "CoralUser.h"
#include "CsEvent.h"
#include "CsInit.h"
#include "CsGeom.h"

#include "CoralUser.h"

#include "Detectors/DriftDetector.h"
#include "CoralDet.h"

using namespace CS;

vector<CoralDet::Hit> hits;
CoralDets *coral_dets=NULL;
TH1F *h_ST_hits=NULL, *h_ST_hits6=NULL, *h_ST_hits10=NULL;

namespace Albert
{
  TDirectory *dPrivate;

  unsigned int ndet;  // Total number of detectors.

  void CoralUserInit();
  void CoralUserEvent();

  map<string,TH3F*> Udist3;  // Detector name -> histogram
}

////////////////////////////////////////////////////////////////////////////////

bool CoralUserSetup(int argc, char* argv[])
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CoralUserInit()
{
    CsHistograms::SetCurrentPath("/");
    h_ST_hits   = new TH1F("SThits","Straw hits",50,0,50);
    h_ST_hits6  = new TH1F("SThits6mm","Straw hits in 6mm straws",50,0,50);
    h_ST_hits10 = new TH1F("SThits10mm","Straw hits in 10mm straws",50,0,50);


    coral_dets = new CoralDets(*CsInit::Instance()->getDetectorTable());

    Albert::CoralUserInit();
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CoralUserEnd(void)
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CoralUserEvent()
{
    coral_dets->Clear();

    // The loop on all tracks: find all tracks which pass the detectors
    for( list<CsTrack*>::const_iterator t=CsEvent::Instance()->getTracks().begin();
         t!=CsEvent::Instance()->getTracks().end(); t++,coral_dets->tracks_counter++ )
    {
        set<string> hits6, hits10;
        
        // Find all channels associated with the track
        for( std::map<std::string,CsDet*>::const_iterator d=CsDet::GetAllDetectors().begin();
             d!=CsDet::GetAllDetectors().end(); d++ )
        {
            if( coral_dets->MakeAssociation(**t,*d->second,0) )
                if( d->first.substr(0,2)=="ST" )
                    if( d->first[7]=='b' )
                        hits6.insert(d->first);
                    else
                        hits10.insert(d->first);
        }

        h_ST_hits->Fill(hits6.size()+hits10.size());
        h_ST_hits6->Fill(hits6.size());
        h_ST_hits10->Fill(hits10.size());
    }
    
    coral_dets->DoubleHitsAnalyse();
    coral_dets->Fill();

    Albert::CoralUserEvent();

    return true;
}

////////////////////////////////////////////////////////////////////////////////

namespace Albert
{
void CoralUserInit() {

  TFile *fs = gFile;
  fs->cd();
  
  dPrivate = new TDirectory("dPrivate","My Histos");
  dPrivate-> cd();

  list<CsDetector*> dtt= CsGeom::Instance()->getDetectors(); 
  ndet = dtt.size();

  for (list<CsDetector*>::const_iterator id=dtt.begin(); id!=dtt.end(); id++){
    const string& det_name = (*id)->GetTBName();
    if ((det_name[0] != 'S' || det_name[1] != 'T')
	&& (det_name[0] != 'D' || det_name[1] != 'W')
	&& (det_name[0] != 'D' || det_name[1] != 'C'))
      // we're not interested in this detector.
      continue;
    string isto_name = "Udist_" + det_name;
    string isto_title = "Profile of " + det_name;
    unsigned int nbins = (*id)->getNWir();
    double dimension = (nbins - 1) * (*id)->getWirP();
    double center = (*id)-> getWirD() + dimension/2; 
    float minx = center - dimension/2 - (*id)->getWirP()/2;
    float maxx = center + dimension/2 + (*id)->getWirP()/2;
    cout << isto_name << " " << isto_title << endl;
    Udist3[det_name] = new TH3F( isto_name.c_str(), isto_title.c_str(), nbins, minx, maxx, 12, -1800, 1800, 200, -2, 2 );
  }

  fs->cd();
}

void CoralUserEvent() {

  const list<CsVertex*> &vrts = CsEvent::Instance()->getVertices();
  const list<CsTrack* > &Trks = CsEvent::Instance()->getTracks();
  const vector<CsParticle* > &parts = CsEvent::Instance()->getParticles();
  

  ///////////////// LOOP OVER PARTICLES //////////////// 
  
  for( vector<CsParticle*>::const_iterator ip=parts.begin();
       ip!=parts.end(); ip++ ) {
    const CsTrack *trk = (*ip)->getTrack();

    if( trk == 0 ) {
      //cout<<"particle w/o track!!!"<<endl;
      continue;
    }

    CsVertex* fvrt = trk->getFirstVertex();

    if( fvrt==0 || !fvrt->isPrimary() ) {
      //cout << (!fvrt ? "track w/o vertex\n" : "No primary vertex.\n");
      continue;  // this particle has NO PRIMARY VERTEX
    }

    list<CsCluster*> cl = trk->getClusters();
    vector<CsHelix> v = trk->getHelices();

    // less than 21 hits or last hit in front of SM1?
    if ( cl.size() <= 20 || v.back().getZ() <= 4500 )
      continue;
    
    // evaluate the residuals of a track vs cluster and fill into histograms

    for ( list<CsCluster*>::const_iterator il = cl.begin();
	  il != cl.end(); il++) {
      const list<CsDetector*> &dets = (*il)->getDetsList();
      if ( dets.size() <= 0 ) { cout << "dets <= 0 !!!    How is this possible?" << endl; continue; }

      unsigned int NumHelix = v.size();
      if ( NumHelix < 2 ) { cout << "Helix Vector too small !!!    Size = " << NumHelix << endl; continue; }
      if ( NumHelix >  ndet+2 ) { cout << "Helix Vector too large !!!    Size = " << NumHelix << endl; continue; }

      double Zcl           = (*il)->getW();
      const string &nameCl = dets.front()->GetTBName();
      if (Udist3.find(nameCl) == Udist3.end())
	// We're not interested in this detector.
	continue;

      for ( unsigned int ii =0; ii < NumHelix; ii++ ) {
	if (Zcl != v[ii].getZ()) {
	  continue;
	}

	double X        = v[ii].getX();
	double Y        = v[ii].getY();
	const HepMatrix& iR = dets.front()->getRotWRSInv();
	double residual = (*il)->getU() - iR(1,1) * X - iR(1,2) * Y;
	// Udist3[nameCl] is guaranteed to be != NULL
	Udist3[nameCl]->Fill(iR(1,1)*X + iR(1,2)*Y, iR(2,1)*X + iR(2,2)*Y, residual);
	//if( nameCl.substr(0,2)=="ST" )
	printf("fill: %s %g\n",nameCl.c_str(),(*il)->getU());
	break;
      }
    }
  }
}
}
