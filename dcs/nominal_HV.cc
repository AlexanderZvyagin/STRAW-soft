/*! \file nominal_HV.cc
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include <ctime>
#include <cstring>
#include "TSQLServer.h"

float get_nominal_HV(TSQLServer &db,const char *detector,const char *channel,time_t t)
{
    if( NULL!=strstr(channel,"6mm") )
        return 1640;

    if( NULL!=strstr(channel,"10mm") )
        return 1780;

    return 0;
}
