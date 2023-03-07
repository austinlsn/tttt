// Makes root hists out of test_looper's output .root file. Each lepton vector has 2 entries, which are sorted leading,trailing.

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
  
  // write hists to output file:
  histOS->GetXaxis()->SetTitle(XaxisTitle.c_str());
  histSS->GetXaxis()->SetTitle(XaxisTitle.c_str());

  // Create output file
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*",strhistOS.c_str())); // Delete any existing hists with the same name
  outFile->Delete(Form("%s;*",strhistSS.c_str())); // Delete any existing hists with the same name
  histOS->Write();
  histSS->Write();
  outFile->Close();
} // end DrawBranchToHist


void canvasKinematicHists(string inFileName, string outFileName, 
			  string branchName_lead, string branchName_trail, 
			  int nbins,int xMin,int xMax, string XaxisTitle) {

  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  vector<float> *branch_lead      = 0; // initialize input leadingPt branch
  vector<float> *branch_trail     = 0; // initialize input trailingPt branch
  vector<int> *pdgID_branch1      = 0; // get pdgIDs for filtering
  vector<int> *pdgID_branch2      = 0;
  vector<int> *genMatchID_branch1 = 0; // get genMatch pdgIDs for filtering
  vector<int> *genMatchID_branch2 = 0;

  // Link branch variable address with Tree's branch
  SkimTree->SetBranchAddress(branchName_lead.c_str(), &branch_lead);
  SkimTree->SetBranchAddress(branchName_trail.c_str(), &branch_trail);
  SkimTree->SetBranchAddress("ee_leadingPdgId", &pdgID_branch1);
  SkimTree->SetBranchAddress("ee_trailingPdgId", &pdgID_branch2);
  SkimTree->SetBranchAddress("ee_genmatch_leadingPdgId", &genMatchID_branch1);
  SkimTree->SetBranchAddress("ee_genmatch_trailingPdgId", &genMatchID_branch2);

  // Make Canvas, hists:
  std::string strhistSS_general = "ee"+ branchName_lead.substr(10,-1);
  std::string strhistSS_lead = "SS_" + branchName_lead;
  std::string strhistSS_trail = "SS_" + branchName_trail;
  std::string strcanvSS = strhistSS_general + ", 2 genmatches and 1 genmatch";

  TCanvas* c1 = new TCanvas("c1", strcanvSS.c_str(), 1200,600);
  std::string strhistSS_2M = strhistSS_general + "_2gM";
  std::string strhistSS_1M = strhistSS_general + "_1gM";
  TH1F *histSS_2M = new TH1F(strhistSS_2M.c_str(), 
			     strhistSS_2M.c_str(), 
			     nbins, xMin, xMax);
  TH1F *histSS_1M = new TH1F(strhistSS_1M.c_str(), 
			     strhistSS_1M.c_str(), 
			     nbins, xMin, xMax);

  // loop to fill hist w/ branch's values (and filter)
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < branch_lead->size(); j++) {
      if ((*pdgID_branch1)[j] * (*pdgID_branch2)[j] > 0 // SS filter
	  && (abs((*genMatchID_branch1)[j] * (*genMatchID_branch2)[j]) == 121)) { 
	histSS_2M->Fill((*branch_lead)[j]);
	histSS_2M->Fill((*branch_trail)[j]); // combine leading/trailing branches
      }	// end "nGenMatch == 2" filter
      if ((*pdgID_branch1)[j] * (*pdgID_branch2)[j] > 0 // SS filter
	  && (abs((*genMatchID_branch1)[j] * (*genMatchID_branch2)[j]) == 363)) { 
	histSS_1M->Fill((*branch_lead)[j]);
	histSS_1M->Fill((*branch_trail)[j]); // combine leading/trailing branches
      }	// end "nGenMatch == 1 filter" (assumes "pdgID = 33" is "noMatch")

    }// end for j
  }// end for i
  
  // draw hists onto canvas:
  histSS_2M->GetXaxis()->SetTitle(XaxisTitle.c_str());
  histSS_1M->GetXaxis()->SetTitle(XaxisTitle.c_str());
  c1->Divide(2,1);
  c1->cd(1);
  histSS_2M->Draw();
  c1->cd(2);
  histSS_1M->Draw("SAME");	// draw to same TCanvas

  // Create output file and write hists:
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*",strhistSS_2M.c_str())); // Delete any existing hists with the same name
  outFile->Delete(Form("%s;*",strhistSS_1M.c_str())); // Delete any existing hists with the same name
  outFile->Delete(Form("%s;*","c1"));

  histSS_2M->Write();
  histSS_1M->Write();
  c1->Write();
  outFile->Close();
} // end DrawBranchToHist



void make2Dhists(string inFileName, string outFileName) {

  int OS_counter = 0;
  int SS_counter = 0;

  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile

  TH2I *histOS = new TH2I("ee_OS_truthMatch", "Truth-matched ee (OS)", 25, -12, 13, 25, -12, 34);
  TH2I *histSS = new TH2I("ee_SS_truthMatch", "Truth-matched ee (SS)", 25, -12, 13, 25, -12, 34);

  vector<int> *pdgID1_branch = 0;
  vector<int> *pdgID2_branch = 0;
  vector<int> *genMatch_branch1 = 0;
  vector<int> *genMatch_branch2 = 0;

  SkimTree->SetBranchAddress("ee_genmatch_leadingPdgId", &genMatch_branch1);
  SkimTree->SetBranchAddress("ee_genmatch_trailingPdgId", &genMatch_branch2);
  SkimTree->SetBranchAddress("ee_leadingPdgId", &pdgID1_branch);
  SkimTree->SetBranchAddress("ee_trailingPdgId", &pdgID2_branch);


  // loop to fill hists w/ branch's values
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < genMatch_branch1->size(); j++) {
      if ((*pdgID1_branch)[j] * (*pdgID2_branch)[j] < 0) { // filter OS
	OS_counter++;
	histOS->Fill((*pdgID1_branch)[j],(*genMatch_branch1)[j]);
	histOS->Fill((*pdgID2_branch)[j],(*genMatch_branch2)[j]);
      }
      if ((*pdgID1_branch)[j] * (*pdgID2_branch)[j] > 0) { // filter SS
	SS_counter++;
	histSS->Fill((*pdgID1_branch)[j],(*genMatch_branch1)[j]);
	histSS->Fill((*pdgID2_branch)[j],(*genMatch_branch2)[j]);
      }
    } // end for j
  }   // end for i
  // draw hists and write to output file:
  histOS->GetXaxis()->SetTitle("Reconstructed pdgID");
  histOS->GetYaxis()->SetTitle("Genmatched pdgID");
  histOS->SetMarkerSize(1.8);
  histSS->GetXaxis()->SetTitle("Reconstructed pdgID");
  histSS->GetYaxis()->SetTitle("Genmatched pdgID");
  histSS->SetMarkerSize(1.8);
  // Create output file
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*","ee_OS_truthMatch")); // Delete any existing histograms with the same name
  outFile->Delete(Form("%s;*","ee_SS_truthMatch")); // Delete any existing histograms with the same name
  histOS->Write();
  histSS->Write();
  outFile->Close();

  std::cout << 	OS_counter << endl;
  std::cout << 	SS_counter << endl;

} // end make2Dhists


void make_phiEta_hists(string inFileName, string outFileName) {
  // input file, branch:
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  // output hist:
  TH2D *histPhiEta = new TH2D("ee_2DPhiEta", "all leptons eta vs. phi", 9,-4,4, 7,-3,3);

  vector<double> *etaBranchLead = 0;
  vector<double> *phiBranchLead = 0;
  vector<double> *etaBranchTrail = 0;
  vector<double> *phiBranchTrail = 0;
  SkimTree->SetBranchAddress("ee_leading_eta", &etaBranchLead);
  SkimTree->SetBranchAddress("ee_leading_phi", &phiBranchLead);
  SkimTree->SetBranchAddress("ee_trailing_eta", &etaBranchTrail);
  SkimTree->SetBranchAddress("ee_trailing_phi", &phiBranchTrail);

  // loop to fill hists w/ branch's values
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < etaBranchLead->size(); j++) {
      histPhiEta->Fill((*phiBranchLead)[j],(*etaBranchLead)[j]);
    } // end for j
    for (unsigned int j = 0; j < etaBranchTrail->size(); j++) {
      histPhiEta->Fill((*phiBranchTrail)[j],(*etaBranchTrail)[j]);
    } // end for j
  }   // end for i

  // draw hists and write to output file:
  histPhiEta->GetXaxis()->SetTitle("Phi (rad)");
  histPhiEta->GetYaxis()->SetTitle("Eta (rad)");
  // Create output file:
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*","ee_2DPhiEta")); // Delete any existing histograms with the same name

  histPhiEta->Write();
  outFile->Close();
} // end make_phiEta_hists


void make_ptEta_hists(string inFileName, string outFileName) {
  // input file, branch:
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  // output hist:
  TH2D *histPtEta_num = new TH2D("ptEta_flips", "flipped leptons eta vs. pt", 25,0,200, 20,0,3);
  TH2D *histPtEta_den = new TH2D("ptEta_matched", "matched leptons eta vs. pt", 25,0,200, 20,0,3);

  vector<double> *etaBranchLead = 0;
  vector<double> *ptBranchLead = 0;
  vector<double> *etaBranchTrail = 0;
  vector<double> *ptBranchTrail = 0;
  SkimTree->SetBranchAddress("ee_leading_eta", &etaBranchLead);
  SkimTree->SetBranchAddress("ee_leadingPt", &ptBranchLead);
  SkimTree->SetBranchAddress("ee_trailing_eta", &etaBranchTrail);
  SkimTree->SetBranchAddress("ee_trailingPt", &ptBranchTrail);

  // This is horrible, I should combine some functions
  vector<int> *pdgID1_branch = 0;
  vector<int> *pdgID2_branch = 0;
  vector<int> *genMatch_branch1 = 0;
  vector<int> *genMatch_branch2 = 0;
  SkimTree->SetBranchAddress("ee_genmatch_leadingPdgId", &genMatch_branch1);
  SkimTree->SetBranchAddress("ee_genmatch_trailingPdgId", &genMatch_branch2);
  SkimTree->SetBranchAddress("ee_leadingPdgId", &pdgID1_branch);
  SkimTree->SetBranchAddress("ee_trailingPdgId", &pdgID2_branch);

  // loop to fill hists w/ branch's values
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < etaBranchLead->size(); j++) {
      // Opposite Sign (numerator):
      if ((*pdgID1_branch)[j] * (*genMatch_branch1)[j] == -121) 
	histPtEta_num->Fill((*ptBranchLead)[j],std::abs((*etaBranchLead)[j]));
      if ((*pdgID2_branch)[j] * (*genMatch_branch2)[j] == -121) 
	histPtEta_num->Fill((*ptBranchTrail)[j],std::abs((*etaBranchTrail)[j]));

      // Matched to something (denominator):
      if (std::abs((*pdgID1_branch)[j] * (*genMatch_branch1)[j]) == 121) 
	histPtEta_den->Fill((*ptBranchLead)[j],std::abs((*etaBranchLead)[j]));
      if (std::abs((*pdgID2_branch)[j] * (*genMatch_branch2)[j]) == 121) 
	histPtEta_den->Fill((*ptBranchTrail)[j],std::abs((*etaBranchTrail)[j]));
    } // end for j
  }   // end for i

  // draw hists and write to output file:
  histPtEta_num->GetXaxis()->SetTitle("Pt (GeV/c)");
  histPtEta_num->GetYaxis()->SetTitle("Eta (rad)");
  histPtEta_den->GetXaxis()->SetTitle("Pt (GeV/c)");
  histPtEta_den->GetYaxis()->SetTitle("Eta (rad)");

  TH2D* histPtEta_flRate = (TH2D*) histPtEta_num->Clone("ptEta_flipRate");
  histPtEta_flRate->Divide(histPtEta_den);
  histPtEta_flRate->SetTitle("flip rate (flipped over matched leptons)");

  // Create output file:
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  // Delete any existing histograms with the same name:
  outFile->Delete(Form("%s;*","ptEta_flips")); 
  outFile->Delete(Form("%s;*","ptEta_matched")); 
  outFile->Delete(Form("%s;*","ptEta_flipRate"));

  histPtEta_num->Write();
  histPtEta_den->Write();
  histPtEta_flRate->Write();
  outFile->Close();
} // end make_ptEta_hists




int plotMaker() {
  gROOT->SetBatch(kTRUE); // prevent auto-opening of last drawn histogram
  string inFile = "/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.root";
  //string inFile = "/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/saved_outputs/DY_2l_M_50_5files/DY_2l_M_50.root";
  string outFile = "/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_plotMaker.root";


  DrawBranchToHist(inFile, outFile, "ee_pt", 50,0,200, "pT (GeV/c)");
  DrawBranchToHist(inFile, outFile, "ee_leadingPt", 50,0,200, "pT (GeV/c)");
  DrawBranchToHist(inFile, outFile, "ee_trailingPt", 50,0,200, "pT (GeV/c)");
  DrawBranchToHist(inFile, outFile, "ee_leading_eta", 50,-3,3, "eta");
  DrawBranchToHist(inFile, outFile, "ee_trailing_eta", 50,-3,3, "eta");
  //DrawBranchToHist(inFile, outFile, "ee_leading_phi", 100,-4,4, "phi (rad)");
  //DrawBranchToHist(inFile, outFile, "ee_trailing_phi", 100,-4,4, "phi (rad)");
  DrawBranchToHist(inFile, outFile, "ee_mass", 80,0,125, "mass (GeV/c^2)");
  DrawBranchToHist(inFile, outFile, "ee_nJets", 15,0,15, "# jets / event");

  canvasKinematicHists(inFile,outFile, "ee_leadingPt","ee_trailingPt", 50,0,200, "pT (GeV/c)");
  //canvasKinematicHists(inFile,outFile, "ee_leading_eta","ee_trailing_eta", 50,-3,3, "eta");
  //canvasKinematicHists(inFile,outFile, "ee_leading_phi","ee_trailing_phi", 100,-4,4, "phi (rad)");

  make2Dhists(inFile, outFile);
  make_phiEta_hists(inFile, outFile);
  make_ptEta_hists(inFile, outFile);
  return 0;
}
