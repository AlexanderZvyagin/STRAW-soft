/*! \file db_write.cc
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include "TSQLServer.h"

#include "dcs.h"

static bool debug=true;

////////////////////////////////////////////////////////////////////////////////

// Copy the DCS values from a file to DB
void db_write_HV(TSQLServer *db,const char *file_name)
{
    if( db==NULL )
        return;
    db->Query("USE dcs;");

    FILE *f=fopen(file_name,"r");
    if( f==NULL )
    {
        printf("Can not open file \"%s\"\n",file_name);
        return;
    }

    char buf[333], *s;

    while( NULL!=(s=fgets(buf,sizeof(buf),f)) )
    {
        if( strlen(s)<3 || 0!=strncmp(s,"dcs",3) )
            continue;

        char dcs_name[77], full_channel_name[77];
        float voltage;
        int year,month,day,hour,min,sec;

        // remove the end-of-line
        if( strlen(s)>0 && s[strlen(s)-1]=='\n' )
            s[strlen(s)-1]=0;

        if( 9!=sscanf(s,"%s %s %g %d %d %d %d %d %d",dcs_name,full_channel_name,&voltage,&year,&month,&day,&hour,&min,&sec) )
        {
            if( debug )
                printf("Can not read line \"%s\"\n",s);
            continue;
        }

        char timestamp[22];
        sprintf(timestamp,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",year,month,day,hour,min,sec);
        
        int dcs,dcs_module,dcs_channel, DL, can;
        char detector[77], detector_channel[77];
        
        // Decode DCS module and channel
        if( 4!=sscanf(dcs_name,"dcs%d:Iseg/can%d/ma%d/ch%d",&dcs,&can,&dcs_module,&dcs_channel) )
        {
            printf("Can not read dcs channel information in \"%s\"\n",dcs_name);
            continue;
        }

        // Decode detector name and channel    
        char *s2=strrchr(full_channel_name,'/');
        if( 0==strncmp(s2,"/Notused",8) )
            continue;
        if( s2==NULL || 3!=sscanf(s2,"/St_Hv_Dl%d_%4s_%s",&DL,detector,detector_channel) )
        {
            printf("Can not read straw channel information in \"%s\"\n",s2);
            continue;
        }

        sprintf(buf,"REPLACE INTO dcs.HV (time,dcs_module,dcs_channel,DL,detector,channel,voltage) VALUES(%s,%d,%d,%d,'%s','%s',%g);",
                timestamp,dcs_module,dcs_channel,DL,detector,detector_channel,voltage);
        if( !db->Query(buf) )
            printf("**** FAILED: %s\n",buf);
    }

    fclose(f);    
}

////////////////////////////////////////////////////////////////////////////////

// Copy the DCS values from a file to DB
void db_write_current(TSQLServer *db,const char *file_name)
{
    if( db==NULL )
        return;
    db->Query("USE dcs;");

    FILE *f=fopen(file_name,"r");
    if( f==NULL )
    {
        printf("Can not open file \"%s\"\n",file_name);
        return;
    }

    printf("db_write_current()\n");

    char buf[333], *s;

    while( NULL!=(s=fgets(buf,sizeof(buf),f)) )
    {
        if( strlen(s)<3 || 0!=strncmp(s,"dcs",3) )
            continue;

        char dcs_name[33], full_channel_name[33];
        float current;
        int year,month,day,hour,min,sec;

        // remove the end-of-line
        if( strlen(s)>0 && s[strlen(s)-1]=='\n' )
            s[strlen(s)-1]=0;

        if( 9!=sscanf(s,"%s %s %g %d %d %d %d %d %d",dcs_name,full_channel_name,&current,&year,&month,&day,&hour,&min,&sec) )
        {
            if( debug )
                printf("Can not read line \"%s\"\n",s);
            continue;
        }

        char timestamp[22];
        sprintf(timestamp,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",year,month,day,hour,min,sec);
        //printf("%s\n",timestamp);
        
        int dcs,dcs_module,dcs_channel, DL;
        char detector[22], detector_channel[55];
        
        // Decode DCS module and channel
        if( 3!=sscanf(dcs_name,"dcs%d:Module%d.Channel%d",&dcs,&dcs_module,&dcs_channel) )
        {
            printf("Can not read dcs channel information: %s\n",dcs_name);
            continue;
        }

        // Decode detector name and channel    
        if( 3!=sscanf(full_channel_name,"DL%d_%6s_%s",&DL,detector,detector_channel) )
        {
            printf("Can not read straw channel information.\n");
            continue;
        }

        sprintf(buf,"REPLACE INTO dcs.current (time,dcs_module,dcs_channel,DL,detector,channel,current) VALUES(%s,%d,%d,%d,'%s','%s',%g);",
                timestamp,dcs_module,dcs_channel,DL,detector,detector_channel,current);
        if( !db->Query(buf) )
            printf("**** FAILED: %s\n",buf);
    }

    fclose(f);    
}

////////////////////////////////////////////////////////////////////////////////

// Copy the DCS values from a file to DB
void db_write_temperature(TSQLServer *db,const char *file_name)
{
    if( db==NULL )
        return;
    db->Query("USE dcs;");

    FILE *f=fopen(file_name,"r");
    if( f==NULL )
    {
        printf("Can not open file \"%s\"\n",file_name);
        return;
    }

    printf("db_write_temperature()\n");

    char buf[333], *s;

    while( NULL!=(s=fgets(buf,sizeof(buf),f)) )
    {
        if( strlen(s)<3 || 0!=strncmp(s,"dcs",3) )
            continue;

        char dcs_name[33], channel_name[44];
        float temperature;
        int year,month,day,hour,min,sec;

        // remove the end-of-line
        if( strlen(s)>0 && s[strlen(s)-1]=='\n' )
            s[strlen(s)-1]=0;

        if( 9!=sscanf(s,"%s %s %g %d %d %d %d %d %d",dcs_name,channel_name,&temperature,&year,&month,&day,&hour,&min,&sec) )
        {
            if( debug )
                printf("Can not read line \"%s\"\n",s);
            continue;
        }

        char timestamp[22];
        sprintf(timestamp,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",year,month,day,hour,min,sec);
        //printf("%s\n",timestamp);
        
        int dcs;
        char detector[55], sensor[55];
        // Decode DCS module and channel
        char *slash = strrchr(channel_name,'/'), *char_=strchr(slash,'_');
        assert( strlen(slash)<sizeof(detector) );
        if( slash!=NULL && char_!=NULL )
        {
            strncpy(detector,slash+1,char_-slash-1);
            strcpy(sensor,char_+1);
        }
        else
            printf("Can not decode name: %s\n",channel_name);

        if( 0==strncmp("ST",detector,2) )
	{
            sprintf(buf,"REPLACE INTO dcs.Temperature (time,detector,sensor,temperature) VALUES(%s,'%s','%s',%g);",
                    timestamp,detector,sensor,temperature);
            if( !db->Query(buf) )
                printf("**** FAILED: %s\n",buf);
        }
    }

    fclose(f);    
}

////////////////////////////////////////////////////////////////////////////////

// Copy the DCS values from a file to DB
void db_write_hall_pressure_temperature(TSQLServer *db,const char *file_name)
{
    if( db==NULL )
        return;
    db->Query("USE dcs;");

    FILE *f=fopen(file_name,"r");
    if( f==NULL )
    {
        printf("Can not open file \"%s\"\n",file_name);
        return;
    }

    printf("db_write_hall_pressure_temperature()\n");

    char buf[333], *s;

    while( NULL!=(s=fgets(buf,sizeof(buf),f)) )
    {
        char dcs_name[37],junk[13];
        float v;
        int year,month,day,hour,min,sec;

        // remove the end-of-line
        if( strlen(s)>0 && s[strlen(s)-1]=='\n' )
            s[strlen(s)-1]=0;

        if( 9!=sscanf(s,"%s %s %g %d %d %d %d %d %d",dcs_name,junk,&v,&year,&month,&day,&hour,&min,&sec) )
        {
            if( debug )
                printf("Can not read line \"%s\"\n",s);
            continue;
        }

        char timestamp[22];
        sprintf(timestamp,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",year,month,day,hour,min,sec);

       // if( 0==strcmp(dcs_name,"Patm_hall") )
         //   sprintf(buf,"REPLACE INTO dcs.Hall_pressure (time,pressure) VALUES(%s,%g);",
           //         timestamp,v);
        //else
        if( 0==strcmp(junk,"Env/T_Outside") )
            sprintf(buf,"REPLACE INTO dcs.Hall_temperature (time,temperature) VALUES(%s,%g);",
                    timestamp,v);
        //else
        //if( 0==strcmp(dcs_name,"Hum1_HAll") )
          //  sprintf(buf,"REPLACE INTO dcs.Hall_humidity (time,humidity) VALUES(%s,%g);",
                 //   timestamp,v);
        else
        {
            printf("Unknown value: %s\n",dcs_name);
            buf[0]=0;
        }
        
        if( buf[0]!=0 && !db->Query(buf) )
            printf("**** FAILED: %s\n",buf);
    }

    fclose(f);    
}

////////////////////////////////////////////////////////////////////////////////
