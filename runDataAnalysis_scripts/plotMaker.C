// Makes root hists out of test_looper's output .root file. Each lepton vector has 2 entries, which are sorted leading,trailing.

// maybe Electron_tightCharge is the trueBranch I need???

using namespace std;

void DrawBranchToHist(string inFileName, string outFileName, string branchName,int nbins, float xMin, float xMax, string XaxisTitle) {

  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  vector<float> *branch = 0;// initialize branch
  vector<int> *pdgID1_branch = 0; // get pdgIDs for filtering
  vector<int> *pdgID2_branch = 0;

  // Link branch variable address with Tree's branch
  SkimTree->SetBranchAddress(branchName.c_str(), &branch);
  SkimTree->SetBranchAddress("ee_leadingPdgId", &pdgID1_branch);
  SkimTree->SetBranchAddress("ee_trailingPdgId", &pdgID2_branch);
  
  std::string strhistOS = "OS_" + branchName;
  std::string strhistSS = "SS_" + branchName;
  TH1F *histOS = new TH1F(strhistOS.c_str(), strhistOS.c_str(), nbins, xMin, xMax);
  TH1F *histSS = new TH1F(strhistSS.c_str(), strhistSS.c_str(), nbins, xMin, xMax);

  // loop to fill hist w/ branch's values
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < branch->size(); j++) {
      if ((*pdgID1_branch)[j] * (*pdgID2_branch)[j] < 0) {
	histOS->Fill((*branch)[j]);
      }
      if ((*pdgID1_branch)[j] * (*pdgID2_branch)[j] > 0) {
	histSS->Fill((*branch)[j]);
      }
    }// end for j
  }// end for i
  
  // draw histOS and write to output file:
  histOS->Draw();
  histOS->GetXaxis()->SetTitle(XaxisTitle.c_str());
  histSS->Draw();
  histSS->GetXaxis()->SetTitle(XaxisTitle.c_str());

  // Create output file
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*",strhistOS.c_str())); // Delete any existing hists with the same name
  outFile->Delete(Form("%s;*",strhistSS.c_str())); // Delete any existing hists with the same name
  histOS->Write();
  histSS->Write();
  outFile->Close();
} // end DrawBranchToHist

void make2Dhists(string inFileName, string outFileName) {
  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile

  TH2I *histOS = new TH2I("ee_OS_truthMatch", "Truth-matched ee (OS)", 25, -12, 13, 25, -12, 34);
  TH2I *histSS = new TH2I("ee_SS_truthMatch", "Truth-matched ee (SS)", 25, -12, 13, 25, -12, 34);

  vector<int> *pdgID1_branch = 0;
  vector<int> *pdgID2_branch = 0;
  vector<int> *branch1 = 0;
  vector<int> *branch2 = 0;

  SkimTree->SetBranchAddress("ee_genmatch_leadingPdgId", &branch1);
  SkimTree->SetBranchAddress("ee_genmatch_trailingPdgId", &branch2);
  SkimTree->SetBranchAddress("ee_leadingPdgId", &pdgID1_branch);
  SkimTree->SetBranchAddress("ee_trailingPdgId", &pdgID2_branch);


  // loop to fill hists w/ branch's values
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < branch1->size(); j++) {
      if ((*pdgID1_branch)[j] * (*pdgID2_branch)[j] < 0) { // filter OS
	histOS->Fill((*pdgID1_branch)[j],(*branch1)[j]);
	histOS->Fill((*pdgID2_branch)[j],(*branch2)[j]);
      }
      if ((*pdgID1_branch)[j] * (*pdgID2_branch)[j] > 0) { // filter SS
	histSS->Fill((*pdgID1_branch)[j],(*branch1)[j]);
	histSS->Fill((*pdgID2_branch)[j],(*branch2)[j]);
      }
    } // end for j
  }   // end for i
  // draw hists and write to output file:
  histOS->GetXaxis()->SetTitle("Reconstructed pdgID");
  histOS->GetYaxis()->SetTitle("Genmatched pdgID");
  histOS->SetMarkerSize(1.8);
  histOS->Draw("TEXT");
  histSS->GetXaxis()->SetTitle("Reconstructed pdgID");
  histSS->GetYaxis()->SetTitle("Genmatched pdgID");
  histSS->SetMarkerSize(1.8);
  histSS->Draw("TEXT");
  // Create output file
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*","ee_OS_truthMatch")); // Delete any existing histograms with the same name
  outFile->Delete(Form("%s;*","ee_SS_truthMatch")); // Delete any existing histograms with the same name
  histOS->Write();
  histSS->Write();
  outFile->Close();
} // end make2Dhists



int plotMaker() {
  gROOT->SetBatch(kTRUE); // prevent auto-opening of last drawn histogram
  string inFile = "/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.root";
  string outFile = "/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_plotMaker.root";
  string inFile2 = "/ceph/cms/store/group/tttt/Skims/230105/2018/DY_2l_M_50/DY_2l_M_50_1.root";

  DrawBranchToHist(inFile, outFile, "ee_pt", 50,0,200, "pT (GeV/c)");
  DrawBranchToHist(inFile, outFile, "ee_leadingPt", 50,0,200, "pT (GeV/c)");
  DrawBranchToHist(inFile, outFile, "ee_trailingPt", 50,0,200, "pT (GeV/c)");
  DrawBranchToHist(inFile, outFile, "ee_leading_eta", 50,-3,3, "eta");
  DrawBranchToHist(inFile, outFile, "ee_trailing_eta", 50,-3,3, "eta");
  DrawBranchToHist(inFile, outFile, "ee_leading_phi", 100,-4,4, "phi (rad)");
  DrawBranchToHist(inFile, outFile, "ee_trailing_phi", 100,-4,4, "phi (rad)");
  DrawBranchToHist(inFile, outFile, "ee_mass", 50,75,105, "mass (GeV/c^2)");
  DrawBranchToHist(inFile, outFile, "ee_nJets", 15,0,15, "# jets / event");

  make2Dhists(inFile, outFile);
  return 0;
}
