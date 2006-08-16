/*

  This macro will add histograms from a list of root files and write them
  to a target root file. The target file is newly created and must not be
  identical to one of the source files.

  Author: Sven A. Schmidt, sven.schmidt@cern.ch
  Date:   13.2.2001

  This code is based on the hadd.C example by Rene Brun and Dirk Geppert,
  which had a problem with directories more than one level deep.
  (see macro hadd_old.C for this previous implementation).
  
  The macro from Sven has been enhanced by 
     Anne-Sylvie Nicollerat <Anne-Sylvie.Nicollerat@cern.ch>
   to automatically add Trees (via a chain of trees).
  
  To use this macro, modify the file names in function hadd.
  
  NB: This macro is provided as a tutorial.
      Use $ROOTSYS/bin/hadd to merge many histogram files

 */


#include <string.h>
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TKey.h"
#include "Riostream.h"

TList *FileList;
TFile *Target;

void MergeRootfile( TDirectory *target, TList *sourcelist );


void hadd() {
   // in an interactive ROOT session, edit the file names
   // Target and FileList, then
   // root > .L hadd.C
   // root > hadd()
   
  Target = TFile::Open( "result.root", "RECREATE" );
  
  FileList = new TList();
    //FileList->Add( TFile::Open("") );
  FileList->Add( TFile::Open("root/cdr09001-37059.root") );
  FileList->Add( TFile::Open("root/cdr09002-37059.root") );
  FileList->Add( TFile::Open("root/cdr09003-37059.root") );
  FileList->Add( TFile::Open("root/cdr09004-37059.root") );
  FileList->Add( TFile::Open("root/cdr09005-37059.root") );
  FileList->Add( TFile::Open("root/cdr09006-37059.root") );
  FileList->Add( TFile::Open("root/cdr09007-37059.root") );
  FileList->Add( TFile::Open("root/cdr09008-37059.root") );
  FileList->Add( TFile::Open("root/cdr09009-37059.root") );
  FileList->Add( TFile::Open("root/cdr09010-37059.root") );
  FileList->Add( TFile::Open("root/cdr09011-37059.root") );
  FileList->Add( TFile::Open("root/cdr09012-37059.root") );
  FileList->Add( TFile::Open("root/cdr09013-37059.root") );
  FileList->Add( TFile::Open("root/cdr09014-37059.root") );
  FileList->Add( TFile::Open("root/cdr09015-37059.root") );
  FileList->Add( TFile::Open("root/cdr09016-37059.root") );
  FileList->Add( TFile::Open("root/cdr09017-37059.root") );
  FileList->Add( TFile::Open("root/cdr09018-37059.root") );
  FileList->Add( TFile::Open("root/cdr09019-37059.root") );
  FileList->Add( TFile::Open("root/cdr10001-37059.root") );
  FileList->Add( TFile::Open("root/cdr10002-37059.root") );
  FileList->Add( TFile::Open("root/cdr10003-37059.root") );
  FileList->Add( TFile::Open("root/cdr10004-37059.root") );
  FileList->Add( TFile::Open("root/cdr10005-37059.root") );
  FileList->Add( TFile::Open("root/cdr10006-37059.root") );
  FileList->Add( TFile::Open("root/cdr10007-37059.root") );
  FileList->Add( TFile::Open("root/cdr10008-37059.root") );
  FileList->Add( TFile::Open("root/cdr10009-37059.root") );
  FileList->Add( TFile::Open("root/cdr10010-37059.root") );
  FileList->Add( TFile::Open("root/cdr10011-37059.root") );
  FileList->Add( TFile::Open("root/cdr10012-37059.root") );
  FileList->Add( TFile::Open("root/cdr10013-37059.root") );
  FileList->Add( TFile::Open("root/cdr10014-37059.root") );
  FileList->Add( TFile::Open("root/cdr10015-37059.root") );
  FileList->Add( TFile::Open("root/cdr10016-37059.root") );
  FileList->Add( TFile::Open("root/cdr10017-37059.root") );
  FileList->Add( TFile::Open("root/cdr10018-37059.root") );
  FileList->Add( TFile::Open("root/cdr10019-37059.root") );
  FileList->Add( TFile::Open("root/cdr11001-37059.root") );
  FileList->Add( TFile::Open("root/cdr11002-37059.root") );
  FileList->Add( TFile::Open("root/cdr11003-37059.root") );
  FileList->Add( TFile::Open("root/cdr11004-37059.root") );
  FileList->Add( TFile::Open("root/cdr11005-37059.root") );
  FileList->Add( TFile::Open("root/cdr11006-37059.root") );
  FileList->Add( TFile::Open("root/cdr11007-37059.root") );
  FileList->Add( TFile::Open("root/cdr11008-37059.root") );
  FileList->Add( TFile::Open("root/cdr11009-37059.root") );
  FileList->Add( TFile::Open("root/cdr11010-37059.root") );
  FileList->Add( TFile::Open("root/cdr11011-37059.root") );
  FileList->Add( TFile::Open("root/cdr11012-37059.root") );
  FileList->Add( TFile::Open("root/cdr11013-37059.root") );
  FileList->Add( TFile::Open("root/cdr11014-37059.root") );
  FileList->Add( TFile::Open("root/cdr11015-37059.root") );
  FileList->Add( TFile::Open("root/cdr11016-37059.root") );
  FileList->Add( TFile::Open("root/cdr11017-37059.root") );
  FileList->Add( TFile::Open("root/cdr11018-37059.root") );
  FileList->Add( TFile::Open("root/cdr11019-37059.root") );
  FileList->Add( TFile::Open("root/cdr12001-37059.root") );
  FileList->Add( TFile::Open("root/cdr12002-37059.root") );
  FileList->Add( TFile::Open("root/cdr12003-37059.root") );
  FileList->Add( TFile::Open("root/cdr12004-37059.root") );
  FileList->Add( TFile::Open("root/cdr12005-37059.root") );
  FileList->Add( TFile::Open("root/cdr12006-37059.root") );
  FileList->Add( TFile::Open("root/cdr12007-37059.root") );
  FileList->Add( TFile::Open("root/cdr12008-37059.root") );
  FileList->Add( TFile::Open("root/cdr12009-37059.root") );
  FileList->Add( TFile::Open("root/cdr12010-37059.root") );
  FileList->Add( TFile::Open("root/cdr12011-37059.root") );
  FileList->Add( TFile::Open("root/cdr12012-37059.root") );
  FileList->Add( TFile::Open("root/cdr12013-37059.root") );
  FileList->Add( TFile::Open("root/cdr12014-37059.root") );
  FileList->Add( TFile::Open("root/cdr12015-37059.root") );
  FileList->Add( TFile::Open("root/cdr12016-37059.root") );
  FileList->Add( TFile::Open("root/cdr12017-37059.root") );
  FileList->Add( TFile::Open("root/cdr12018-37059.root") );
  FileList->Add( TFile::Open("root/cdr12019-37059.root") );
  FileList->Add( TFile::Open("root/cdr13001-37059.root") );
  FileList->Add( TFile::Open("root/cdr13002-37059.root") );
  FileList->Add( TFile::Open("root/cdr13003-37059.root") );
  FileList->Add( TFile::Open("root/cdr13004-37059.root") );
  FileList->Add( TFile::Open("root/cdr13005-37059.root") );
  FileList->Add( TFile::Open("root/cdr13006-37059.root") );
  FileList->Add( TFile::Open("root/cdr13007-37059.root") );
  FileList->Add( TFile::Open("root/cdr13008-37059.root") );
  FileList->Add( TFile::Open("root/cdr13009-37059.root") );
  FileList->Add( TFile::Open("root/cdr13010-37059.root") );
  FileList->Add( TFile::Open("root/cdr13011-37059.root") );
  FileList->Add( TFile::Open("root/cdr13012-37059.root") );
  FileList->Add( TFile::Open("root/cdr13013-37059.root") );
  FileList->Add( TFile::Open("root/cdr13014-37059.root") );
  FileList->Add( TFile::Open("root/cdr13015-37059.root") );
  FileList->Add( TFile::Open("root/cdr13016-37059.root") );
  FileList->Add( TFile::Open("root/cdr13017-37059.root") );
  FileList->Add( TFile::Open("root/cdr13018-37059.root") );
  FileList->Add( TFile::Open("root/cdr13019-37059.root") );
  FileList->Add( TFile::Open("root/cdr14001-37059.root") );
  FileList->Add( TFile::Open("root/cdr14002-37059.root") );
  FileList->Add( TFile::Open("root/cdr14003-37059.root") );
  FileList->Add( TFile::Open("root/cdr14004-37059.root") );
  FileList->Add( TFile::Open("root/cdr14005-37059.root") );
  FileList->Add( TFile::Open("root/cdr14006-37059.root") );
  FileList->Add( TFile::Open("root/cdr14007-37059.root") );
  FileList->Add( TFile::Open("root/cdr14008-37059.root") );
  FileList->Add( TFile::Open("root/cdr14009-37059.root") );
  FileList->Add( TFile::Open("root/cdr14010-37059.root") );
  FileList->Add( TFile::Open("root/cdr14011-37059.root") );
  FileList->Add( TFile::Open("root/cdr14012-37059.root") );
  FileList->Add( TFile::Open("root/cdr14013-37059.root") );
  FileList->Add( TFile::Open("root/cdr14014-37059.root") );
  FileList->Add( TFile::Open("root/cdr14015-37059.root") );
  FileList->Add( TFile::Open("root/cdr14016-37059.root") );
  FileList->Add( TFile::Open("root/cdr14017-37059.root") );
  FileList->Add( TFile::Open("root/cdr14018-37059.root") );
  FileList->Add( TFile::Open("root/cdr14019-37059.root") );
  FileList->Add( TFile::Open("root/cdr15001-37059.root") );
  FileList->Add( TFile::Open("root/cdr15002-37059.root") );
  FileList->Add( TFile::Open("root/cdr15003-37059.root") );
  FileList->Add( TFile::Open("root/cdr15004-37059.root") );
  FileList->Add( TFile::Open("root/cdr15005-37059.root") );
  FileList->Add( TFile::Open("root/cdr15006-37059.root") );
  FileList->Add( TFile::Open("root/cdr15007-37059.root") );
  FileList->Add( TFile::Open("root/cdr15008-37059.root") );
  FileList->Add( TFile::Open("root/cdr15009-37059.root") );
  FileList->Add( TFile::Open("root/cdr15010-37059.root") );
  FileList->Add( TFile::Open("root/cdr15011-37059.root") );
  FileList->Add( TFile::Open("root/cdr15012-37059.root") );
  FileList->Add( TFile::Open("root/cdr15013-37059.root") );
  FileList->Add( TFile::Open("root/cdr15014-37059.root") );
  FileList->Add( TFile::Open("root/cdr15015-37059.root") );
  FileList->Add( TFile::Open("root/cdr15016-37059.root") );
  FileList->Add( TFile::Open("root/cdr15017-37059.root") );
  FileList->Add( TFile::Open("root/cdr15018-37059.root") );
  FileList->Add( TFile::Open("root/cdr15019-37059.root") );
  FileList->Add( TFile::Open("root/cdr16001-37059.root") );
  FileList->Add( TFile::Open("root/cdr16002-37059.root") );
  FileList->Add( TFile::Open("root/cdr16003-37059.root") );
  FileList->Add( TFile::Open("root/cdr16004-37059.root") );
  FileList->Add( TFile::Open("root/cdr16005-37059.root") );
  FileList->Add( TFile::Open("root/cdr16006-37059.root") );
  FileList->Add( TFile::Open("root/cdr16007-37059.root") );
  FileList->Add( TFile::Open("root/cdr16008-37059.root") );
  FileList->Add( TFile::Open("root/cdr16009-37059.root") );
  FileList->Add( TFile::Open("root/cdr16010-37059.root") );
  FileList->Add( TFile::Open("root/cdr16011-37059.root") );
  FileList->Add( TFile::Open("root/cdr16012-37059.root") );
  FileList->Add( TFile::Open("root/cdr16013-37059.root") );
  FileList->Add( TFile::Open("root/cdr16014-37059.root") );
  FileList->Add( TFile::Open("root/cdr16015-37059.root") );
  FileList->Add( TFile::Open("root/cdr16016-37059.root") );
  FileList->Add( TFile::Open("root/cdr16017-37059.root") );
  FileList->Add( TFile::Open("root/cdr16018-37059.root") );
  FileList->Add( TFile::Open("root/cdr16019-37059.root") );
  FileList->Add( TFile::Open("root/cdr17001-37059.root") );
  FileList->Add( TFile::Open("root/cdr17002-37059.root") );
  FileList->Add( TFile::Open("root/cdr17003-37059.root") );
  FileList->Add( TFile::Open("root/cdr17004-37059.root") );
  FileList->Add( TFile::Open("root/cdr17005-37059.root") );
  FileList->Add( TFile::Open("root/cdr17006-37059.root") );
  FileList->Add( TFile::Open("root/cdr17007-37059.root") );
  FileList->Add( TFile::Open("root/cdr17008-37059.root") );
  FileList->Add( TFile::Open("root/cdr17009-37059.root") );
  FileList->Add( TFile::Open("root/cdr17010-37059.root") );
  FileList->Add( TFile::Open("root/cdr17012-37059.root") );
  FileList->Add( TFile::Open("root/cdr17015-37059.root") );
  FileList->Add( TFile::Open("root/cdr17017-37059.root") );
  FileList->Add( TFile::Open("root/cdr17019-37059.root") );
  FileList->Add( TFile::Open("root/cdr18007-37059.root") );
  FileList->Add( TFile::Open("root/cdr18009-37059.root") );
  FileList->Add( TFile::Open("root/cdr18010-37059.root") );
  FileList->Add( TFile::Open("root/cdr18012-37059.root") );
  FileList->Add( TFile::Open("root/cdr18016-37059.root") );
  FileList->Add( TFile::Open("root/cdr18018-37059.root") );
  FileList->Add( TFile::Open("root/cdr18019-37059.root") );
  FileList->Add( TFile::Open("root/cdr19001-37059.root") );
  FileList->Add( TFile::Open("root/cdr19003-37059.root") );
  FileList->Add( TFile::Open("root/cdr19005-37059.root") );
  FileList->Add( TFile::Open("root/cdr19006-37059.root") );
  FileList->Add( TFile::Open("root/cdr19007-37059.root") );
  FileList->Add( TFile::Open("root/cdr19009-37059.root") );
  FileList->Add( TFile::Open("root/cdr19010-37059.root") );
  FileList->Add( TFile::Open("root/cdr19011-37059.root") );
  FileList->Add( TFile::Open("root/cdr19013-37059.root") );
  FileList->Add( TFile::Open("root/cdr19014-37059.root") );
  FileList->Add( TFile::Open("root/cdr19015-37059.root") );
  FileList->Add( TFile::Open("root/cdr19016-37059.root") );
  FileList->Add( TFile::Open("root/cdr19017-37059.root") );
  FileList->Add( TFile::Open("root/cdr19018-37059.root") );
  FileList->Add( TFile::Open("root/cdr19019-37059.root") );
  FileList->Add( TFile::Open("root/cdr20003-37059.root") );
  FileList->Add( TFile::Open("root/cdr20004-37059.root") );
  FileList->Add( TFile::Open("root/cdr20007-37059.root") );
  FileList->Add( TFile::Open("root/cdr20009-37059.root") );
  FileList->Add( TFile::Open("root/cdr20010-37059.root") );
  FileList->Add( TFile::Open("root/cdr20013-37059.root") );
  FileList->Add( TFile::Open("root/cdr20014-37059.root") );
  FileList->Add( TFile::Open("root/cdr20016-37059.root") );
  FileList->Add( TFile::Open("root/cdr20018-37059.root") );
  FileList->Add( TFile::Open("root/cdr20019-37059.root") );
  FileList->Add( TFile::Open("root/cdr21007-37059.root") );
  FileList->Add( TFile::Open("root/cdr21009-37059.root") );
  FileList->Add( TFile::Open("root/cdr21010-37059.root") );
  FileList->Add( TFile::Open("root/cdr21012-37059.root") );
  FileList->Add( TFile::Open("root/cdr21013-37059.root") );
  FileList->Add( TFile::Open("root/cdr21016-37059.root") );
  FileList->Add( TFile::Open("root/cdr21017-37059.root") );
  FileList->Add( TFile::Open("root/cdr21018-37059.root") );
  FileList->Add( TFile::Open("root/cdr21019-37059.root") );

  MergeRootfile( Target, FileList );

}   

void MergeRootfile( TDirectory *target, TList *sourcelist ) {

  //  cout << "Target path: " << target->GetPath() << endl;
  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  TFile *first_source = (TFile*)sourcelist->First();
  first_source->cd( path );
  TDirectory *current_sourcedir = gDirectory;

  // loop over all keys in this directory
  TChain *globChain = 0;
  TIter nextkey( current_sourcedir->GetListOfKeys() );
  TKey *key;
  while ( (key = (TKey*)nextkey())) {

    // read object from first source file
    first_source->cd( path );
    TObject *obj = key->ReadObj();

    if ( obj->IsA()->InheritsFrom( "TH1" ) ) {
      // descendant of TH1 -> merge it

      //      cout << "Merging histogram " << obj->GetName() << endl;
      TH1 *h1 = (TH1*)obj;

      // loop over all source files and add the content of the
      // correspondant histogram to the one pointed to by "h1"
      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      while ( nextsource ) {
        
        // make sure we are at the correct directory level by cd'ing to path
        nextsource->cd( path );
        TH1 *h2 = (TH1*)gDirectory->Get( h1->GetName() );
        if ( h2 ) {
          h1->Add( h2 );
          delete h2; // don't know if this is necessary, i.e. if 
                     // h2 is created by the call to gDirectory above.
        }

        nextsource = (TFile*)sourcelist->After( nextsource );
      }
    }
    else if ( obj->IsA()->InheritsFrom( "TTree" ) ) {
      
      // loop over all source files create a chain of Trees "globChain"
      const char* obj_name= obj->GetName();

      globChain = new TChain(obj_name);
      globChain->Add(first_source->GetName());
      TFile *nextsource = (TFile*)sourcelist->After( first_source );
      //      const char* file_name = nextsource->GetName();
      // cout << "file name  " << file_name << endl;
     while ( nextsource ) {
     	  
       globChain->Add(nextsource->GetName());
       nextsource = (TFile*)sourcelist->After( nextsource );
     }

    } else if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      // it's a subdirectory

      cout << "Found subdirectory " << obj->GetName() << endl;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newdir = target->mkdir( obj->GetName(), obj->GetTitle() );

      // newdir is now the starting point of another round of merging
      // newdir still knows its depth within the target file via
      // GetPath(), so we can still figure out where we are in the recursion
      MergeRootfile( newdir, sourcelist );

    } else {

      // object is of no type that we know or can handle
      cout << "Unknown object type, name: " 
           << obj->GetName() << " title: " << obj->GetTitle() << endl;
    }

    // now write the merged histogram (which is "in" obj) to the target file
    // note that this will just store obj in the current directory level,
    // which is not persistent until the complete directory itself is stored
    // by "target->Write()" below
    if ( obj ) {
      target->cd();

      //!!if the object is a tree, it is stored in globChain...
	if(obj->IsA()->InheritsFrom( "TTree" ))
	  globChain->Write( key->GetName() );
	else
	obj->Write( key->GetName() );
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  // save modifications to target file
  target->Write();

}
