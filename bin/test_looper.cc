/*
This is an example looper to go through the different setup, looper, and output methods.
*/

#include <string>
#include <iostream>
#include <iomanip>
#include "TSystem.h"
#include "TDirectory.h"
#include "TFile.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"
#include "IvyFramework/IvyDataTools/interface/BaseTree.h"
#include "IvyFramework/IvyDataTools/interface/SimpleEntry.h"
#include "GlobalCollectionNames.h"
#include "RunLumiEventBlock.h"
#include "SamplesCore.h"
#include "MuonSelectionHelpers.h"
#include "ElectronSelectionHelpers.h"
#include "AK4JetSelectionHelpers.h"
#include "IsotrackSelectionHelpers.h"
#include "ParticleSelectionHelpers.h"
#include "ParticleDisambiguator.h"
#include "MuonHandler.h"
#include "ElectronHandler.h"
#include "JetMETHandler.h"
#include "EventFilterHandler.h"
#include "SimEventHandler.h"
#include "GenInfoHandler.h"
#include "IsotrackHandler.h"
#include "SamplesCore.h"
#include "FourTopTriggerHelpers.h"
#include "DileptonHandler.h"
#include "InputChunkSplitter.h"
#include "SplitFileAndAddForTransfer.h"

using namespace std;
using namespace IvyStreamHelpers;

// A class to keep track of events that pass or fail selection
struct SelectionTracker{
  std::vector<TString> ordered_reqs;
  std::unordered_map<TString, std::pair<double, double>> req_sumws_pair_map;

  void accumulate(TString const& strsel, double const& wgt);
  void print() const;
};


void SelectionTracker::accumulate(TString const& strsel, double const& wgt){
  if (!HelperFunctions::checkListVariable(ordered_reqs, strsel)){
    req_sumws_pair_map[strsel] = std::pair<double, double>(0, 0);
    ordered_reqs.push_back(strsel);
  }
  auto it_req_sumws_pair = req_sumws_pair_map.find(strsel);
  it_req_sumws_pair->second.first += wgt;
  it_req_sumws_pair->second.second += wgt*wgt;
}
void SelectionTracker::print() const{
  IVYout << "Selection summary:" << endl;
  for (auto const& strsel:ordered_reqs){
    auto it_req_sumws_pair = req_sumws_pair_map.find(strsel);
    IVYout << "\t- " << strsel << ": " << setprecision(15) << it_req_sumws_pair->second.first << " +- " << std::sqrt(it_req_sumws_pair->second.second) << endl;
  }
}


int ScanChain(std::string const& strdate, std::string const& dset, std::string const& proc, double const& xsec, int const& ichunk, int const& nchunks, SimpleEntry const& extra_arguments){
  // Ctrl+C should cause the program to gracefully exit without errors unless you are running on Condor.
  bool const isCondorRun = SampleHelpers::checkRunOnCondor();
  if (!isCondorRun) std::signal(SIGINT, SampleHelpers::setSignalInterrupt);

  TDirectory* curdir = gDirectory;

  /***********************/
  /* COMMON GLOBAL SETUP */
  /***********************/

  // Systematics hypothesis: For now, keep as "Nominal" for the purposes of this example. This could be made an argument, or the looper could go through each relevant systematic hypothesis in each event.
  SystematicsHelpers::SystematicVariationTypes const theGlobalSyst = SystematicsHelpers::sNominal;

  // Data period quantities
  auto const& theDataPeriod = SampleHelpers::getDataPeriod();
  auto const& theDataYear = SampleHelpers::getDataYear();

  // Keep this as true for checks as cleaning jets from fakeable leptons
  constexpr bool useFakeableIdForPhysicsChecks = true;
  ParticleSelectionHelpers::setUseFakeableIdForPhysicsChecks(useFakeableIdForPhysicsChecks);

  // b-tagging |eta| threshold should be 2.4 in year<=2016 (Phase-0 tracker acceptance) and 2.5 afterward (Phase-1 tracker)
  float const absEtaThr_ak4jets = (theDataYear<=2016 ? AK4JetSelectionHelpers::etaThr_btag_Phase0Tracker : AK4JetSelectionHelpers::etaThr_btag_Phase1Tracker);

  // Integrated luminosity for the data period
  double const lumi = SampleHelpers::getIntegratedLuminosity(theDataPeriod);
  // If theDataPeriod is not a specific era, print the data eras that are actually contained.
  // lumi is computed for the contained eras only.
  IVYout << "Valid data periods for " << theDataPeriod << ": " << SampleHelpers::getValidDataPeriods() << endl;
  IVYout << "Integrated luminosity: " << lumi << endl;

  // This is the output directory.
  // Output should always be recorded as if you are running the job locally.
  // If we are running on Condor, we will inform the batch job later on that some files would need transfer.
  TString coutput_main = TString("output/ExampleLooper/") + strdate.data() + "/" + theDataPeriod;
  if (!isCondorRun) coutput_main = ANALYSISPKGPATH + "/test/" + coutput_main;
  HostHelpers::ExpandEnvironmentVariables(coutput_main);
  gSystem->mkdir(coutput_main, true);

  /*********************/
  /* INTERPRET OPTIONS */
  /*********************/

  // One should be able to run on the entire (real) data for a given era.
  // In that case, dset and proc are comma-separated strings of same size.
  // Note that running over multiple simulation data sets is forbidden below.
  std::vector<std::pair<std::string, std::string>> dset_proc_pairs;
  {
    std::vector<std::string> dsets, procs;
    HelperFunctions::splitOptionRecursive(dset, dsets, ',', false);
    HelperFunctions::splitOptionRecursive(proc, procs, ',', false);
    HelperFunctions::zipVectors(dsets, procs, dset_proc_pairs);
  }
  bool const has_multiple_dsets = dset_proc_pairs.size()>1;

  // Configure output file name
  std::string output_file;
  extra_arguments.getNamedVal("output_file", output_file);
  if (output_file==""){
    if (has_multiple_dsets){
      IVYerr << "Multiple data sets are passed. An output file identifier must be specified through the option 'output_file'." << endl;
      assert(0);
    }
    output_file = proc;
  }
  if (nchunks>0) output_file = Form("%s_%i_of_%i", output_file.data(), ichunk, nchunks);

  // Configure full output file names with path
  TString stroutput = coutput_main + "/" + output_file.data() + ".root"; // This is the output ROOT file.
  TString stroutput_log = coutput_main + "/log_" + output_file.data() + ".out"; // This is the output log file.
  TString stroutput_err = coutput_main + "/log_" + output_file.data() + ".err"; // This is the error log file.
  TString stroutput_csv = coutput_main + "/" + output_file.data() + ".csv"; // This is the csv file to be filled with important numbers
  ofstream output_csv;
  
  IVYout.open(stroutput_log.Data());
  IVYerr.open(stroutput_err.Data());
  output_csv.open(stroutput_csv);

  output_csv << "Event, nTightEle, nTightMu, hasOS, nJets, nBJets, hasZCand" << std::endl; // Run,Lumi,

  // In case the user wants to run on particular files
  std::string input_files;
  extra_arguments.getNamedVal("input_files", input_files);

  // Shorthand option for the Run 2 UL analysis proposal
  bool use_shorthand_Run2_UL_proposal_config = true; // CHANGED
  extra_arguments.getNamedVal("shorthand_Run2_UL_proposal_config", use_shorthand_Run2_UL_proposal_config);

  // Options to set alternative muon and electron IDs, or b-tagging WP
  std::string muon_id_name;
  std::string electron_id_name;
  std::string btag_WPname;
  double minpt_jets = 30;	// 40
  double minpt_bjets = 30;
  double minpt_l3 = 0;
  if (use_shorthand_Run2_UL_proposal_config){
    muon_id_name = electron_id_name = "TopMVA_Run2";
    btag_WPname = "loose";
    minpt_jets = minpt_bjets = 25;
    minpt_l3 = 10;
  }
  else{
    extra_arguments.getNamedVal("muon_id", muon_id_name);
    extra_arguments.getNamedVal("electron_id", electron_id_name);
    extra_arguments.getNamedVal("btag", btag_WPname);
    extra_arguments.getNamedVal("minpt_jets", minpt_jets);
    extra_arguments.getNamedVal("minpt_bjets", minpt_bjets);
    extra_arguments.getNamedVal("minpt_l3", minpt_l3);
  }

  double minpt_miss = 0;	// 50
  extra_arguments.getNamedVal("minpt_miss", minpt_miss);
  double minHT_jets = 0;	// 300
  extra_arguments.getNamedVal("minHT_jets", minHT_jets);

  if (muon_id_name!=""){
    IVYout << "Switching to muon id " << muon_id_name << "..." << endl;
    MuonSelectionHelpers::setSelectionTypeByName(muon_id_name);
  }
  else IVYout << "Using default muon id = " << MuonSelectionHelpers::selection_type << "..." << endl;

  if (electron_id_name!=""){
    IVYout << "Switching to electron id " << electron_id_name << "..." << endl;
    ElectronSelectionHelpers::setSelectionTypeByName(electron_id_name);
  }
  else IVYout << "Using default electron id = " << ElectronSelectionHelpers::selection_type << "..." << endl;

  double const minpt_anyjet = std::min(minpt_jets, minpt_bjets);
  AK4JetSelectionHelpers::SelectionBits bit_preselection_btag = AK4JetSelectionHelpers::kPreselectionTight_BTagged_Medium;
  if (btag_WPname!=""){
    std::string btag_WPname_lower;
    HelperFunctions::lowercase(btag_WPname, btag_WPname_lower);
    IVYout << "Switching to b-tagging WP " << btag_WPname_lower << "..." << endl;
    if (btag_WPname_lower=="loose") bit_preselection_btag = AK4JetSelectionHelpers::kPreselectionTight_BTagged_Loose;
    else if (btag_WPname_lower=="medium") bit_preselection_btag = AK4JetSelectionHelpers::kPreselectionTight_BTagged_Medium;
    else if (btag_WPname_lower=="tight") bit_preselection_btag = AK4JetSelectionHelpers::kPreselectionTight_BTagged_Tight;
    else{
      IVYerr << "btag=" << btag_WPname << " is not implemented." << endl;
      assert(0);
    }
  }
  else IVYout << "Using default b-tagging WP = " << static_cast<int>(bit_preselection_btag)-static_cast<int>(AK4JetSelectionHelpers::kPreselectionTight_BTagged_Loose) << "..." << endl;

  /***************************/
  /* SETUP TRIGGER SELECTION */
  /***************************/

  // Trigger configuration
  std::vector<TriggerHelpers::TriggerType> requiredTriggers_Dilepton{
    TriggerHelpers::kDoubleMu,
    TriggerHelpers::kDoubleEle,
    TriggerHelpers::kMuEle
  };
  // These PFHT triggers were used in the 2016 analysis. They are a bit more efficient.
  if (theDataYear==2016){
    requiredTriggers_Dilepton = std::vector<TriggerHelpers::TriggerType>{
      TriggerHelpers::kDoubleMu_PFHT,
      TriggerHelpers::kDoubleEle_PFHT,
      TriggerHelpers::kMuEle_PFHT
    };
    // Related to triggers is how we apply loose and fakeable IDs in electrons.
    // This setting should vary for 2016 when analyzing fake rates instead of the signal or SR-like control regions.
    // If trigger choices change, this setting may not be relevant either.
    if (ElectronSelectionHelpers::selection_type == ElectronSelectionHelpers::kCutbased_Run2) ElectronSelectionHelpers::setApplyMVALooseFakeableNoIsoWPs(true);
  }
  // We use bare HLT names to require events to pass a trigger.
  // If HLT object matching is desired, one should use the full HLT menu properties.
  std::vector<std::string> const hltnames_Dilepton = TriggerHelpers::getHLTMenus(requiredTriggers_Dilepton);
  auto triggerPropsCheckList_Dilepton = TriggerHelpers::getHLTMenuProperties(requiredTriggers_Dilepton);

  /******************/
  /* SETUP HANDLERS */
  /******************/

  // Declare handlers
  GenInfoHandler genInfoHandler;
  SimEventHandler simEventHandler;
  EventFilterHandler eventFilter(requiredTriggers_Dilepton);
  MuonHandler muonHandler;
  ElectronHandler electronHandler;
  JetMETHandler jetHandler;
  // We do not use isolated tracks, but we could think about it.
  IsotrackHandler isotrackHandler;

  // These are called handlers, but they are more like helpers.
  DileptonHandler dileptonHandler;
  ParticleDisambiguator particleDisambiguator;

  // Some advanced event filters
  // Allows tracking of run number/lumi block/event number in data
  eventFilter.setTrackDataEvents(true);
  // If true, eventFilter will test the tracked data events for duplicates.
  eventFilter.setCheckUniqueDataEvent(has_multiple_dsets);
  // If true, a veto on HLT path run ranges will be applied on each trigger for runs where that trigger is not (supposed to be) used.
  // Note that this is ALSO DONE IN SIMULATION as time dependency is implemented through SimEventHandler.
  eventFilter.setCheckHLTPathRunRanges(true);

  // Once everything is done, do the safe thing and return to the original ROOT directory.
  curdir->cd();

  // We conclude the setup of event processing specifications and move on to I/O configuration next.

  // Open the output ROOT file
  TFile* foutput = TFile::Open(stroutput, "recreate"); foutput->cd();
  // BaseTree is the wrapper to communicate with ROOT TTrees.
  // SimpleEntry is a container class to communicate with an output BaseTree.
  SimpleEntry rcd_output;	// OUTPUT BRANCHES
  BaseTree* tout = new BaseTree("SkimTree");
  tout->setAutoSave(0);

  // Again, do the safe thing and return back to the original ROOT directory.
  curdir->cd();

  /**************************/
  /* SETUP INPUT ROOT TREES */
  /**************************/

  // Acquire input tree/chains
  TString strinputdpdir = theDataPeriod;
  if (SampleHelpers::testDataPeriodIsLikeData(theDataPeriod)){
    if (theDataYear==2016){
      if (SampleHelpers::isAPV2016Affected(theDataPeriod)) strinputdpdir = Form("%i_APV", theDataYear);
      else strinputdpdir = Form("%i_NonAPV", theDataYear);
    }
    else strinputdpdir = Form("%i", theDataYear);
  }

  signed char is_sim_data_flag = -1; // =0 for sim, =1 for data
  int nevents_total = 0;
  unsigned int nevents_total_traversed = 0;
  std::vector<BaseTree*> tinlist; tinlist.reserve(dset_proc_pairs.size());
  std::unordered_map<BaseTree*, double> tin_normScale_map;
  for (auto const& dset_proc_pair:dset_proc_pairs){
    TString strinput = SampleHelpers::getInputDirectory() + "/" + strinputdpdir + "/" + dset_proc_pair.second.data();
    TString cinput = (input_files=="" ? strinput + ("/DY_2l_M_50_1.root","/DY_2l_M_50_2.root") : strinput + "/" + input_files.data()); // CHANGED FROM '/*.root' 
    IVYout << "Accessing input files " << cinput << "..." << endl;
    TString const sid = SampleHelpers::getSampleIdentifier(dset_proc_pair.first);
    bool const isData = SampleHelpers::checkSampleIsData(sid);
    BaseTree* tin = new BaseTree(cinput, "Events", "", (isData ? "" : "Counters"));
    tin->sampleIdentifier = sid;
    if (!isData){
      if (xsec<0.){
        IVYerr << "xsec = " << xsec << " is not valid." << endl;
        assert(0);
      }
      if (has_multiple_dsets){
        IVYerr << "Should not process multiple data sets in sim. mode. Aborting..." << endl;
        assert(0);
      }
    }
    if (is_sim_data_flag==-1) is_sim_data_flag = (isData ? 1 : 0);
    else if (is_sim_data_flag != (isData ? 1 : 0)){
      IVYerr << "Should not process data and simulation at the same time." << endl;
      assert(0);
    }

    double sum_wgts = (isData ? 1 : 0);
    if (!isData){
      int ix = 1;
      switch (theGlobalSyst){
      case SystematicsHelpers::ePUDn:
        ix = 2;
        break;
      case SystematicsHelpers::ePUUp:
        ix = 3;
        break;
      default:
        break;
      }
      TH2D* hCounters = (TH2D*) tin->getCountersHistogram();
      sum_wgts = hCounters->GetBinContent(ix, 1);
    }
    if (sum_wgts==0.){
      IVYerr << "Sum of pre-recorded weights cannot be zero." << endl;
      assert(0);
    }

    // Add the tree to the list of trees to process
    tinlist.push_back(tin);

    // Calculate the overall normalization scale on the events.
    // Includes xsec (in fb), lumi (in fb-1), and 1/sum of weights in all of the MC.
    // Data normalizaion factor is always 1.
    double norm_scale = (isData ? 1. : xsec * xsecScale * lumi)/sum_wgts;
    tin_normScale_map[tin] = norm_scale;
    IVYout << "Acquired a sum of weights of " << sum_wgts << ". Overall normalization will be " << norm_scale << "." << endl;

    nevents_total += tin->getNEvents();

    curdir->cd();

    // Book the necessary branches
    genInfoHandler.bookBranches(tin);
    simEventHandler.bookBranches(tin);
    eventFilter.bookBranches(tin);
    muonHandler.bookBranches(tin);
    electronHandler.bookBranches(tin);
    jetHandler.bookBranches(tin);
    isotrackHandler.bookBranches(tin);

    // Book a few additional branches
    tin->bookBranch<EventNumber_t>("event", 0);
    if (!isData){
      tin->bookBranch<float>("GenMET_pt", 0);
      tin->bookBranch<float>("GenMET_phi", 0);
    }
    else{
      tin->bookBranch<RunNumber_t>("run", 0);
      tin->bookBranch<LuminosityBlock_t>("luminosityBlock", 0);
    }
  }

  curdir->cd();

  /**************************/
  /* EVENT LOOP PREPARATION */
  /**************************/

  // Keep track of sums of weights
  SelectionTracker seltracker;

  // Keep track of the traversed events
  bool firstOutputEvent = true;
  int eventIndex_begin = -1;
  int eventIndex_end = -1;
  int eventIndex_tracker = 0;
  splitInputEventsIntoChunks((is_sim_data_flag==1), nevents_total, ichunk, nchunks, eventIndex_begin, eventIndex_end);

  for (auto const& tin:tinlist){
    if (SampleHelpers::doSignalInterrupt==1) break;

    auto const& norm_scale = tin_normScale_map.find(tin)->second;
    bool const isData = (is_sim_data_flag==1);

    // Wrap the ivies around the input tree:
    // Booking is basically SetBranchStatus+SetBranchAddress. You can book for as many trees as you would like.
    // In some cases, bookBranches also informs the ivy dynamically that it is supposed to consume certain entries.
    // For entries common to all years or any data or MC, the consumption information is handled in the ivy constructor already.
    // None of these mean the ivy establishes its access to the input tree yet.
    // Wrapping a tree informs the ivy that it is supposed to consume the booked entries from that particular tree.
    // Without wrapping, you are not really accessing the entries from the input tree to construct the physics objects;
    // all you would get are 0 electrons, 0 jets, everything failing event filters etc.
    genInfoHandler.wrapTree(tin);
    simEventHandler.wrapTree(tin);
    eventFilter.wrapTree(tin);
    muonHandler.wrapTree(tin);
    electronHandler.wrapTree(tin);
    jetHandler.wrapTree(tin);
    isotrackHandler.wrapTree(tin);

    // Explicit pointers to the entries in the trees
    RunNumber_t* ptr_RunNumber = nullptr;
    LuminosityBlock_t* ptr_LuminosityBlock = nullptr;
    EventNumber_t* ptr_EventNumber = nullptr;
    float* ptr_genmet_pt = nullptr;
    float* ptr_genmet_phi = nullptr;
    tin->getValRef("event", ptr_EventNumber);
    if (!isData){
      tin->getValRef("GenMET_pt", ptr_genmet_pt);
      tin->getValRef("GenMET_phi", ptr_genmet_phi);
    }
    else{
      tin->getValRef("run", ptr_RunNumber);
      tin->getValRef("luminosityBlock", ptr_LuminosityBlock);
    }

    /********************/
    /* BEGIN EVENT LOOP */
    /********************/

    unsigned int n_traversed = 0;
    unsigned int n_recorded = 0;
    unsigned int n_branched = 0;
    int nEntries = tin->getNEvents(); 
    IVYout << "Looping over " << nEntries << " events from " << tin->sampleIdentifier << "..." << endl;
    for (int ev=0; ev<nEntries; ev++){ 
      if (SampleHelpers::doSignalInterrupt==1) break;

      // Event accumulation in chunks:
      // In simulation, events are accumlated if the entry index falls into the range for the requested chunk (if nchunks<=0, all events are included).
      // In data, max(nchunks) = Number of run numbers. Events are split into chunks based on the ordered list of run numbers in the specified data era.
      bool doAccumulate = true;
      if (isData){
        if (eventIndex_begin>0 || eventIndex_end>0) doAccumulate = (
          tin->updateBranch(ev, "run", false)
          &&
          (eventIndex_begin<0 || (*ptr_RunNumber)>=static_cast<RunNumber_t>(eventIndex_begin))
          &&
          (eventIndex_end<0 || (*ptr_RunNumber)<=static_cast<RunNumber_t>(eventIndex_end))
          );
      }
      else doAccumulate = (
        (eventIndex_begin<0 || eventIndex_tracker>=static_cast<int>(eventIndex_begin))
        &&
        (eventIndex_end<0 || eventIndex_tracker<static_cast<int>(eventIndex_end))
        );

      eventIndex_tracker++;
      if (!doAccumulate) continue;

      tin->getEvent(ev);
      HelperFunctions::progressbar(ev, nEntries);
      n_traversed++;

      // Construct gen. info.
      genInfoHandler.constructGenInfo();
      auto const& genInfo = genInfoHandler.getGenInfo();
      auto const& genparticles = genInfoHandler.getGenParticles();

      std::vector<GenParticleObject*> genmatch_promptleptons;
      for (auto const& part:genparticles){
        if (part->status()!=1) continue; // Only final states
        if (!part->extras.isPromptFinalState) continue; // Only prompt final states
        bool const isLepton = std::abs(part->pdgId())==11 || std::abs(part->pdgId())==13;
        if (!isLepton) continue; // Only leptons
        genmatch_promptleptons.push_back(part);
      }

      // Construct sim. event info.
      // Motice both gen. info and sim. info can be constructed for data.
      // Internally, the function just returns true without doing anything else.
      simEventHandler.constructSimEvent();

      double wgt = 1;
      if (!isData){
        // Regular gen. weight
        double genwgt = 1;
        genwgt = genInfo->getGenWeight(theGlobalSyst);

        // PU reweighting (time-dependent)
        double puwgt = 1;
        puwgt = simEventHandler.getPileUpWeight(theGlobalSyst);

        wgt = genwgt * puwgt;

        // Add L1 prefiring weight for 2016 and 2017
        wgt *= simEventHandler.getL1PrefiringWeight(theGlobalSyst);
      }
      // Overall sample normalization
      wgt *= norm_scale;

      muonHandler.constructMuons();
      electronHandler.constructElectrons();

      // JetMET handler requires gen. and sim. info for JER and MET purposes
      jetHandler.constructJetMET(&genInfoHandler, &simEventHandler, theGlobalSyst);

      // !!!IMPORTANT!!!
      // NEVER USE LEPTONS AND JETS IN AN ANALYSIS BEFORE DISAMBIGUATING THEM!
      // Muon and electron handlers do not apply any selection, so the selection bits are all 0.
      // ParticleDisambiguator does the matching, and assigns the overlapping jets (or closest ones) as 'mothers' of the leptons.
      // That is required in TopLeptonMVA computations! That is why disambiguation comes before setting selection bits!
      // ParticleDisambiguator then cleans all geometrically overlapping jets by moving them to the ak4jets_masked collection of JetMETHandler.
      particleDisambiguator.disambiguateParticles(&muonHandler, &electronHandler, nullptr, &jetHandler);

      /***********/
      /* LEPTONS */
      /***********/

      // Get leptons and match them to gen. particles
      auto const& muons = muonHandler.getProducts();
      auto const& electrons = electronHandler.getProducts();

      // Make a map of reco. lepton -> gen. prompt lepton so that we can use it for ID scale factor purposes later on.
      // Note that there is no charge matchingm just delta R < 0.2.
      std::vector<ParticleObject*> leptons; leptons.reserve(muons.size()+electrons.size());
      for (auto const& part:muons) leptons.push_back(part);
      for (auto const& part:electrons) leptons.push_back(part);
      std::unordered_map<ParticleObject*, GenParticleObject*> lepton_genmatchpart_map;
      ParticleObjectHelpers::matchParticles( // MATCHING LEPTONS
        ParticleObjectHelpers::kMatchBy_DeltaR, 0.2,
	leptons.begin(), leptons.end(),
        genmatch_promptleptons.begin(), genmatch_promptleptons.end(),
        lepton_genmatchpart_map
      );
      

      // Keep track of leptons
      // ID can be any of the disjoint categories tight, fakeable (yes, not using 'fakable' correctly), or loose.
      // selected = tight or fakeable or loose.

      std::vector<MuonObject*> muons_selected;
      std::vector<MuonObject*> muons_tight;
      std::vector<MuonObject*> muons_fakeable;
      std::vector<MuonObject*> muons_loose;
      for (auto const& part:muons){
        if (ParticleSelectionHelpers::isTightParticle(part)) muons_tight.push_back(part);
        else if (ParticleSelectionHelpers::isFakeableParticle(part)) muons_fakeable.push_back(part);
        else if (ParticleSelectionHelpers::isLooseParticle(part)) muons_loose.push_back(part);
        /*
        // Here is how to get some other quantities. Commented out so that looper remains faster.
        float pt = part->pt();
        float eta = part->eta();
        float phi = part->phi();
        float mass = part->mass();

        float ptrel_final = part->ptrel();
        float ptratio_final = part->ptratio();

        // This is the TopLeptonMVA(v2) score
        float extMVAscore=-99;
        bool has_extMVAscore = part->getExternalMVAScore(MuonSelectionHelpers::selection_type, extMVAscore);

        float bscore = 0;
        AK4JetObject* mother = nullptr;
        for (auto const& mom:part->getMothers()){
          mother = dynamic_cast<AK4JetObject*>(mom);
          if (mother) break;
        }
        if (mother) bscore = mother->extras.btagDeepFlavB;
        */
      }
      HelperFunctions::appendVector(muons_selected, muons_tight);
      HelperFunctions::appendVector(muons_selected, muons_fakeable);
      HelperFunctions::appendVector(muons_selected, muons_loose);

      std::vector<ElectronObject*> electrons_selected;
      std::vector<ElectronObject*> electrons_tight;
      std::vector<ElectronObject*> electrons_fakeable;
      std::vector<ElectronObject*> electrons_loose;
      for (auto const& part:electrons){
        if (ParticleSelectionHelpers::isTightParticle(part)) electrons_tight.push_back(part);
        else if (ParticleSelectionHelpers::isFakeableParticle(part)) electrons_fakeable.push_back(part);
        else if (ParticleSelectionHelpers::isLooseParticle(part)) electrons_loose.push_back(part);
        /*
        // Here is how to get some other quantities. Commented out so that looper remains faster.
        float pt = part->pt();
        float eta = part->eta();
        float etaSC = part->etaSC();
        float phi = part->phi();
        float mass = part->mass();

        float mvaFall17V2noIso_raw = 0.5 * std::log((1. + part->extras.mvaFall17V2noIso)/(1. - part->extras.mvaFall17V2noIso));
        float ptrel_final = part->ptrel();
        float ptratio_final = part->ptratio();

        // This is the TopLeptonMVA(v2) score
        float extMVAscore=-99;
        bool has_extMVAscore = part->getExternalMVAScore(ElectronSelectionHelpers::selection_type, extMVAscore);

        // Get the b-tagging score of the mother jet.
        float bscore = 0;
        AK4JetObject* mother = nullptr;
        for (auto const& mom:part->getMothers()){
          mother = dynamic_cast<AK4JetObject*>(mom);
          if (mother) break;
        }
        if (mother) bscore = mother->extras.btagDeepFlavB;
        */
      }
      
      HelperFunctions::appendVector(electrons_selected, electrons_tight);
      HelperFunctions::appendVector(electrons_selected, electrons_fakeable);
      HelperFunctions::appendVector(electrons_selected, electrons_loose);

      unsigned int const nleptons_tight = muons_tight.size() + electrons_tight.size();
      unsigned int const nleptons_fakeable = muons_fakeable.size() + electrons_fakeable.size();
      unsigned int const nleptons_loose = muons_loose.size() + electrons_loose.size();
      unsigned int const nleptons_selected = nleptons_tight + nleptons_fakeable + nleptons_loose;

      // VECTORS FOR BRANCHES
      std::vector<ParticleObject*> leptons_selected; leptons_selected.reserve(nleptons_selected);
      for (auto const& part:muons_selected) leptons_selected.push_back(dynamic_cast<ParticleObject*>(part));
      for (auto const& part:electrons_selected) leptons_selected.push_back(dynamic_cast<ParticleObject*>(part));
      ParticleObjectHelpers::sortByGreaterPt(leptons_selected);

      std::vector<ParticleObject*> leptons_fakeable; leptons_fakeable.reserve(nleptons_fakeable);
      for (auto const& part:muons_fakeable) leptons_fakeable.push_back(dynamic_cast<ParticleObject*>(part));
      for (auto const& part:electrons_fakeable) leptons_fakeable.push_back(dynamic_cast<ParticleObject*>(part));
      ParticleObjectHelpers::sortByGreaterPt(leptons_fakeable);

      std::vector<ParticleObject*> leptons_tight; leptons_tight.reserve(nleptons_tight);
      for (auto const& part:muons_tight) leptons_tight.push_back(dynamic_cast<ParticleObject*>(part)); 
      for (auto const& part:electrons_tight) leptons_tight.push_back(dynamic_cast<ParticleObject*>(part));
      ParticleObjectHelpers::sortByGreaterPt(leptons_tight);

      /************/
      /* AK4 JETS */
      /************/
      double HT_ak4jets=0;
      auto const& ak4jets = jetHandler.getAK4Jets();
      std::vector<AK4JetObject*> ak4jets_tight_recordable;
      std::vector<AK4JetObject*> ak4jets_tight_selected;
      std::vector<AK4JetObject*> ak4jets_tight_selected_btagged;

      for (auto const& jet:ak4jets){
        float pt = jet->pt();
        float eta = jet->eta();
        float phi = jet->phi();
        float mass = jet->mass();

        bool is_tight = ParticleSelectionHelpers::isTightJet(jet);
        bool is_btagged = jet->testSelectionBit(bit_preselection_btag);

        if (is_tight && pt>=minpt_anyjet && std::abs(eta)<absEtaThr_ak4jets){
          bool recordJet = false;
          if (is_btagged && pt>=minpt_bjets){
            recordJet |= true;
            ak4jets_tight_selected_btagged.push_back(jet);
          }
          if (pt>=minpt_jets){
            recordJet |= true;
            ak4jets_tight_selected.push_back(jet);
            HT_ak4jets += pt;
          }
          if (recordJet) ak4jets_tight_recordable.push_back(jet);
        }
      }

      unsigned int const nak4jets_tight_selected = ak4jets_tight_selected.size();
      unsigned int const nak4jets_tight_selected_btagged = ak4jets_tight_selected_btagged.size();

      // MET is hopefully already calibrated at this point
      auto const& eventmet = jetHandler.getPFMET();

      // BEGIN PRESELECTION
      seltracker.accumulate("Full sample", wgt);

      int numJets = ak4jets_tight_recordable.size();
      seltracker.accumulate("nJets before preselections", numJets);
      //bool const pass_Nj_geq_2 = nak4jets_tight_selected>=2;
      //bool const pass_Nb_geq_2 = nak4jets_tight_selected_btagged>=2;
      //if (!(pass_Nj_geq_2 && pass_Nb_geq_2)) continue;
      //seltracker.accumulate("Pass Nj>=2 and Nb>=2", wgt);

      double const pTmiss = eventmet->pt();
      double const phimiss = eventmet->phi();

      //bool const pass_pTmiss = pTmiss>=minpt_miss;
      // if (!pass_pTmiss) continue;
      //seltracker.accumulate("Pass pTmiss", wgt);

      // bool const pass_HTjets = HT_ak4jets>=minHT_jets;
      // if (!pass_HTjets) continue;
      // seltracker.accumulate("Pass HT (necessary?)", wgt);

      bool const pass_Nleptons = (nleptons_tight == 2); // CHANGED, ONLY 2 leptons   nleptons_tight>=2 && nleptons_tight<5
      if (!pass_Nleptons) continue;
      seltracker.accumulate("Has ==2 tight leptons", wgt); // Has >=2 and <=4 tight leptons

      bool const pass_electrons = (abs(leptons_tight.front()->pdgId())==11 
				   && abs(leptons_tight.at(1)->pdgId())==11); // Added tight electrons only cut 
      if (!pass_electrons) continue;

      bool const pass_pTl1 = leptons_tight.front()->pt()>=25.;
      bool const pass_pTl2 = leptons_tight.at(1)->pt()>=20.;
      if (!(pass_pTl1 && pass_pTl2)) continue;
      seltracker.accumulate("Pass ptl1, ptl2 cuts", wgt);


      /////////DILEPTONS/////////      
      // Construct all possible dilepton pairs (only 1 for DY, here)
      // Note that loose leptons are included as well in the 'selected' collection
      // so that dilepton vetos can be done easily next.
      dileptonHandler.constructDileptons(&muons_selected, &electrons_selected);
      auto const& dileptons = dileptonHandler.getProducts();

      // Dilepton vetos
      bool fail_vetos = false;
      DileptonObject* dilepton_OS_ZCand_tight = nullptr;
      DileptonObject* dilepton_SS_ZCand_tight = nullptr;
      for (auto const& dilepton:dileptons){ // DILEPTON CUTS
        bool isSS = !dilepton->isOS();
        bool isTight = dilepton->nTightDaughters()==2;
        bool isSF = dilepton->isSF();
        bool is_LowMass = dilepton->m()<12.;
        bool is_ZClose = std::abs(dilepton->m()-91.2)<15.;
        bool is_DYClose = is_ZClose || is_LowMass;

	if (isSF && isTight && is_ZClose) {
	  if (isSS && !dilepton_SS_ZCand_tight) {dilepton_SS_ZCand_tight = dilepton;} // marking Z Boson candidates SS
	  if (!isSS && !dilepton_OS_ZCand_tight) {dilepton_OS_ZCand_tight = dilepton;} // marking Z Boson candidates SS
	  else {break;}		// don't veto
	}
	else {fail_vetos = true; break;} // Only one dilepton per event so the break is fine here.

      }	// end for dilepton:dileptons
      if (fail_vetos) continue;
      seltracker.accumulate("has ZCand dilepton", wgt);


      bool const has_dilepton_SS_ZCand_tight = (dilepton_SS_ZCand_tight!=nullptr);
      bool const has_dilepton_OS_ZCand_tight = (dilepton_OS_ZCand_tight!=nullptr);


      // Do not skip the event if there is an OS Z cand.
      // Instead, record a mass variable (=-1 if it doesn't exist).
      rcd_output.setNamedVal<float>("mass_OS_ZCand_tight", (has_dilepton_OS_ZCand_tight ? dilepton_OS_ZCand_tight->mass() : -1.f));

      // Do not skip the event if there is an SS Z cand.
      // Instead, record a mass variable (=-1 if it doesn't exist).

      rcd_output.setNamedVal<float>("mass_SS_ZCand_tight", (has_dilepton_SS_ZCand_tight ? dilepton_SS_ZCand_tight->mass() : -1.f));

      seltracker.accumulate("Has OS Z candidate", wgt*static_cast<double>(has_dilepton_OS_ZCand_tight));
      seltracker.accumulate("Has SS Z candidate", wgt*static_cast<double>(has_dilepton_SS_ZCand_tight));
      ///////END DILEPTONS///////      


      // Put event filters to the last because data has unique event tracking enabled.
      // Data event does not get the chance to be tracked until constructFilters is called.
      // Tracking costs time, so it is better to run it on the smallest possible set of events.
      eventFilter.constructFilters(&simEventHandler);

      // For now, we do not apply a HEM15/16 veto for 2018D and part of C. There are two ways to apply it: Only on jets, or on jets and electrons.
      // - HEM veto should be apply on jets since events where there is a jet in this region are known to have a biased (PF?)MET.
      // - If electron selection is loose, the HEM failure can also cause a hot spot in the relevant eta-phi region for electrons.
      // However, a veto on electrons needs to be checked carefully as it depends on the specifics of the ID.
      constexpr bool pass_HEMveto = true;
      //bool const pass_HEMveto = eventFilter.test2018HEMFilter(&simEventHandler, nullptr, nullptr, &ak4jets);
      //bool const pass_HEMveto = eventFilter.test2018HEMFilter(&simEventHandler, &electrons, nullptr, &ak4jets);
      if (!pass_HEMveto) continue;
      seltracker.accumulate("Pass HEM veto (High-Eta Muon)", wgt);

      // MET filters are regular filters from JetMET.
      bool const pass_METFilters = eventFilter.passMETFilters();
      if (!pass_METFilters) continue;
      seltracker.accumulate("Pass MET filters", wgt);

      // Test if the data event is unique (i.e., dorky). Does not do anything in the MC.
      // if (!eventFilter.isUniqueDataEvent()) continue;
      // seltracker.accumulate("Pass unique event check", wgt);

      // Triggers without HLT object matching
      float event_weight_triggers_dilepton = eventFilter.getTriggerWeight(hltnames_Dilepton);
      bool const pass_triggers_dilepton = (event_weight_triggers_dilepton!=0.);
      if (!pass_triggers_dilepton) continue; // Test if any triggers passed at all
      seltracker.accumulate("Pass any trigger", wgt);

      // Trigger with HLT object matching
      float event_weight_triggers_dilepton_matched = eventFilter.getTriggerWeight(
        triggerPropsCheckList_Dilepton,
        &muons, &electrons, nullptr, &ak4jets, nullptr, nullptr
      );
      bool const pass_triggers_dilepton_matched = (event_weight_triggers_dilepton_matched!=0.);
      // Do not skip the event. Instead, record a flag for HLT object matching.
      rcd_output.setNamedVal("pass_triggers_dilepton_matched", pass_triggers_dilepton_matched);
      seltracker.accumulate("Pass triggers after matching", wgt*static_cast<double>(pass_triggers_dilepton_matched));


      /*************************************************/
      /* NO MORE CALLS TO SELECTION BEYOND THIS POINT! */
      /*************************************************/
      if (tout){
        // Event weight
        rcd_output.setNamedVal("event_wgt", static_cast<float>(wgt));
        rcd_output.setNamedVal("EventNumber", *ptr_EventNumber);
	
        if (!isData){ // This one is triggered for charge flips
          rcd_output.setNamedVal("GenMET_pt", *ptr_genmet_pt);
          rcd_output.setNamedVal("GenMET_phi", *ptr_genmet_phi);

        }
        else{
          rcd_output.setNamedVal("RunNumber", *ptr_RunNumber); 
	  int runN = *ptr_RunNumber; // dereference ptr
	  output_csv << "Run Number: " << runN; // add to csv  

          rcd_output.setNamedVal("LuminosityBlock", *ptr_LuminosityBlock); // LUMI
	  double LumiVal;
	  output_csv << ",Lumi: " << LumiVal;
        }

        rcd_output.setNamedVal<float>("HT_ak4jets", HT_ak4jets);
        rcd_output.setNamedVal<float>("pTmiss", pTmiss);
        rcd_output.setNamedVal<float>("phimiss", phimiss);



	// Record SS electon-electron pairs
	{
	if (has_dilepton_SS_ZCand_tight) {
#define BRANCH_VECTOR_COMMANDS				    \
	  BRANCH_VECTOR_COMMAND(int, genmatch_leadingPdgId) \
	  BRANCH_VECTOR_COMMAND(int, genmatch_trailingPdgId)\
	  BRANCH_VECTOR_COMMAND(float, pt)	        \
	  BRANCH_VECTOR_COMMAND(float, leadingPt)	\
	  BRANCH_VECTOR_COMMAND(float, trailingPt)	\
	  BRANCH_VECTOR_COMMAND(float, leading_eta)	\
	  BRANCH_VECTOR_COMMAND(float, trailing_eta)	\
	  BRANCH_VECTOR_COMMAND(float, leading_phi)	\
	  BRANCH_VECTOR_COMMAND(float, trailing_phi)	\
	  BRANCH_VECTOR_COMMAND(int, leadingPdgId)	\
	  BRANCH_VECTOR_COMMAND(int, trailingPdgId)	\
	  BRANCH_VECTOR_COMMAND(int, nJets)		\
	  BRANCH_VECTOR_COMMAND(float, mass)

#define BRANCH_VECTOR_COMMAND(TYPE, NAME) std::vector<TYPE> ee_SS_##NAME;
          BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

          for (auto const& dilepton:dileptons) {
	      //----------------------------------------//
	      float pt1 = dilepton->getDaughter_leadingPt()->pt();
	      float pt2 = dilepton->getDaughter_subleadingPt()->pt();
	      float phi1 = dilepton->getDaughter_leadingPt()->phi();
	      float phi2 = dilepton->getDaughter_subleadingPt()->phi();
	      float pt = std::sqrt(std::pow((dilepton->p4().px()),2)
				   +std::pow((dilepton->p4().py()),2));
	      float mass         = dilepton_SS_ZCand_tight->mass();
	      float leadingPt    = dilepton->getDaughter_leadingPt()->pt();
	      float trailingPt   = dilepton->getDaughter_subleadingPt()->pt();
	      float leading_phi  = dilepton->getDaughter_leadingPt()->phi();
	      float trailing_phi = dilepton->getDaughter_subleadingPt()->phi();
	      float leading_eta  = dilepton->getDaughter_leadingPt()->eta();
	      float trailing_eta = dilepton->getDaughter_subleadingPt()->eta();
	      int nJets          = numJets; // no cuts on jets

	      //-- GEN MATCHING --//
	      int leadingPdgId;
	      int trailingPdgId;
	      int genmatch_leadingPdgId;
	      int genmatch_trailingPdgId;

	      auto it_genmatch1 = lepton_genmatchpart_map.find(dynamic_cast<ParticleObject*>(dilepton->getDaughter_leadingPt()));
	      bool is_genmatched_prompt1 = it_genmatch1!=lepton_genmatchpart_map.end() && it_genmatch1->second!=nullptr;
	      auto it_genmatch2 = lepton_genmatchpart_map.find(dynamic_cast<ParticleObject*>(dilepton->getDaughter_subleadingPt()));
	      bool is_genmatched_prompt2 = it_genmatch2!=lepton_genmatchpart_map.end() && it_genmatch2->second!=nullptr;

	      if (is_genmatched_prompt1 && is_genmatched_prompt2) {
	      	if (*ptr_EventNumber == 49216625) {std::cout << "event 49216625, inside genmatch cut. ";}
	      	leadingPdgId   = dilepton->getDaughter_leadingPt()->pdgId();		
	      	trailingPdgId  = dilepton->getDaughter_subleadingPt()->pdgId();
	      	genmatch_leadingPdgId = it_genmatch1->second->pdgId();		
	      	genmatch_trailingPdgId = it_genmatch2->second->pdgId();
	      }
		
	      // pretty sure they must both flip, or neither. But just in case, I'll use two separate ifs:
	      if (!is_genmatched_prompt1) {
	      	genmatch_leadingPdgId = 33;
	      	std::cout << "leading daughter, other: " << (*ptr_EventNumber) << endl;
	      }
	      if (!is_genmatched_prompt2) {
	      	genmatch_trailingPdgId = 33;
		std::cout << "trailing daughter, other: " << (*ptr_EventNumber) << endl;
	      }

	      n_branched++;
	      //-- END GEN MATCHING --//		
	      //----------------------------------------//	
#define BRANCH_VECTOR_COMMAND(TYPE, NAME) ee_SS_##NAME.push_back(NAME);
	      BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

	  } // end for dilepton:dileptons

#define BRANCH_VECTOR_COMMAND(TYPE, NAME) rcd_output.setNamedVal(Form("ee_SS_%s", #NAME), ee_SS_##NAME);
          BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

#undef BRANCH_VECTOR_COMMANDS
	} // end if SS_ZCand_tight
        }



	// Record OS electon-electron pairs
	{
	if (has_dilepton_OS_ZCand_tight) {
#define BRANCH_VECTOR_COMMANDS				    \
	  BRANCH_VECTOR_COMMAND(int, genmatch_leadingPdgId) \
	  BRANCH_VECTOR_COMMAND(int, genmatch_trailingPdgId)\
	  BRANCH_VECTOR_COMMAND(float, pt)	        \
	  BRANCH_VECTOR_COMMAND(float, leadingPt)	\
	  BRANCH_VECTOR_COMMAND(float, trailingPt)	\
	  BRANCH_VECTOR_COMMAND(float, leading_eta)	\
	  BRANCH_VECTOR_COMMAND(float, trailing_eta)	\
	  BRANCH_VECTOR_COMMAND(float, leading_phi)	\
	  BRANCH_VECTOR_COMMAND(float, trailing_phi)	\
	  BRANCH_VECTOR_COMMAND(int, leadingPdgId)	\
	  BRANCH_VECTOR_COMMAND(int, trailingPdgId)	\
	  BRANCH_VECTOR_COMMAND(int, nJets)		\
	  BRANCH_VECTOR_COMMAND(float, mass)

#define BRANCH_VECTOR_COMMAND(TYPE, NAME) std::vector<TYPE> ee_OS_##NAME;
          BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

          for (auto const& dilepton:dileptons) {
	      //----------------------------------------//
	      float pt1 = dilepton->getDaughter_leadingPt()->pt();
	      float pt2 = dilepton->getDaughter_subleadingPt()->pt();
	      float phi1 = dilepton->getDaughter_leadingPt()->phi();
	      float phi2 = dilepton->getDaughter_subleadingPt()->phi();
	      float pt = std::sqrt(std::pow((dilepton->p4().px()),2)
				   +std::pow((dilepton->p4().py()),2));
	      float mass         = dilepton_OS_ZCand_tight->mass();
	      float leadingPt    = dilepton->getDaughter_leadingPt()->pt();
	      float trailingPt   = dilepton->getDaughter_subleadingPt()->pt();
	      float leading_phi  = dilepton->getDaughter_leadingPt()->phi();
	      float trailing_phi = dilepton->getDaughter_subleadingPt()->phi();
	      float leading_eta  = dilepton->getDaughter_leadingPt()->eta();
	      float trailing_eta = dilepton->getDaughter_subleadingPt()->eta();
	      int nJets          = numJets; // no cuts on jets

	      //-- GEN MATCHING --//
	      int leadingPdgId;
	      int trailingPdgId;
	      int genmatch_leadingPdgId;
	      int genmatch_trailingPdgId;

	      auto it_genmatch1 = lepton_genmatchpart_map.find(dynamic_cast<ParticleObject*>(dilepton->getDaughter_leadingPt()));
	      bool is_genmatched_prompt1 = it_genmatch1!=lepton_genmatchpart_map.end() && it_genmatch1->second!=nullptr;
	      auto it_genmatch2 = lepton_genmatchpart_map.find(dynamic_cast<ParticleObject*>(dilepton->getDaughter_subleadingPt()));
	      bool is_genmatched_prompt2 = it_genmatch2!=lepton_genmatchpart_map.end() && it_genmatch2->second!=nullptr;

	      if (is_genmatched_prompt1 && is_genmatched_prompt2) {
	      	if (*ptr_EventNumber == 49216625) {std::cout << "event 49216625, inside genmatch cut. ";}
	      	leadingPdgId   = dilepton->getDaughter_leadingPt()->pdgId();		
	      	trailingPdgId  = dilepton->getDaughter_subleadingPt()->pdgId();
	      	genmatch_leadingPdgId = it_genmatch1->second->pdgId();		
	      	genmatch_trailingPdgId = it_genmatch2->second->pdgId();
	      }
		
	      // pretty sure they must both flip, or neither. But just in case, I'll use two separate ifs:
	      if (!is_genmatched_prompt1) {
	      	genmatch_leadingPdgId = 33;
	      	std::cout << "leading daughter, other: " << (*ptr_EventNumber) << endl;
	      }
	      if (!is_genmatched_prompt2) {
	      	genmatch_trailingPdgId = 33;
		std::cout << "trailing daughter, other: " << (*ptr_EventNumber) << endl;
	      }

	      n_branched++;
	      //-- END GEN MATCHING --//		
	      //----------------------------------------//	
#define BRANCH_VECTOR_COMMAND(TYPE, NAME) ee_OS_##NAME.push_back(NAME);
	      BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

	  } // end for dilepton:dileptons

#define BRANCH_VECTOR_COMMAND(TYPE, NAME) rcd_output.setNamedVal(Form("ee_OS_%s", #NAME), ee_OS_##NAME);
          BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

#undef BRANCH_VECTOR_COMMANDS
	} // end if OS_ZCand_tight
        }

  
	/*
	// Record SS electron-electron pairs
	{
	if (has_dilepton_SS_ZCand_tight) {
	  std::cout << "has SS, event number: " << (*ptr_EventNumber) << endl;
#define BRANCH_VECTOR_COMMANDS				    \
          BRANCH_VECTOR_COMMAND(int, genmatch_leadingPdgId) \
	  BRANCH_VECTOR_COMMAND(int, genmatch_trailingPdgId)\
	  BRANCH_VECTOR_COMMAND(float, pt)	        \
	  BRANCH_VECTOR_COMMAND(float, leadingPt)	\
	  BRANCH_VECTOR_COMMAND(float, trailingPt)	\
	  BRANCH_VECTOR_COMMAND(float, leading_eta)	\
	  BRANCH_VECTOR_COMMAND(float, trailing_eta)	\
	  BRANCH_VECTOR_COMMAND(float, leading_phi)	\
	  BRANCH_VECTOR_COMMAND(float, trailing_phi)	\
	  BRANCH_VECTOR_COMMAND(int, leadingPdgId)	\
	  BRANCH_VECTOR_COMMAND(int, trailingPdgId)	\
	  BRANCH_VECTOR_COMMAND(int, nJets)		\
	  BRANCH_VECTOR_COMMAND(float, mass)

#define BRANCH_VECTOR_COMMAND(TYPE, NAME) std::vector<TYPE> ee_SS_##NAME;
          BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

          for (auto const& dilepton:dileptons) {
	      //----------------------------------------//
	      float pt1 = dilepton->getDaughter_leadingPt()->pt();
	      float pt2 = dilepton->getDaughter_subleadingPt()->pt();
	      float phi1 = dilepton->getDaughter_leadingPt()->phi();
	      float phi2 = dilepton->getDaughter_subleadingPt()->phi();
	      float pt = std::sqrt(std::pow((dilepton->p4().px()),2)
				   +std::pow((dilepton->p4().py()),2));
	      float mass         = dilepton_SS_ZCand_tight->mass();
	      float leadingPt    = dilepton->getDaughter_leadingPt()->pt();
	      float trailingPt   = dilepton->getDaughter_subleadingPt()->pt();
	      float leading_phi  = dilepton->getDaughter_leadingPt()->phi();
	      float trailing_phi = dilepton->getDaughter_subleadingPt()->phi();
	      float leading_eta  = dilepton->getDaughter_leadingPt()->eta();
	      float trailing_eta = dilepton->getDaughter_subleadingPt()->eta();
	      int nJets          = numJets; // no cuts on jets

	      //-- GEN MATCHING --//
	      int leadingPdgId;
	      int trailingPdgId;
	      int genmatch_leadingPdgId;
	      int genmatch_trailingPdgId;


	      auto it_genmatch1 = lepton_genmatchpart_map.find(dynamic_cast<ParticleObject*>(dilepton->getDaughter_leadingPt()));
	      bool is_genmatched_prompt1 = it_genmatch1!=lepton_genmatchpart_map.end() && it_genmatch1->second!=nullptr;
	      auto it_genmatch2 = lepton_genmatchpart_map.find(dynamic_cast<ParticleObject*>(dilepton->getDaughter_subleadingPt()));
	      bool is_genmatched_prompt2 = it_genmatch2!=lepton_genmatchpart_map.end() && it_genmatch2->second!=nullptr;
	      std::cout << "has SS, event number: " << (*ptr_EventNumber) << endl;
	      // separate ifs since SS will have just one charge flip
	      if (is_genmatched_prompt1) { 
		leadingPdgId = dilepton->getDaughter_leadingPt()->pdgId();		
		genmatch_leadingPdgId = it_genmatch1->second->pdgId();		
	      }
	      if (is_genmatched_prompt2) {
		trailingPdgId = dilepton->getDaughter_subleadingPt()->pdgId();
		genmatch_trailingPdgId = it_genmatch2->second->pdgId();
	      }
	      if (!is_genmatched_prompt1) {
	      	genmatch_leadingPdgId = 33;
	      	std::cout << "leading daughter, other: " << (*ptr_EventNumber) << endl;
	      }
	      if (!is_genmatched_prompt2) {
	      	genmatch_trailingPdgId = 33;
		std::cout << "trailing daughter, other: " << (*ptr_EventNumber) << endl;
	      }
	  
	      n_branched++;
	      //-- END GEN MATCHING --//		
	      //----------------------------------------//	
#define BRANCH_VECTOR_COMMAND(TYPE, NAME) ee_SS_##NAME.push_back(NAME);
	      BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

	  } // end for dilepton:dileptons

#define BRANCH_VECTOR_COMMAND(TYPE, NAME) rcd_output.setNamedVal(Form("ee_SS_%s", #NAME), ee_SS_##NAME);
          BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

#undef BRANCH_VECTOR_COMMANDS
	} // end if SS_ZCand_tight
        }
	*/

        // Record ak4jets
        {
#define BRANCH_VECTOR_COMMANDS \
          BRANCH_VECTOR_COMMAND(bool, pass_btagging_selection)	 \
          BRANCH_VECTOR_COMMAND(bool, pass_regularJet_selection) \
          BRANCH_VECTOR_COMMAND(int, hadronFlavour) \
          BRANCH_VECTOR_COMMAND(float, pt) \
          BRANCH_VECTOR_COMMAND(float, eta) \
          BRANCH_VECTOR_COMMAND(float, phi) \
          BRANCH_VECTOR_COMMAND(float, mass)
#define BRANCH_VECTOR_COMMAND(TYPE, NAME) std::vector<TYPE> ak4jets_##NAME;
          BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

          for (auto const& jet:ak4jets_tight_recordable){
            bool pass_btagging_selection = HelperFunctions::checkListVariable(ak4jets_tight_selected_btagged, jet);
            bool pass_regularJet_selection = HelperFunctions::checkListVariable(ak4jets_tight_selected, jet);
            auto const& hadronFlavour = jet->extras.hadronFlavour;
            float pt = jet->pt();
            float eta = jet->eta();
            float phi = jet->phi();
            float mass = jet->mass();

#define BRANCH_VECTOR_COMMAND(TYPE, NAME) ak4jets_##NAME.push_back(NAME);
            BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND
          }

#define BRANCH_VECTOR_COMMAND(TYPE, NAME) rcd_output.setNamedVal(Form("ak4jets_%s", #NAME), ak4jets_##NAME);
          BRANCH_VECTOR_COMMANDS;
#undef BRANCH_VECTOR_COMMAND

#undef BRANCH_VECTOR_COMMANDS
        }



        if (firstOutputEvent){
#define SIMPLE_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.named##name_t##s.begin(); itb!=rcd_output.named##name_t##s.end(); itb++) tout->putBranch(itb->first, itb->second);
#define VECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.namedV##name_t##s.begin(); itb!=rcd_output.namedV##name_t##s.end(); itb++) tout->putBranch(itb->first, &(itb->second));
#define DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.namedVV##name_t##s.begin(); itb!=rcd_output.namedVV##name_t##s.end(); itb++) tout->putBranch(itb->first, &(itb->second));
          SIMPLE_DATA_OUTPUT_DIRECTIVES;
          VECTOR_DATA_OUTPUT_DIRECTIVES;
          DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVES;
#undef SIMPLE_DATA_OUTPUT_DIRECTIVE
#undef VECTOR_DATA_OUTPUT_DIRECTIVE
#undef DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE
        }
#define SIMPLE_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.named##name_t##s.begin(); itb!=rcd_output.named##name_t##s.end(); itb++) tout->setVal(itb->first, itb->second);
#define VECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.namedV##name_t##s.begin(); itb!=rcd_output.namedV##name_t##s.end(); itb++) tout->setVal(itb->first, &(itb->second));
#define DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE(name_t, type) for (auto itb=rcd_output.namedVV##name_t##s.begin(); itb!=rcd_output.namedVV##name_t##s.end(); itb++) tout->setVal(itb->first, &(itb->second));
        SIMPLE_DATA_OUTPUT_DIRECTIVES;
        VECTOR_DATA_OUTPUT_DIRECTIVES;
        DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVES;
#undef SIMPLE_DATA_OUTPUT_DIRECTIVE
#undef VECTOR_DATA_OUTPUT_DIRECTIVE
#undef DOUBLEVECTOR_DATA_OUTPUT_DIRECTIVE

        tout->fill();		// output TTree
      }

      n_recorded++;

      if (firstOutputEvent) firstOutputEvent = false;


      if (*ptr_EventNumber == 49216625) {std::cout << "event 49216625, just before output_csv. ";} // one of 48 valid events that were previously getting dropped

      int EventN = *ptr_EventNumber; // dereference ptr
      output_csv << EventN; // addto csv Event Number: 
      output_csv << "," << electrons_tight.size(); // NtightEle: 
      output_csv << "," << muons_tight.size();	   // NtightMu: 
      output_csv << "," << has_dilepton_OS_ZCand_tight;  // has OS: 
      output_csv << "," << ak4jets_tight_selected.size(); // Njets: 
      output_csv << "," << ak4jets_tight_selected_btagged.size(); // Nbjets: 
      output_csv << "," << (has_dilepton_OS_ZCand_tight || has_dilepton_SS_ZCand_tight); 
      output_csv << endl;
    } // end ev entries loop

    IVYout << "Number of events recorded: " << n_recorded << " / " << n_traversed << " / " << nEntries << endl;
    nevents_total_traversed += n_traversed;
    
    IVYout << "Number of events added to branches: " << n_branched << endl;
  }
  seltracker.print();

  if (tout){
    tout->writeToFile(foutput);
    delete tout;
  }
  foutput->Close();

  curdir->cd();
  for (auto& tin:tinlist) delete tin;;

  // Split large files, and add them to the transfer queue from Condor to the target site
  // Does nothing if you are running the program locally because your output is already in the desired location.
  SampleHelpers::splitFileAndAddForTransfer(stroutput);
  SampleHelpers::splitFileAndAddForTransfer(stroutput_csv);

  // Close the output and error log files (and csv file)
  IVYout.close();
  IVYerr.close();
  output_csv.close();

  return 0;
} // end ScanChain


/*
main function:
Type help for the most up-to-date list of arguments and their descriptions.
Any argument that is a column name in the csv files inside the test/ directory must be passed even if you do not use them.
This is to ensure that batch submission using configureFTAnalysisJobs.py works as intended.
In addition, the arguments 'input_tag' and 'output_tag' must be present either.
The rest of the arguments here are up to the user to decide or add more onto.
Any unknown argument causes the script to return a non-zero return value in the current implementation.
*/
int main(int argc, char** argv){
  // argv[0]==[Executable name]
  constexpr int iarg_offset=1;

  // Switches that do not need =true.
  std::vector<std::string> const extra_argument_flags{
    "shorthand_Run2_UL_proposal_config"
      };

  bool print_help=false, has_help=false;
  int ichunk=0, nchunks=0;
  std::string str_dset;
  std::string str_proc;
  std::string str_period;
  std::string str_tag;
  std::string str_outtag;
  SimpleEntry extra_arguments;
  double xsec = -1;
  for (int iarg=iarg_offset; iarg<argc; iarg++){
    std::string strarg = argv[iarg];
    std::string wish, value;
    HelperFunctions::splitOption(strarg, wish, value, '=');

    if (wish.empty()){
      // Usually, one would run '[executable] [flag] [argument]=[value]'.
      // splitOption with a delimiter '=' records wish=[argument] and value=[value].
      // However, for [flag] arguments (those without specifying '=true' explicitly), wish="" and value=[flag].
      // For those possible flags, we make sure that value is read instead of wish.
      if (value=="help"){ print_help=has_help=true; }
      else if (HelperFunctions::checkListVariable(extra_argument_flags, value)) extra_arguments.setNamedVal<bool>(value, true);
      else{
        IVYerr << "ERROR: Unknown argument " << value << endl;
        print_help=true;
      }
    }
    else if (HelperFunctions::checkListVariable(extra_argument_flags, wish)){
      // Case where the user runs '[executable] [flag]=true/false' as a variation of the above.
      bool tmpval;
      HelperFunctions::castStringToValue(value, tmpval);
      extra_arguments.setNamedVal(wish, tmpval);
    }
    else if (wish=="dataset") str_dset = value;
    else if (wish=="short_name") str_proc = value;
    else if (wish=="period") str_period = value;
    else if (wish=="input_tag") str_tag = value;
    else if (wish=="output_tag") str_outtag = value;
    else if (wish=="input_files"){
      if (value.find("/")!=std::string::npos){
        IVYerr << "ERROR: Input file specification cannot contain directory structure." << endl;
        print_help=true;
      }
      extra_arguments.setNamedVal(wish, value);
    }
    else if (wish=="muon_id" || wish=="electron_id" || wish=="btag" || wish=="output_file") extra_arguments.setNamedVal(wish, value);
    else if (wish=="xsec"){
      if (xsec<0.) xsec = 1;
      xsec *= std::stod(value);
    }
    else if (wish=="BR"){
      if (xsec<0.) xsec = 1;
      xsec *= std::stod(value);
    }
    else if (wish=="nchunks") nchunks = std::stoi(value);
    else if (wish=="ichunk") ichunk = std::stoi(value);
    else if (wish=="minpt_jets" || wish=="minpt_bjets" || wish=="minpt_l3" || wish=="minpt_miss" || wish=="minHT_jets"){
      double tmpval;
      HelperFunctions::castStringToValue(value, tmpval);
      extra_arguments.setNamedVal(wish, tmpval);
    }
    else{
      IVYerr << "ERROR: Unknown argument " << wish << "=" << value << endl;
      print_help=true;
    }
  }

  if (!print_help && (str_proc=="" || str_dset=="" || str_period=="" || str_tag=="" || str_outtag=="")){
    IVYerr << "ERROR: Not all mandatory inputs are present." << endl;
    print_help=true;
  }
  if (!print_help && (nchunks>0 && (ichunk<0 || ichunk>=nchunks))){
    IVYerr << "ERROR: Invalid arguments." << endl;
    print_help=true;
  }

  // Ensure that nchunks and ichunk make sense.
  if (nchunks<0) nchunks = 0;
  if (nchunks<=0) ichunk = 0;

  if (print_help){
    IVYout << argv[0] << " options:\n\n";
    IVYout << "- help: Prints this help message.\n";
    IVYout << "- dataset: Data set name. Mandatory.\n";
    IVYout << "- short_name: Process short name. Mandatory.\n";
    IVYout << "- period: Data period. Mandatory.\n";
    IVYout << "- input_tag: Version of the input skims. Mandatory.\n";
    IVYout << "- output_tag: Version of the output. Mandatory.\n";
    IVYout << "- xsec: Cross section value. Mandatory in the MC.\n";
    IVYout << "- BR: BR value. Mandatory in the MC.\n";
    IVYout << "- nchunks: Number of splits of the input. Optional. Input splitting is activated only if nchunks>0.\n";
    IVYout << "- ichunk: Index of split input split to run. Optional. The index should be in the range [0, nchunks-1].\n";
    IVYout
      << "- output_file: Identifier of the output file if different from 'short_name'. Optional.\n"
      << "  The full output file name is '[identifier](_[ichunk]_[nchunks]).root.'\n";
    IVYout << "- input_files: Input files to run. Optional. Default is to run on all files.\n";
    IVYout
      << "- muon_id: Can be 'Cutbased_Run2', 'TopMVA_Run2', or 'TopMVAv2_Run2'.\n"
      << "  Default is whatever is in MuonSelectionHelpers (currently 'Cutbased_Run2') if no value is given.\n";
    IVYout
      << "- electron_id: Can be 'Cutbased_Run2', 'TopMVA_Run2', or 'TopMVAv2_Run2'.\n"
      << "  Default is whatever is in ElectronSelectionHelpers (currently 'Cutbased_Run2') if no value is given.\n";
    IVYout << "- btag: Name of the b-tagging WP. Can be 'medium' or 'loose' (case-insensitive). Default='medium'.\n";
    IVYout << "- minpt_jets: Minimum pT of jets in units of GeV. Default=40.\n";
    IVYout << "- minpt_bjets: Minimum pT of b-tagged jets in units of GeV. Default=25.\n";
    IVYout << "- minpt_l3: Minimum pT of third lepton in units of GeV. Default=20.\n";
    IVYout << "- minpt_miss: Minimum pTmiss in units of GeV. Default=50.\n";
    IVYout << "- minHT_jets: Minimum HT over ak4 jets in units of GeV. Default=300.\n";
    IVYout
      << "- shorthand_Run2_UL_proposal_config: Shorthand flag for the switches for the Run 2 UL analysis proposal:\n"
      << "  * muon_id='TopMVA_Run2'\n"
      << "  * electron_id='TopMVA_Run2'\n"
      << "  * btag='loose'\n"
      << "  * minpt_jets=25\n"
      << "  * minpt_bjets=25\n"
      << "  * minpt_l3=10\n"
      << "  The use of this shorthand will ignore the user-defined setting of these options above.\n";
    IVYout << endl;
    return (has_help ? 0 : 1);
  }

  // A universal configuration to set the samples directory and tag, on which T2 site they are, and at which data period we are looking.
  SampleHelpers::configure(str_period, Form("skims:%s", str_tag.data()), HostHelpers::kUCSDT2);



  // Run the actual program
  return ScanChain(str_outtag, str_dset, str_proc, xsec, ichunk, nchunks, extra_arguments);
}
