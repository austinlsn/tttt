// Makes root hists out of test_looper's output .root file. Each lepton vector has 2 entries, which are sorted leading,trailing.

// True data has branch called "nJet" with type: "UInt_t slimmedJets, i.e. ak4 PFJets CHS with JECs applied, after basic selection (pt > 15)*"

// maybe Electron_tightCharge is the trueBranch I need???

using namespace std;

void DrawBranchToHist(string inFileName, string outFileName, string branchName, int nbins, float xMin, float xMax) {
  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");

  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  vector<float> *branch = 0;	// initialize branch
  vector<float> *pdgID_branch = 0; // init pdgbranch to filter muons

  // Link branch variable address with Tree's branch
  SkimTree->SetBranchAddress(branchName.c_str(), &branch);
  SkimTree->SetBranchAddress("leptons_pdgId", &pdgID_branch);

  TH1F *hist = new TH1F(branchName.c_str(), branchName.c_str(), nbins, xMin, xMax);

  // loop to fill hist w/ branch's values
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < branch->size(); j++) { // change to even/odd increments to select only leading/trailing
      //      if (branch->size() <= 2) { // dileptons only
      //if (abs((*pdgID_branch)[j]) == 11) { // only electrons in hist
	  hist->Fill((*branch)[j]);
	  //} // end if dileptons only
	  //}	// end if electrons only
    }	// end for j
  }	// end for i

  // draw hist and write to output file:
  hist->Draw();
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*",branchName.c_str())); // Delete any existing histograms with the same name
  hist->Write();
  outFile->Close();
}

/*
void drawExtraHists(string inFile1Name, string inFile2Name, string outFileName) {
  // Open input .root file
  TFile *f1 = new TFile(inFile1Name.c_str(), "READ");
  TFile *f2 = new TFile(inFile2Name.c_str(), "READ");

  TTree *tree1 = (TTree*)f1->Get("SkimTree"); // Get Tree from inFile1
  TTree *tree2 = (TTree*)f2->Get("Events"); // Get Tree from inFile2

  vector<int> *branch1 = 0;	// initialize branch from file1
  UInt_t nJet = 0;	// initialize branch from file2  

  // Link branch variable address with Tree's branch
  //  tree1->SetBranchAddress(branch1Name.c_str(), &branch1);
  tree2->SetBranchAddress("nJet", &nJet);

  TH1I *nJet_hist = new TH1I("nJet", "nJet", 100, -1, 20);

  // Loop over nJet branch to fill hist:
  for (unsigned int i = 0; i < tree2->GetEntries(); i++) {
    tree2->GetEntry(i);
    nJet_hist->Fill((nJet));
    // for (unsigned int j = 0; j < nJet->size(); j++) {

    // }	// end for j
  }	// end for i


  // draw hist and write to output file:
  nJet_hist->Draw();
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*","nJet_hist")); // Delete any existing histograms with the same name
  nJet_hist->Write();
  outFile->Close();
}
*/

int plotMaker() {
  gROOT->SetBatch(kTRUE); // prevent auto-opening of last hist
  string inFile = "/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.root";
  string inFile2 = "/ceph/cms/store/group/tttt/Skims/230105/2018/DY_2l_M_50/DY_2l_M_50_1.root";
  string outFile = "/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_plotMaker.root";

  // 6 args: strInFile, strOutFile, strBranchName (only takes vectors)
  //, nbins, xMin, xMax
  DrawBranchToHist(inFile, outFile, "dileptons_pt", 50,0,200);
  DrawBranchToHist(inFile, outFile, "leptons_eta", 50,-3,3);
  DrawBranchToHist(inFile, outFile, "leptons_phi", 100,-4,4);
  DrawBranchToHist(inFile, outFile, "dileptons_mass", 50,-2,100);
  //drawExtraHists(inFile, inFile2, outFile);

  return 0;
}
