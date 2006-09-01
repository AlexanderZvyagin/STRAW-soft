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

namespace Sasha {

using namespace CS;

vector<CoralDet::Hit> hits;
CoralDets coral_dets;

TH1F *h_ST_hits=NULL, *h_ST_hits6=NULL, *h_ST_hits10=NULL;

namespace Albert
{
TDirectory *dPrivate;

map<string,TH3F*> Udist3;
void CoralUserInit();
void CoralUserEvent();

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

    coral_dets.ReadDetectorsDAT(*CsInit::Instance()->getDetectorTable());

    coral_dets.AddDetectors();
    
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
    coral_dets.Clear();
    
    // The loop on all tracks: find all tracks which pass the detectors
    for( list<CsTrack*>::const_iterator t=CsEvent::Instance()->getTracks().begin();
         t!=CsEvent::Instance()->getTracks().end(); t++,coral_dets.tracks_counter++ )
    {
        set<string> hits6, hits10;
        
        // Find all channels associated with the track
        for( std::map<std::string,CsDet*>::const_iterator d=CsDet::GetAllDetectors().begin();
             d!=CsDet::GetAllDetectors().end(); d++ )
        {
            if( coral_dets.MakeAssociation(**t,*d->second,0) )
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
    
    coral_dets.DoubleHitsAnalyse();
    coral_dets.Fill();

    Albert::CoralUserEvent();

    return true;
}

////////////////////////////////////////////////////////////////////////////////

namespace Albert
{
void CoralUserInit()
{
    TFile *fs = gFile;
    fs->cd();
  
    dPrivate = new TDirectory("dPrivate","My Histos");
    dPrivate-> cd();

    for ( std::map<std::string,CsDet*>::const_iterator it=CsDet::GetAllDetectors().begin();
          it!=CsDet::GetAllDetectors().end(); it++ )
    {
        const CsDetector *det = dynamic_cast<const CsDetector *>(it->second);
        
        if( det==NULL )
            continue;
        
        if( it->first.substr(0,2)!="ST" &&
            it->first.substr(0,2)!="DC" &&
            it->first.substr(0,2)!="DW" )
            continue;

        const string &det_name = it->first;
        char histo_name[55];
        sprintf(histo_name, "Udist_%s", det_name.c_str());

        string          histo_title = "Profile of " + det_name;
        unsigned int    nbins       = det->getNWir();
        double          dimension   = (nbins - 1) * det->getWirP();
        double          center      = det-> getWirD() + dimension/2; 
        float           minx        = center - dimension/2 - det->getWirP()/2;
        float           maxx        = center + dimension/2 + det->getWirP()/2;

        Udist3[det_name] = new TH3F( histo_name,histo_title.c_str(), nbins, minx, maxx, 12, -1800, 1800, 200, -2, 2 );
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

    float HelZ = v.front().getZ();
    float HelZba = v.back().getZ();
    CsVertex* fvrt = trk->getFirstVertex();
    float Chi2 = trk->getChi2();

    if( fvrt==0 || !fvrt->isPrimary() ) continue;  // this particle has NO PRIMARY VERTEX

    // evaluate the residuals of a track vs cluster and fill into histograms
    int ClusterSize = cl.size();

    if ( ClusterSize > 20 && HelZba > 4500 ) {    // more than 20 hits AND last hit after SM1
      for ( list<CsCluster*>::const_iterator  il = cl.begin(); il != cl.end(); il++) {
        const list<CsDetector*> &dets = (*il)->getDetsList();
	if ( dets.size() <= 0 ) { cout << "dets <= 0 !!!    How is this possible?" << endl; continue; }
	int NumHelix = v.size();
	if ( NumHelix < 2 ) { cout << "Helix Vector too small !!!    Size = " << NumHelix << endl; continue; }
	//if ( NumHelix >  Ndet+2 ) { cout << "Helix Vector too large !!!    Size = " << NumHelix << endl; continue; }
	double Zcl           = (*il)->getW();
        const string &nameCl = dets.front()->GetTBName();
        HepMatrix iR         = dets.front()->getRotWRSInv();
        TH3F * &h = Udist3[nameCl];
        if( h==NULL )
            continue;
        for ( int ii = 1; ii <= NumHelix - 2; ii++ )
        {
            if( fabs(Zcl-v[ii].getZ())>1e-3 )
                continue;
            
	    double X        = v[ii].getX();
	    double Y        = v[ii].getY();
	    double residual = (*il)->getU() - iR(1,1) * X - iR(1,2) * Y;
 	    h->Fill(iR(1,1)*X + iR(1,2)*Y, iR(2,1)*X + iR(2,2)*Y, residual);
            break;
        }
	continue;
      }
    }
  }
}
}
}
