#include "CoralDet.h"
#undef Check
#include "CsDetector.h"
#include "CsDigit.h"
#include "CsTrack.h"
#include "CsStrawTubesDetector.h"

namespace CS {

////////////////////////////////////////////////////////////////////////////////
// Returns true if this detector and the one named s form a double-layer.

bool CoralDetST::IsDoubleLayer(const string &s) const
{
    const string &n = det->GetTBName();
    
    if( n.length()<8 )
        return false;

    if( s.length()!=n.length() )
        return false;
    
    if( s.substr(0,6)!=n.substr(0,6) )
        return false;
    
    if( s[7]!=n[7] )
        return false;

    if( (s[6]=='u' && n[6]=='d') || (s[6]=='d' && n[6]=='u') )
        return true;

    return false;

//     if(0)
//         if( (h1.co.GetSrcID()!=h2.co.GetSrcID()) ||
//             (h1.co.GetGeoID()!=h2.co.GetGeoID()) ||
//             (abs(h1.co.GetCardChan()-h2.co.GetCardChan())!=1) )
//         {
//             printf("u: %d,%d    d: %d,%d\n",
//                     h1.co.GetGeoID(), h1.co.GetCardChan(),
//                     h2.co.GetGeoID(), h2.co.GetCardChan() );
//             return false;
//         }
//         else
//         {
//             printf("ST double hit ok: %s geo=%d ch=%d\n",
//                     h1.coral_det->det->getAltDet()->GetName().c_str(),
//                     h1.co.GetGeoID(), h1.co.GetCardChan());
//         }
}

////////////////////////////////////////////////////////////////////////////////

float CoralDetST::DistRTCoral(float t) const
{
    CsRTRelation *rt = DetCoral()->getRTRelation();
    if( rt==NULL )
        return -1;

    bool error;
    float r = rt->getRfromT(t-DetCoral()->getT0(),error);
    if( error )
        return -1;

    return r;
}

////////////////////////////////////////////////////////////////////////////////


}
