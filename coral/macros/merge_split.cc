#include <regex.h>
#include <popt.h>
#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <fstream>

#include "TFile.h"
#include "TNtuple.h"

using namespace std;

bool string_match(const string &str, const string &pattern)
{
    int    status;
    regex_t    re;

    if( regcomp(&re, pattern.c_str(), REG_EXTENDED|REG_NOSUB) != 0 )
        return false;

    status = regexec(&re, str.c_str(), (size_t) 0, NULL, 0);
    regfree(&re);

    if (status != 0)
        return false;

    return true;
}

void ntuple_copy(TNtuple *&ntuple_dst,TNtuple &ntuple_src)
{
    // Now we need to create the ntuple variables list.
    char name[222]="";

    auto_ptr<TIterator> iter(ntuple_src.GetListOfBranches()->MakeIterator());
    TBranch *b;
    float vars[55];
    int vars_counter=0;
    while( (b=(TBranch*)iter->Next())!=NULL )
    {
        if( vars_counter>=sizeof(vars)/sizeof(*vars) )
        {
            printf("Wow!!! Too may vaiables!\n");
            return;
        }
        ntuple_src.SetBranchAddress(b->GetName(),&vars[vars_counter++]);
        sprintf(name+strlen(name),"%s:",b->GetName());
    }
    name[strlen(name)-1]=0;
    if( strlen(name)>=sizeof(name) )
    {
        printf("ntuple_copy(): FATAL ERROR: short internal buffer!\n");
        abort();
    }
    
    if( ntuple_dst==NULL )
    {
        //printf("Creation of a new ntuple: %s\n",ntuple_src.GetName());
        ntuple_dst = new TNtuple(ntuple_src.GetName(),ntuple_src.GetTitle(),name);
    }

    for( int i=0; i<ntuple_src.GetEntries(); i++ )
    {
        if( ntuple_src.GetEntry(i)<=0 )
        {
            printf("Can not get entry??\n");
            break;
        }

        ntuple_dst->Fill(vars);
    }
}

/*! 
*/
void merge_split(const vector<string> &files,const string &dir_out)
{
    map<string, pair<TFile*,TNtuple*> > det_objs;
    
    for( vector<string>::const_iterator f=files.begin(); f!=files.end(); f++ )
    {
        printf("Working with file \"%s\"...\n",f->c_str());
        
        auto_ptr<TFile> f_in(TFile::Open(f->c_str()));
        if( !f_in->IsOpen() )
            continue;

        TIter nextobj(f_in->GetListOfKeys());
        for( TObject *ptr; NULL!=(ptr=nextobj.Next()); )
        {
            // A TNtuple object name should finish on _CORAL.
            int l = strlen(ptr->GetName());
            if( l<=6 )
                continue;   // The object name is too short!
            
            if( strcmp(ptr->GetName()+l-6,"_CORAL") )
                continue;   // Not correct ended
            
            TNtuple *nt = dynamic_cast<TNtuple*>(f_in->Get(ptr->GetName()));
            if( nt==NULL )
                continue;   // It is not ntuple!
            
            //printf("Found ntuple: \"%s\"\n",nt->GetName());
            pair<TFile*,TNtuple*> & o = det_objs[nt->GetName()];
            if( o.first==NULL )
            {
                assert(o.second==NULL);
                o.first = TFile::Open((dir_out+"/"+string(ptr->GetName(),l-6)+".root").c_str(),"RECREATE");
            }
            if( !o.first->IsOpen() )
                throw "Can not create file!";
            
            ntuple_copy(o.second,*nt);
        }
    }
    
    printf("Saving the files...\n");
    for( map<string, pair<TFile*,TNtuple*> >::iterator o=det_objs.begin(); o!=det_objs.end(); o++ )
    {
        TFile *f=o->second.first;
        f -> Write();
        f -> Close();
        delete f;
    }
}

void get_files_list(vector<string> &files,const string &dir,const string &pattern)
{
    char cmd[dir.length()+33];
    sprintf(cmd,"rfdir %s > rfdir.lst",dir.c_str());
    if( system(cmd) )
        throw "Failed to run rfdir!";
    
    ifstream f("rfdir.lst");
    if( !f.is_open() )
        throw "Can not read rfdir.lst file.";
    
    string s;
    while( getline(f,s) )
    {
        char *ss = strrchr(s.c_str(),' ');
        if( ss==NULL )
            throw "get_files_list(): bad line";
        ss++;   // skip the space
        
        if( string_match(ss,pattern) )
        {
            sprintf(cmd,"%s/%s",dir.c_str(),ss);
            files.push_back(cmd);
        }
    }
}

int main(int argc,const char *argv[])
{
    try
    {
        char
            *dir_in = "./",
            *dir_out = "./",
            *pattern= "^cdr.*\\.root";

        struct poptOption options[] =
        {
            { "dir-in",     '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &dir_in, 0,
                                          "Directory with input files. "
                                          "Use rfio:/ prefix for CASTOR files.", "PATH" },
            { "dir-out",    '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &dir_out,     0,
                                          "Directory for output root files. Use rfio:/ prefix for CASTOR files.", "PATH" },
            { "pattern",    '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &pattern,     0,
                                          "Pattern for input ROOT files.", "STRING" },
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        poptContext poptcont=poptGetContext(NULL,argc,argv,options,0);
        poptSetOtherOptionHelp(poptcont,
            "<options...>\n"
            "  Author: Alexander Zvyagin <Alexander.Zvyagin@cern.ch>\n"
        );
        
        int rc;
        while( (rc=poptGetNextOpt(poptcont))>0 )
        {
	        fprintf(stderr, "bad argument %s: %s\n",
		            poptBadOption(poptcont, POPT_BADOPTION_NOALIAS),
		            poptStrerror(rc));
            throw "Bad argument!";
        }

        // ==================

        vector<string> files;

        get_files_list(files,dir_in,pattern);
        
        for( const char *f; NULL!=(f=poptGetArg(poptcont)); )
            files.push_back(f);

        merge_split(files,dir_out);
    
        return 0;
    }
    catch(const char *e)
    {
        printf("%s\n",e);
    }
    catch(const std::exception &e)
    {
        printf("%s\n",e.what());
    }
    catch(...)
    {
        printf("Unknown exception.\n");
    }

    return 1;
}
