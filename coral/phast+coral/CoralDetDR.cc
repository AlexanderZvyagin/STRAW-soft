#include "CoralDet.h"
#undef Check
#include "CsDetector.h"
#include "CsDigit.h"
#include "CsTrack.h"
#include "CsRichWallDetector.h"

namespace CS {

////////////////////////////////////////////////////////////////////////////////

bool CoralDetDR::IsDoubleLayer(const string &s) const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////

float CoralDetDR::DistRTCoral(float t) const
{
    bool error;
    float r= DetCoral()->getRTRelation()->getRfromT(t-DetCoral()->getT0(),error);
    if( error )
        r=-1;
    return r;
}

////////////////////////////////////////////////////////////////////////////////

}
