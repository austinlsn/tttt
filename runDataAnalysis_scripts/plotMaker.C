// Makes root hists out of test_looper's output .root file. Each lepton vector has 2 entries, which are sorted leading,trailing.

// maybe Electron_tightCharge is the trueBranch I need???

using namespace std;

void DrawBranchToHist(string inFileName, string outFileName, string branchName,int nbins, float xMin, float xMax, string XaxisTitle) {

  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  vector<float> *branch = 0;	// initialize branch

  // Link branch variable address with Tree's branch
  SkimTree->SetBranchAddress(branchName.c_str(), &branch);
  
  TH1F *hist = new TH1F(branchName.c_str(), branchName.c_str(), nbins, xMin, xMax);
    // loop to fill hist w/ branch's values
    for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
      SkimTree->GetEntry(i);
	
      for (unsigned int j = 0; j < branch->size(); j++) {
	hist->Fill((*branch)[j]);
      }	// end for j
    }	// end for i

    // draw hist and write to output file:
    hist->Draw();
    hist->GetXaxis()->SetTitle(XaxisTitle.c_str());
    // Create output file
    TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
    outFile->Delete(Form("%s;*",branchName.c_str())); // Delete any existing histograms with the same name
    hist->Write();
    outFile->Close();
} // end DrawBranchToHist

void make2Dhist(string inFileName, string outFileName) {
  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile

  vector<int> *pdgID1_branch = 0; // init pdgbranch to filter muons
  vector<int> *pdgID2_branch = 0;
  vector<int> *branch1 = 0;
  vector<int> *branch2 = 0;

  SkimTree->SetBranchAddress("ee_genmatch_leadingPdgId", &branch1);
  SkimTree->SetBranchAddress("ee_genmatch_trailingPdgId", &branch2);
  SkimTree->SetBranchAddress("ee_leadingPdgId", &pdgID1_branch);
  SkimTree->SetBranchAddress("ee_trailingPdgId", &pdgID2_branch);
  TH2I *hist = new TH2I("ee_truthMatch", "Truth-matched ee, sim vs reco", 25, -12, 13, 25, -12, 34);

  // loop to fill hist w/ branch's values
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < branch1->size(); j++) {
      hist->Fill((*pdgID1_branch)[j],(*branch1)[j]);
      hist->Fill((*pdgID2_branch)[j],(*branch2)[j]);
    } // end for j
  }   // end for i
  // draw hist and write to output file:
  hist->GetXaxis()->SetTitle("Reconstructed pdgID");
  hist->GetYaxis()->SetTitle("Genmatched pdgID");
  hist->Draw("TEXT");
  // Create output file
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*","ee_truthMatch")); // Delete any existing histograms with the same name
  hist->Write();
  outFile->Close();
} // end make2Dhist

int plotMaker() {
  gROOT->SetBatch(kTRUE); // prevent auto-opening of last hist
  string inFile = "/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.root";
  string outFile = "/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_plotMaker.root";
  string inFile2 = "/ceph/cms/store/group/tttt/Skims/230105/2018/DY_2l_M_50/DY_2l_M_50_1.root";


  // 7 args: strInFile, strOutFile, strBranchName (only takes vectors)
  //, bool do2Dhist, nbins, xMin, xMax

  DrawBranchToHist(inFile, outFile, "ee_pt", 50,0,200, "pT (GeV/c)");
  DrawBranchToHist(inFile, outFile, "ee_leadingPt", 50,0,200, "pT (GeV/c)");
  DrawBranchToHist(inFile, outFile, "ee_trailingPt", 50,0,200, "pT (GeV/c)");
  DrawBranchToHist(inFile, outFile, "ee_leading_eta", 50,-3,3, "eta");
  DrawBranchToHist(inFile, outFile, "ee_trailing_eta", 50,-3,3, "eta");
  DrawBranchToHist(inFile, outFile, "ee_leading_phi", 100,-4,4, "phi (rad)");
  DrawBranchToHist(inFile, outFile, "ee_trailing_phi", 100,-4,4, "phi (rad)");
  DrawBranchToHist(inFile, outFile, "ee_mass", 50,75,105, "mass (GeV/c^2)");
  DrawBranchToHist(inFile, outFile, "ee_nJets", 15,0,15, "# jets / event");

  make2Dhist(inFile, outFile); // do2D hist on genmatching
  return 0;
}
