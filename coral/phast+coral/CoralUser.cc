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

list<CsDetector*> dtt;

TH3F* Udist3[100];
void CoralUserInit();
void CoralUserEvent();

char *naDet[] =     { "ST03X1ua", "ST03X1uc", "ST03X1ub", "ST03X1db", "ST03X1da", "ST03X1dc", "ST03Y1ua", "ST03Y1uc", "ST03Y1ub", "ST03Y1db",
                      "ST03Y1da", "ST03Y1dc", "ST03U1ua", "ST03U1uc", "ST03U1ub", "ST03U1db", "ST03U1da", "ST03U1dc", "ST03V1ua", "ST03V1uc",
                      "ST03V1ub", "ST03V1db", "ST03V1da", "ST03V1dc", "ST03Y2ua", "ST03Y2uc", "ST03Y2ub", "ST03Y2db", "ST03Y2da", "ST03Y2dc",
                      "ST03X2ua", "ST03X2uc", "ST03X2ub", "ST03X2db", "ST03X2da", "ST03X2dc", "ST04V1ua", "ST04V1uc", "ST04V1ub", "ST04V1db",
                      "ST04V1da", "ST04V1dc", "ST04Y1ua", "ST04Y1uc", "ST04Y1ub", "ST04Y1db", "ST04Y1da", "ST04Y1dc", "ST04X1ua", "ST04X1uc",
                      "ST04X1ub", "ST04X1db", "ST04X1da", "ST04X1dc", "ST05X1ua", "ST05X1uc", "ST05X1ub", "ST05X1db", "ST05X1da", "ST05X1dc",
                      "ST05Y1ua", "ST05Y1uc", "ST05Y1ub", "ST05Y1db", "ST05Y1da", "ST05Y1dc", "ST05U1ua", "ST05U1uc", "ST05U1ub", "ST05U1db",
                      "ST05U1da", "ST05U1dc", "ST06V1ua", "ST06V1uc", "ST06V1ub", "ST06V1db", "ST06V1da", "ST06V1dc", "ST06Y1ua", "ST06Y1uc",
                      "ST06Y1ub", "ST06Y1db", "ST06Y1da", "ST06Y1dc", "ST06X1ua", "ST06X1uc", "ST06X1ub", "ST06X1db", "ST06X1da", "ST06X1dc",

                      "DC01Y1__","DC01Y2__","DC01X1__","DC01X2__","DC01U1__","DC01U2__","DC01V1__","DC01V2__","DC02V2__","DC02V1__",
                      "DC02U2__","DC02U1__","DC02X2__","DC02X1__","DC02Y2__","DC02Y1__","DC03V2__","DC03V1__","DC03U2__","DC03U1__",
                      "DC03X2__","DC03X1__","DC03Y2__","DC03Y1__",

                      "DW01X1__","DW01X2__","DW01Y1__","DW01Y2__","DW02X1__","DW02X2__","DW02Y1__","DW02Y2__","DW03V1__","DW03V2__",
                      "DW03Y1__","DW03Y2__","DW04Y1__","DW04Y2__","DW04U1__","DW04U2__","DW05X1__","DW05X2__","DW05V1__","DW05V2__",
                      "DW06U1__","DW06U2__","DW06X1__","DW06X2__"
                    };


const int Ndet = sizeof(naDet)/sizeof(*naDet);
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

    //Albert::CoralUserInit();
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

    //Albert::CoralUserEvent();

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

  dtt= CsGeom::Instance()->getDetectors(); 
  list<CsDetector*>::iterator id;

  unsigned int nbins;
  float minx, maxx;
  string det_name;
  string isto_title;
  unsigned int isto_number = 0;
  char* isto_name = new char[20];

  for ( int k = 0; k < Ndet; k++ ) {
    TString dd = naDet[k];

    for (id=dtt.begin(); id!=dtt.end(); id++){
      det_name = (*id)->GetTBName();
      if (det_name == naDet[k] ) {
	sprintf(isto_name, "Udist_%s", naDet[k]);
	isto_title = "Profile of " + det_name;
	double dimension, center;
	nbins = (*id)->getNWir();
	dimension = (nbins - 1) * (*id)->getWirP();
	center = (*id)-> getWirD() + dimension/2; 
	minx = center - dimension/2 - (*id)->getWirP()/2;
	maxx = center + dimension/2 + (*id)->getWirP()/2;
	cout << isto_name << " " << isto_title << endl;
	Udist3[k] = new TH3F( isto_name,isto_title.c_str(), nbins, minx, maxx, 12, -1800, 1800, 200, -2, 2 );
      }
    }
  }

  fs->cd();
}

void CoralUserEvent() {

  const list<CsVertex*> &vrts = CsEvent::Instance()->getVertices();
  const list<CsTrack* > &Trks = CsEvent::Instance()->getTracks();
  const vector<CsParticle* > &parts = CsEvent::Instance()->getParticles();
  

  ///////////////// LOOP OVER PARTICLES //////////////// 
  
  CsVertex *vPr(0);    // pointer to primary vertex
  vector<CsParticle*>::const_iterator ip;
  for( ip=parts.begin(); ip!=parts.end(); ip++ ) {
    const CsTrack *trk = (*ip)->getTrack();

    if( trk == 0 ) {
//       cout<<"ERROR: particle is special but no track!!!"<<endl;
      continue;
    }

    vector<CsHelix> v = trk->getHelices();
    list<CsCluster*> cl = trk->getClusters();
    list<CsCluster*>::const_iterator il;

    float HelZ = v.front().getZ();
    float HelZba = v.back().getZ();
    CsVertex* fvrt = trk->getFirstVertex();
    float Chi2 = trk->getChi2();

    if( fvrt==0 || !fvrt->isPrimary() ) continue;  // this particle has NO PRIMARY VERTEX

    // evaluate the residuals of a track vs cluster and fill into histograms
    int ClusterSize = cl.size();

    if ( ClusterSize > 20 && HelZba > 4500 ) {    // more than 20 hits AND last hit after SM1
      for ( il = cl.begin(); il != cl.end(); il++) {
        const list<CsDetector*> &dets = (*il)->getDetsList();
	if ( dets.size() <= 0 ) { cout << "dets <= 0 !!!    How is this possible?" << endl; continue; }
	int NumHelix = v.size();
	if ( NumHelix < 2 ) { cout << "Helix Vector too small !!!    Size = " << NumHelix << endl; continue; }
	if ( NumHelix >  Ndet+2 ) { cout << "Helix Vector too large !!!    Size = " << NumHelix << endl; continue; }
	double Zcl           = (*il)->getW();
        const string &nameCl = dets.front()->GetTBName();
        HepMatrix iR         = dets.front()->getRotWRSInv();
        for ( int ii = 1; ii <= NumHelix - 2; ii++ ) {
          for ( int kk = 0; kk < Ndet; kk++ ) {
            if( nameCl == naDet[kk] && Zcl == v[ii].getZ() ) {
	      double X        = v[ii].getX();
	      double Y        = v[ii].getY();
	      double residual = (*il)->getU() - iR(1,1) * X - iR(1,2) * Y;
 	      Udist3[kk]->Fill(iR(1,1)*X + iR(1,2)*Y, iR(2,1)*X + iR(2,2)*Y, residual);
              //if( nameCl.substr(0,2)=="ST" )
              //  printf("fill: %s %g\n",naDet[kk],(*il)->getU());
	      goto ExitLoop;
	    }
	  }
        }
        ExitLoop:
	continue;
      }
    }
  }
}
}
