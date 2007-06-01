#include "CoralDet.h"
#undef Check
#include "CsDetector.h"
#include "CsDigit.h"
#include "CsTrack.h"
#include "CsDWDetector.h"

namespace CS {

////////////////////////////////////////////////////////////////////////////////

bool CoralDetDW::IsDoubleLayer(const string &s) const
{
    const string &n = det->GetTBName();
    
    if( n.length()<8 )
        return false;

    if( s.length()!=n.length() )
        return false;
    
    if( s.substr(0,5)!=n.substr(0,5) )
        return false;
    
    if( abs(s[5]-n[5])!=1 )
        return false;

    if( s.substr(6,2)!=n.substr(6,2) )
        return false;
    
    return true;
}

////////////////////////////////////////////////////////////////////////////////

float CoralDetDW::DistRTCoral(float t) const
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
