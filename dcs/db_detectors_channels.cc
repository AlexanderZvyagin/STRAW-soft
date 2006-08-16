/*! \file db_detectors_channels.cc
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include <memory.h>

#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"

#include "dcs.h"

////////////////////////////////////////////////////////////////////////////////

void get_detectors(TSQLServer &db,time_t t1,time_t t2,map<string,set<string> > &det_chan)
{
    char buf[555];
    sprintf(buf,"SELECT detector,channel FROM dcs.HV WHERE time>%s AND time<%s",
                 time_to_timestamp(t1).c_str(),time_to_timestamp(t2).c_str());

    auto_ptr<TSQLResult> r(db.Query(buf));

    if( r.get()==NULL )
    {
        printf("**** FAILED: %s\n",buf);
        throw "get_detectors(): problem!";
    }
    
    for( int i=0; i<r->GetRowCount(); i++ )
    {
        TSQLRow *rr=r->Next();
        if( rr==NULL )
            throw "get_detectors(): very-very bad!\n";

        det_chan[rr->GetField(0)].insert(rr->GetField(1));
    }
}

////////////////////////////////////////////////////////////////////////////////
