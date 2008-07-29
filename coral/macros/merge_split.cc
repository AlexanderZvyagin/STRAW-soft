#include <regex.h>
#include <popt.h>
#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <fstream>
#include <sstream>

#include "TFile.h"
#include "TNtuple.h"

#include "mysignal.h"

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
    
    for( vector<string>::const_iterator f=files.begin(); f!=files.end() && !flag_end; f++ )
    {
        if( f->size()==0 )
            continue;
    
        string name=*f;
        if( name.substr(0,6)=="/castor/" )
            name="rfio:"+name;
        if( name[name.size()-1]=='\n' )
            name.resize(name.size()-1);

        printf("Working with the file \"%s\"...\n",name.c_str());
        
        auto_ptr<TFile> f_in(TFile::Open(name.c_str()));
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
    char path[PATH_MAX];
    snprintf(path,PATH_MAX,"rfdir %s",dir.c_str());
    FILE *fp = popen(path, "r");

    if (fp == NULL)
        throw "pope() failed!";

    while( fgets(path,PATH_MAX,fp)!=NULL )
    {
        char *ss = strrchr(path,' ');
        if( ss==NULL )
            throw "get_files_list(): bad line";
        ss++;   // skip the space
        
        if( string_match(ss,pattern) )
        {
            sprintf(path,"%s/%s",dir.c_str(),ss);
            files.push_back(path);
        }
    }
    
    if( -1==pclose(fp) )
        throw "pclose() failed!";
}

int main(int argc,const char *argv[])
{
    set_signal_handler();

    try
    {
        const char
            *dir_in = NULL,
            *dir_out = NULL,
            *pattern= "^cdr.*\\.root";

        struct poptOption options[] =
        {
            { "pattern",    '\0', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,  &pattern,     0,
                                          "Pattern for input ROOT files.", "STRING" },
            POPT_AUTOHELP
            POPT_TABLEEND
        };
        poptContext poptcont=poptGetContext(NULL,argc,argv,options,0);
        poptSetOtherOptionHelp(poptcont,
            "[options...] dir-in dir-out\n"
            "  dir-in:  directory name with input files (cdr*.root)\n"
            "  dir-out: directory name where output files will be written (ST0*.root)"
//            "  Author:  Alexander Zvyagin <Zvyagin.Alexander@gmail.com>\n"
        );
        
        int rc;
        while( (rc=poptGetNextOpt(poptcont))>0 )
        {
	        fprintf(stderr, "bad argument %s: %s\n",
		            poptBadOption(poptcont, POPT_BADOPTION_NOALIAS),
		            poptStrerror(rc));
            throw "Bad argument!";
        }

        dir_in  = poptGetArg(poptcont);
        dir_out = poptGetArg(poptcont);
        if( dir_in==NULL || dir_out==NULL || poptPeekArg(poptcont)!=NULL )
        {
            poptPrintHelp(poptcont,stdout,0);
            return 1;
        }

        // ==================

        vector<string> files;

        get_files_list(files,dir_in,pattern);
        printf("%d files found.\n",files.size());
        
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
