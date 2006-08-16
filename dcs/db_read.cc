/*! \file db_read.cc
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include "dcs.h"

#include <vector>

#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"

using namespace std;

namespace {
bool debug=false;
}

////////////////////////////////////////////////////////////////////////////////

// Add measurement with a time order
void add_measurement(TSQLRow* m, vector<TSQLRow*> &measurements)
{
    if( measurements.empty() )
    {
        measurements.push_back(m);
        return;
    }
    
    TSQLRow* last=measurements.back();

    if( strcmp(last->GetField(0),m->GetField(0))>0 )
    {
        printf("BAD TIME ORDERING!!!!\n");
    }
    else
        measurements.push_back(m);
}

////////////////////////////////////////////////////////////////////////////////

TSQLRow *get_previous_measurement(TSQLServer &db,const char *detector,const char *channel,time_t t1,const char *table_name,int time_past)
{
    char cond[111]="", q[222];

    if( *detector!=0 )
        sprintf(cond+strlen(cond)," AND detector='%s'",detector);

    if( *channel!=0 )
        sprintf(cond+strlen(cond)," AND channel='%s'",channel);
    
    sprintf(q,"SELECT * FROM %s WHERE time<=%s AND time>=%s %s ORDER BY time DESC LIMIT 1",
            table_name,time_to_timestamp(t1).c_str(),time_to_timestamp(t1-time_past).c_str(),cond);

    TSQLResult *r = db.Query(q);
    
    if( r==NULL )
        return NULL;

    TSQLRow *rr=r->Next();
    
    //delete r;
    return rr;
}

////////////////////////////////////////////////////////////////////////////////

TSQLRow *get_previous_measurement(TSQLServer &db,const char *cuts,time_t t1,const char *table_name)
{
    char q[222];

    sprintf(q,"SELECT * FROM %s WHERE time<=%s %s ORDER BY time DESC LIMIT 1",
            table_name,time_to_timestamp(t1).c_str(),cuts);

    TSQLResult *r = db.Query(q);
    
    if( r==NULL )
        return NULL;

    TSQLRow *rr=r->Next();
    
    return rr;
}

////////////////////////////////////////////////////////////////////////////////

TSQLResult *get_measurements(TSQLServer &db,const char *detector,const char *channel,
                             time_t t1,time_t t2,const char *table_name,
                             std::vector<TSQLRow*> &measurements)
{
    // Check that the time winodw is fine.
    if(t2<=t1)
    {
        printf("Bad time window: %s\n",interval_s(t1,t2).c_str());
        return NULL;
    }

    char q[555];
    sprintf(q,"SELECT * FROM %s WHERE time>%s AND time<%s",
               table_name,
               time_to_timestamp(t1).c_str(),time_to_timestamp(t2).c_str());

    if( *detector!=0 )
        sprintf(q+strlen(q)," AND detector='%s'",detector);

    if( *channel!=0 )
        sprintf(q+strlen(q)," AND channel='%s'",channel);

    // Retrieve information from the DB.
    TSQLResult *table = db.Query(q);

    if( table==NULL )
    {
        printf("Your query was: %s\n",q);
        return NULL;
    }

    if( debug )
        printf("%s: %s %s:  %d total DB entries\n",table_name,detector,channel,table->GetRowCount());

    if( table==NULL )
    {
        printf("get_measurements(): read table has failed!\n");
        return NULL;
    }

    time_t time, t_prev=0;
    TSQLRow *m_start=get_previous_measurement(db,detector,channel,t1,table_name);

    if( m_start!=NULL )
        add_measurement(m_start,measurements);

    while(1)
    {
        TSQLRow *m=table->Next();
        if( m==NULL )
            break;
        time=timestamp_to_time((char*)m->GetField(0));

        if( t_prev>time )
            printf("Not time-ordered entries in the DB!\n");

        if( time>t2 )
        {
            printf("We should not be here!\n");
            break;
        }

        t_prev=time;
        add_measurement(m,measurements);
    }

    return table;
}

////////////////////////////////////////////////////////////////////////////////

TSQLResult *get_measurements(TSQLServer &db,const char *table_name,
                             time_t t1,time_t t2, const char *cuts,
                             std::vector<TSQLRow*> &measurements)
{
    // Check that the time winodw is fine.
    if(t2<=t1)
    {
        printf("Bad time window: %s\n",interval_s(t1,t2).c_str());
        return NULL;
    }

    char q[555];
    sprintf(q,"SELECT * FROM %s WHERE time>%s AND time<%s %s",
               table_name,
               time_to_timestamp(t1).c_str(),time_to_timestamp(t2).c_str(),
               cuts);

    // Retrieve information from the DB.
    TSQLResult *table = db.Query(q);
    if( table==NULL )
    {
        printf("Your query was: %s\n",q);
        return NULL;
    }

    if( debug )
        printf("%s: %s:  %d total DB entries\n",table_name,cuts,table->GetRowCount());

    if( table==NULL )
    {
        printf("get_measurements(): read table has failed!\n");
        return NULL;
    }

    time_t time, t_prev=0;
    TSQLRow *m_start=get_previous_measurement(db,cuts,t1,table_name);

    if( m_start!=NULL )
        add_measurement(m_start,measurements);

    while(1)
    {
        TSQLRow *m=table->Next();
        if( m==NULL )
            break;
        time=timestamp_to_time((char*)m->GetField(0));

        if( t_prev>time )
            printf("Not time-ordered entries in the DB!\n");

        if( time>t2 )
        {
            printf("We should not be here!\n");
            break;
        }

        t_prev=time;
        add_measurement(m,measurements);
    }

    return table;
}

////////////////////////////////////////////////////////////////////////////////
