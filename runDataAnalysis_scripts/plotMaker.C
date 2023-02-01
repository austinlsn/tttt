// Makes root hists out of test_looper's output .root file. Each lepton vector has 2 entries, which are sorted leading,trailing.

// maybe Electron_tightCharge is the trueBranch I need???

using namespace std;

void DrawBranchToHist(string inFileName, string outFileName, string branchName, int leadingTrailing , int nbins, float xMin, float xMax) {
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
      if (branch->size() <= 2) { // dileptons only
	if (abs((*pdgID_branch)[j]) == 11) { // only electrons in hist
	  hist->Fill((*branch)[j]);
	} // end if dileptons only
      }	// end if electrons only
    }	// end for j
  }	// end for i

  // draw hist and write to output file:
  hist->Draw();
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*",branchName.c_str())); // Delete any existing histograms with the same name
  hist->Write();
  outFile->Close();
}

// void DrawBranchTo2DHist(string inFile1Name, string inFile2Name, string outFileName, string branch1Name, string branch2Name, int nbins1, float xMin1, float xMax1, int nbins2, float xMin2, float xMax2) {
//   // Create the 2D truth-matched hist:
//   // Open input .root file
//   TFile *f1 = new TFile(inFile1Name.c_str(), "READ");
//   TFile *f2 = new TFile(inFile2Name.c_str(), "READ");

//   TTree *tree1 = (TTree*)f1->Get("SkimTree"); // Get Tree from inFile1
//   TTree *tree2 = (TTree*)f2->Get("Events"); // Get Tree from inFile2

//   vector<float> *branch1 = 0;	// initialize branch from file1
//   int *branch2 = 0;	// initialize branch from file2
//   vector<float> *pdgID_branch = 0; // init pdgbranch to filter muons

//   // Link branch variable address with Tree's branch
//   tree1->SetBranchAddress(branch1Name.c_str(), &branch1);
//   tree1->SetBranchAddress("leptons_pdgId", &pdgID_branch);
//   tree2->SetBranchAddress(branch2Name.c_str(), &branch2);

//   TH2I *hist = new TH2I(branch2Name.c_str(), branch2Name.c_str(), nbins1, xMin1, xMax1, nbins2, xMin2, xMax2);

//   // Get entries from trueBranch in File2:
//   int branch2Entries[branch2->size()];
//   for (unsigned int i = 0; i < tree2->GetEntries(); i++) {
//     tree2->GetEntry(i);

//     for (unsigned int j = 0; j < branch2->size(); j++) {
//       branch2Entries[j] = (*branch2)[j];
//     } // end for j
//   } // end for i      

//   // loop over branch from file 1 to fill hist
//   for (unsigned int i = 0; i < tree1->GetEntries(); i++) {
//     tree1->GetEntry(i);

//     for (unsigned int j = 0; j < branch1->size(); j++) { // change to even/odd increments to select only leading/trailing
//       if (abs((*pdgID_branch)[j]) == 11) { // only electrons in hist
// 	hist->Fill((*pdgID_branch)[j] , branch2Entries);
//       }	// end if
//     }	// end for j
//   }	// end for i


  // // draw hist and write to output file:
  // hist->Draw();
  // TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  // outFile->Delete(Form("%s;*",branch2Name.c_str())); // Delete any existing histograms with the same name
  // hist->Write();
  // outFile->Close();
//}


int plotMaker() {
  gROOT->SetBatch(kTRUE); // prevent auto-opening of last hist
  string inFile = "/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.root";
  string outFile = "/home/users/aolson/tttt/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_plotMaker.root";
  string inFile2 = "/ceph/cms/store/group/tttt/Skims/230105/2018/DY_2l_M_50/DY_2l_M_50_1.root";


  // 6 args: strInFile, strOutFile, strBranchName (only takes vectors)
  //, nbins, xMin, xMax
  DrawBranchToHist(inFile, outFile, "leptons_pt", 50,0,200);
  DrawBranchToHist(inFile, outFile, "leptons_eta", 50,-3,3);
  DrawBranchToHist(inFile, outFile, "leptons_phi", 100,-4,4);
  DrawBranchToHist(inFile, outFile, "leptons_mass", 50,0,0.5);
  DrawBranchToHist(inFile, outFile, "ak4jets_nJets", 100,0,20);

  //  DrawBranchTo2DHist(inFile, inFile2, outFile, "leptons_pdgId","Electron_tightCharge",2,-13,13,2,-13,13);

  return 0;
}
