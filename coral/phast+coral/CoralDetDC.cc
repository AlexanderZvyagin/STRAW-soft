#include "CoralDet.h"
#undef Check
#include "CsDetector.h"
#include "CsDigit.h"
#include "CsTrack.h"
#include "CsDriftChamberDetector.h"

namespace CS {

////////////////////////////////////////////////////////////////////////////////

bool CoralDetDC::IsDoubleLayer(const string &s) const
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

float CoralDetDC::DistRTCoral(float t) const
{
    bool error;
    float r= DetCoral()->getRTRelation()->getRfromT(t-DetCoral()->getT0(),error);
    if( error )
        r=-1;
    return r;
}

////////////////////////////////////////////////////////////////////////////////

}
