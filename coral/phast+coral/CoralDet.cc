#include <stdexcept>
#include <cmath>

#include "CoralDet.h"
#undef Check
#include "CsDetector.h"
#include "CsDigit.h"
#include "CsTrack.h"
#include "CsEvent.h"

#include "CsStrawTubesDetector.h"

namespace CS {

using namespace std;

////////////////////////////////////////////////////////////////////////////////

const CsHelix& helix_first(const CsTrack &t)
{
    if( t.getHelices().empty() )
        throw "No helices?!";

    const CsHelix *h=&t.getHelices().front();
    
    for( std::vector<CsHelix>::const_iterator it=t.getHelices().begin();
         it!=t.getHelices().end(); it++ )
        if( h->getZ()>it->getZ() )
            h=&*it;
    return *h;
}

////////////////////////////////////////////////////////////////////////////////

const CsHelix& helix_last(const CsTrack &t)
{
    if( t.getHelices().empty() )
        throw "No helices?!";

    const CsHelix *h=&t.getHelices().front();
    
    for( std::vector<CsHelix>::const_iterator it=t.getHelices().begin();
         it!=t.getHelices().end(); it++ )
        if( h->getZ()<it->getZ() )
            h=&*it;
    return *h;
}

////////////////////////////////////////////////////////////////////////////////

CoralDet::CoralDet(CsDet &det_) :
    det(&det_),
    misalignment(0)
{
    //ntuple = new TNtuple
}

////////////////////////////////////////////////////////////////////////////////

bool CoralDet::IsGoodTrack(const CsTrack &t) const
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////

const CsHelix *CoralDet::FindBestHelix(const CsTrack &track_MRS)
{
    // *****************************
    // Some checks on the track.
    // *****************************

    if( track_MRS.getHelices().empty() )
    {
        Exception("No helixes!").Print();
        return NULL;
    }

    const double det_z = GetDriftDet()->GetPosition().GetShift().Z()*10;
    
    //printf("%s (z=%g) Helices: ",GetDriftDet()->GetName().c_str(),det_z);
    
    map<float,const CsHelix*> closest_helices;
    
    for( std::vector<CsHelix>::const_iterator h=track_MRS.getHelices().begin();
         h!=track_MRS.getHelices().end(); h++ )
    {
        //printf(" %g(%c)",h->getZ(),
        //                 h->getCop()==0?'0':h->getCop()>0?'+':'-');

        float dz=fabs(h->getZ()-det_z);

        // If helix is closer 10cm, we accept it.
        if( dz<100 )
            return &*h;
        else
            // Otherwise add it to the list of found helices.
            closest_helices[dz]=&*h;
    }
    //printf("\n");
    
//     for( map<float,const CsHelix*>::const_iterator h=closest_helices.begin();
//          h!=closest_helices.end(); h++ )
//     {
//         printf("  %g",h->second->getZ());
//     }
//     printf("\n");

    if( closest_helices.empty() )
        return NULL;
    
    return closest_helices.begin()->second;
}

////////////////////////////////////////////////////////////////////////////////

const CsHelix *CoralDet::PropogateAndCheck(const CsTrack &track_MRS, Track3D &trackMRS, Track3D &trackDRS)
{
    // *****************************
    // Find the best helix
    // *****************************
    const CsHelix *helix = FindBestHelix(track_MRS);
    if( helix==NULL )
        return NULL;

    // *****************************
    // Interpolate a helix to the detector
    // *****************************

    const double det_z = GetDriftDet()->GetPosition().GetShift().Z()*10;
    CsHelix intr;
    if( !helix->Extrapolate(det_z,intr) )
    {
        cerr << "Error in a helix extrapolation.\n";
        return NULL;
    }

    // *****************************
    // Now create Track3D objects of the interpolated track
    // *****************************

    const Transform3D
        &DRS_to_MRS = GetDriftDet()->GetPosition(),
        &MRS_to_DRS = DRS_to_MRS.Inverse();

    trackMRS = Track3D(TVector3(intr.getX()/10,intr.getY()/10,intr.getZ()/10),
                       TVector3(intr.getDXDZ(),intr.getDYDZ(),1).Unit()),
    
    trackDRS=trackMRS; // this will be changed just below
    MRS_to_DRS.Transform(trackDRS);
    
    return helix;
}

////////////////////////////////////////////////////////////////////////////////

CoralDet *CoralDets::Create(CsDet &d)
{
    if( d.getAltDet()==NULL )
        return NULL;

    if( d.GetTBName().substr(0,2)=="ST" )
        return new CoralDetST(d);

    if( d.GetTBName().substr(0,2)=="DC" )
        return new CoralDetDC(d);

    if( d.GetTBName().substr(0,2)=="DW" )
        return new CoralDetDW(d);
    
    return new CoralDet(d);
}

////////////////////////////////////////////////////////////////////////////////

// Try to make an association between the track and the detector
bool CoralDet::MakeAssociation(const CsTrack &track_MRS,float misalignment2)
{
    // *****************************
    // Check on the detector
    // *****************************

    assert(NULL!=det->getAltDet());

    DriftDetector *drift_det = dynamic_cast<DriftDetector*>(det->getAltDet());
    assert(drift_det!=NULL);
    drift_det->GetObjCORAL().Clear();

    // *****************************
    // Now create Track3D objects of the interpolated track
    // *****************************

    Track3D trackMRS, trackDRS;
    const CsHelix *helix = PropogateAndCheck(track_MRS,trackMRS,trackDRS);
    if( NULL==helix )
        return false;

    const TVector3
        &hitMRS = trackMRS.GetPoint(),     // reference
        &hitDRS = trackDRS.GetPoint();     // reference

    // *****************************
    // Find the best association (amoung all digits) in the detector with the track
    // *****************************

    bool fill=false;
    CS::DriftDetectorChannel *channel_best=NULL;

    for( list<CsDigit*>::const_iterator dg = det->getMyDigits().begin();
         dg!=det->getMyDigits().end(); dg++ )
    {
        // ***************************
        // Find the channel
        // ***************************

        short int
            channel_num = (*dg)->getAddress(),
            channel_pos = int((*dg)->getData()[1]);
    
        CS::DriftDetectorChannel *channel=drift_det->FindChannel(channel_num,channel_pos);
        
        //if( channel==NULL && channel_pos!=0 )
        //    channel=drift_det->FindChannel(channel_num,0);

        if( channel==NULL )
        {
            printf("can not find channel %d,%d",(unsigned)channel_num,(unsigned)channel_pos);
            return false;
        }

        TVector3 const *spacers=NULL;
        float spacer_dist, drift_dist;  // drift_dist has a sign to indicate left/right crossing
        if( !channel->MakeIntersection(trackDRS,spacers, spacer_dist, drift_dist, misalignment2>0?misalignment2:misalignment) )
            continue;

        // At this point we know for sure that track crossed the channel (+/- misalignment)
        
        if( drift_det->GetObjCORAL().GetChan()<0 || fabs(drift_dist)<fabs(drift_det->GetObjCORAL().GetR()) )
        {
            fill=true;
            channel_best = channel;
            const double time = (*dg)->getData()[2];

            drift_det->GetObjCORAL().SetDist( drift_dist );
            drift_det->GetObjCORAL().SetTime( time );
            drift_det->GetObjCORAL().SetT0( time - (*dg)->getData()[0] );

            float r=-1;
            if( drift_det->GetObjCORAL().GetChan()>=0 )
                r = DistRTCoral(time);
            drift_det->GetObjCORAL().SetR(r);

            drift_det->GetObjCORAL().SetChan( (*dg)->getAddress() );
            drift_det->GetObjCORAL().SetChanPos( (int)(*dg)->getData()[1] );
            drift_det->GetObjCORAL().SetChanX( channel->GetShapeDRS().GetPoint().X() );
            drift_det->GetObjCORAL().SetSrcID( (int)(*dg)->getData()[3] );
            drift_det->GetObjCORAL().SetGeoID( (int)(*dg)->getData()[4] );
            drift_det->GetObjCORAL().SetCardChan( (int)(*dg)->getData()[5] );
            
            CoralDetST *st = dynamic_cast<CoralDetST *>(this);
            if( st!=NULL )
            {
                //if( st -> DetCoral() -> xray !=NULL )
                //    drift_det->GetObjCORAL().SetCorrSpacers( st -> DetCoral() -> xray -> xray_correction((*dg)->getAddress(),hitDRS.Y()) );

                // Signal propagation time velocity.
                static const double v = 28; 

                // Signal propagation time direction.
                int sign = (*dg)->getData()[1]>0 ? -1:1;

                drift_det->GetObjCORAL().SetCorrSignalPropTime( - sign * hitDRS.Y()/v );
            }
            
        }
    }

    if( !fill )
    {
        // no association!
        // But do we have an intersection with the detector?
        set<DriftDetectorChannel*> hits;
        drift_det->MakeIntersection(trackMRS,hits);
        if( !hits.empty() )
            fill=true;           // We supposed to have a hit! (non-efficiency has been detected).
    }

//     {
//         if( hits.size()>1 )
//             printf("More then one hit is found: %d!\n",hits.size());
// 
//         if( drift_det->GetName().substr(0,2)=="ST" &&
//             hits.size()>=1 && channel_best!=NULL &&
//             channel_best->GetChannelPos()!=0 )
//             if( channel_best!=*hits.begin() )
//             {
//                 printf("best hit is in %s channel(%3d,%d)\n",drift_det->GetName().c_str(),channel_best->GetChannel(),channel_best->GetChannelPos());
// 
//                 printf("%s ch=%3d chp=%2d  hMRS(x,y,z)=(%+6.1f,%+6.1f,%+8.1f)  hDRS(x,y,z)=(%+6.1f,%+6.1f,%+8.1f)\n",
//                         drift_det->GetName().c_str(),(*hits.begin())->GetChannel(), (*hits.begin())->GetChannelPos(),
//                         trackMRS.GetPoint().X(),trackMRS.GetPoint().Y(),trackMRS.GetPoint().Z(),
//                         trackDRS.GetPoint().X(),trackDRS.GetPoint().Y(),trackDRS.GetPoint().Z());
//             }
//             else
//                 printf("OK!\n");
//     }

    if( fill )
    {
        drift_det->GetObjCORAL().SetHitMRS(hitMRS);
        drift_det->GetObjCORAL().SetHitDRS(hitDRS);
        drift_det->GetObjCORAL().SetTrackAngle(trackDRS.GetDirection().X()/trackDRS.GetDirection().Z(),
                                               trackDRS.GetDirection().Y()/trackDRS.GetDirection().Z());
        drift_det->GetObjCORAL().SetTrackXi2(track_MRS.getChi2());
        //st->GetObjCORAL().SetTrackResolution(cov(0,0));
        drift_det->GetObjCORAL().SetTrackCharge(helix->getCop());
        drift_det->GetObjCORAL().SetTrackX0(track_MRS.getXX0());
        drift_det->GetObjCORAL().SetTrackHits(track_MRS.getClusters().size());
        drift_det->GetObjCORAL().SetTrackTime(track_MRS.getMeanTime());

        drift_det->GetObjCORAL().SetTrackBegin( helix_first(track_MRS).getZ() );
        drift_det->GetObjCORAL().SetTrackEnd  ( helix_last (track_MRS).getZ() );
        
        CsVertex *v = track_MRS.getFirstVertex();
        if( v!=NULL )
            drift_det->GetObjCORAL().SetPrimaryV(v->isPrimary());
    }

    return fill;
}

////////////////////////////////////////////////////////////////////////////////

bool CoralDets::MakeAssociation(const CsTrack &t,CsDet &d,float misalignment)
{
    if( d.getAltDet()==NULL )
        return false;
    
    CoralDet *&det = dets[&d];
    if( det==NULL )
    {
        det = Create(d);
        printf("NEW CoralDet %s\n",d.GetTBName().c_str());
        //det->det->getAltDet()->CreateObjectXML()->Print();
    }

    if( det==NULL )
        return false;
    
    // 'channel' is NULL if track did not cross the detector
    if( det->MakeAssociation(t,misalignment) )
    {
        assert(det->det->getAltDet()!=NULL);
        DriftDetector *dd = dynamic_cast<DriftDetector*>(det->det->getAltDet());
        assert(dd!=NULL);
        hits.push_back(CoralDet::Hit(dd->GetObjCORAL(),det));
        return true;
    }
    
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void CoralDets::DoubleHitsAnalyse(void)
{
    for( vector<CoralDet::Hit>::iterator it1=hits.begin(); it1!=hits.end(); it1++ )
    {
        if( it1->co.GetChan()<0 )
            continue;

        vector<CoralDet::Hit>::iterator it2=it1;
        for(  it2++; it2!=hits.end(); it2++ )
        {
            if( it2->co.GetChan()<0 )
                continue;
            it1->coral_det->DoubleHitsAnalyse(*it1,*it2);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void CoralDets::Fill(void)
{
    for( vector<CoralDet::Hit>::iterator it=hits.begin(); it!=hits.end(); it++ )
    {
        assert(it->coral_det->det->getAltDet()!=NULL);
        DriftDetector *dd = dynamic_cast<DriftDetector*>(it->coral_det->det->getAltDet());
        assert(dd!=NULL);
        dd->GetObjCORAL() = it->co;
        dd->FillCORALTree();
    }
}

////////////////////////////////////////////////////////////////////////////////

bool CoralDet::IsDoubleLayer(const string &s) const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool CoralDet::DoubleHitsAnalyse(Hit &h1,Hit &h2)
{
    if( !h1.coral_det->IsDoubleLayer(h2.coral_det->det->getAltDet()->GetName()) )
        return false;

    if( abs(h1.co.GetChan()-h2.co.GetChan())>2 )
        return false;
    
    h1.co.SetChan2(h2.co.GetChan());
    h2.co.SetChan2(h1.co.GetChan());

    h1.co.SetTime2(h2.co.GetTime());
    h2.co.SetTime2(h1.co.GetTime());

    h1.co.SetR2(h2.co.GetR());
    h2.co.SetR2(h1.co.GetR());

    h1.co.SetDist2(h2.co.GetDist());
    h2.co.SetDist2(h1.co.GetDist());
    
    return true;
}

////////////////////////////////////////////////////////////////////////////////

}
