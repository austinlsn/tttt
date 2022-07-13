#ifndef SPLITFILEANDADDFORTRANSFER_H
#define SPLITFILEANDADDFORTRANSFER_H

#include <vector>
#include "SamplesCore.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"
#include "TDirectory.h"
#include "TFile.h"


namespace SampleHelpers{
  void splitFileAndAddForTransfer(TString const& stroutput){
    using namespace std;
    using namespace IvyStreamHelpers;

    // Trivial case: If not running on condor, there is no need to transfer. Just exit.
    if (!SampleHelpers::checkRunOnCondor()){
      SampleHelpers::addToCondorTransferList(stroutput);
      return;
    }

    TDirectory* curdir = gDirectory;
    size_t const size_limit = std::pow(1024, 3);

    TFile* finput = TFile::Open(stroutput, "read");
    curdir->cd();

    size_t const size_input = finput->GetSize();
    size_t const nchunks = size_input/size_limit+1;
    std::vector<TString> fnames; fnames.reserve(nchunks);
    if (nchunks>1){
      std::vector<TFile*> foutputlist; foutputlist.reserve(nchunks);

      IVYout << "splitFileAndAddForTransfer: Splitting " << stroutput << " into " << nchunks << " chunks:" << endl;
      for (size_t ichunk=0; ichunk<nchunks; ichunk++){
        TString fname = stroutput;
        TString strchunk = Form("_chunk_%zu_of_%zu%s", ichunk, nchunks, ".root");
        HelperFunctions::replaceString<TString, TString const>(fname, ".root", strchunk);
        IVYout << "\t- Making new file " << fname << "..." << endl;
        TFile* foutput = TFile::Open(fname, "recreate");
        foutputlist.push_back(foutput);
        fnames.push_back(fname);
      }

      std::vector<TDirectory*> outputdirs; outputdirs.reserve(nchunks);
      for (auto& ff:foutputlist) outputdirs.push_back(ff);
      HelperFunctions::distributeObjects(finput, outputdirs, MiscUtils::INFO);

      for (auto& ff:foutputlist) ff->Close();
      IVYout << "\t- Splitting is completed." << endl;
    }
    else{
      IVYout << "splitFileAndAddForTransfer: " << stroutput << " will not be split into chunks." << endl;
      fnames.push_back(stroutput);
    }

    finput->Close();
    curdir->cd();

    for (auto const& fname:fnames) SampleHelpers::addToCondorTransferList(fname);
  }
}


#endif
