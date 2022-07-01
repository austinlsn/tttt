#ifndef TRIGGERHELPERSCORE_H
#define TRIGGERHELPERSCORE_H

#include <unordered_map>
#include "HLTTriggerPathProperties.h"


namespace TriggerHelpers{
  enum TriggerType{
    kTripleLep=0,

    kDoubleMu,
    kDoubleMu_Extra,
    kDoubleMu_PFHT,
    kDoubleMu_Prescaled,

    kDoubleEle,
    kDoubleEle_HighPt,
    kDoubleEle_PFHT,

    kMuEle,
    kMuEle_Extra,
    kMuEle_PFHT,

    kSingleMu,
    kSingleMu_Prescaled,
    kSingleMu_HighPt,
    kSingleMu_Control,
    // Subdivisions of kSingleMu_Control
    kSingleMu_Control_NoIso,
    kSingleMu_Control_Iso,

    kSingleEle,
    kSingleEle_Prescaled,
    kSingleEle_HighPt,
    kSingleEle_Control,
    // Subdivisions of kSingleEle_Control
    kSingleEle_Control_NoIso,
    kSingleEle_Control_Iso,

    kSinglePho,

    kAK8PFJet_Control,
    kVBFJets_Control,
    kPFHT_Control,
    kMET_Control,
    kPFMET_Control,
    kPFHT_PFMET_Control,
    kPFMET_MHT_Control,
    kPFHT_PFMET_MHT_Control,

    nTriggerTypes
  };

  bool hasRunRangeExclusions(std::string const& name, HLTTriggerPathProperties const** out_hltprop = nullptr); // This is to allow a string-based recognition of run range exclusions.
}

#endif
