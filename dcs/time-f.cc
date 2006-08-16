/*! \file time-f.cc
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include <ctime>
#include <string>

#include "TDatime.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

time_t timestamp_to_time(const char *timestamp,bool fix)
{
    return TDatime(timestamp).Convert();
}

////////////////////////////////////////////////////////////////////////////////

string time_to_timestamp(time_t t)
{
    TDatime dt(t);
    char buf[33];
    sprintf(buf,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",
            dt.GetYear(),dt.GetMonth(),dt.GetDay(),
            dt.GetHour(),dt.GetMinute(),dt.GetSecond());
    return buf;
}

////////////////////////////////////////////////////////////////////////////////

string interval_s(time_t t1,time_t t2,int v)
{
    switch( v )
    {
        case 1:
        {
            char buf[66];
            sprintf(buf,"[%s,%s]",time_to_timestamp(t1).c_str(),time_to_timestamp(t2).c_str());
            return buf;
        }
        
        default:
        {
            char st1[26],st2[26], buf[66];

            strcpy(st1,ctime(&t1));
            strcpy(st2,ctime(&t2));
            st1[strlen(st1)-1]=0;
            st2[strlen(st2)-1]=0;

            sprintf(buf,"[%s , %s]",st1,st2);

            return buf;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
