// Makes root hists out of test_looper's output .root file. Each lepton vector has 2 entries, which are sorted leading,trailing.

using namespace std;

// Immensely helps w/ naming histograms/canvases:
string removeSubstring(string str, string substr) {
  size_t pos = str.find(substr); // Find the position of the substring

  if (pos != string::npos) { // Check if the substring is found
    str.replace(pos, substr.length(), ""); // Remove the substring
  }

  return str;
}


// Create 1D histograms using any individual branch  
void make1D_hists(string inFileName, string outFileName, string branchName,int nbins, double xMin, double xMax, string XaxisTitle) {

  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  vector<double> *branch = 0;// initialize branch
  vector<int> *pdgID1_branch = 0; // get pdgIDs for filtering
  vector<int> *pdgID2_branch = 0;

  // Link branch variable address with Tree's branch
  SkimTree->SetBranchAddress(branchName.c_str(), &branch);
  SkimTree->SetBranchAddress("ee_leadingPdgId", &pdgID1_branch);
  SkimTree->SetBranchAddress("ee_trailingPdgId", &pdgID2_branch);
  
  string strhistOS = "h_OS_" + removeSubstring(branchName.c_str(),"ee_");
  string strhistSS = "h_SS_" + removeSubstring(branchName.c_str(),"ee_");

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
} // end make1D_hists


void canvasKinematicHists(string inFileName, string outFileName, 
			  string branchName_lead, string branchName_trail, 
			  int nbins,int xMin,int xMax, string XaxisTitle) {

  // add kinematics hists of events where both leptons are matched and events where a single lepton is matched to the same canvas.

  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  vector<double> *branch_lead      = 0; // initialize input leadingPt branch
  vector<double> *branch_trail     = 0; // initialize input trailingPt branch
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
  string strhistSS_general = removeSubstring(branchName_lead.c_str(), "leading_");
  strhistSS_general = removeSubstring(strhistSS_general.c_str(), "leading");
  string strhistSS_lead = "ee_SS_" + removeSubstring(branchName_lead.c_str(), "ee_");
  string strhistSS_trail = "ee_SS_" + removeSubstring(branchName_trail.c_str(), "ee_");
  string strcanvSS = strhistSS_general + ", 2 genmatches and 1 genmatch";

  TCanvas* c1 = new TCanvas("ee_kinematics_2gMvs1gM", strcanvSS.c_str(), 1200,600);
  string strhistSS_2M = strhistSS_general + "_2gM";
  string strhistSS_1M = strhistSS_general + "_1gM";
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
  outFile->Delete(Form("%s;*","ee_kinematics_2gMvs1gM"));

  c1->Write();
  outFile->Close();
} // end canvasKinematicsHists



void makeTruthMatchHists(string inFileName, string outFileName) {

  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile

  TH2I *histOS = new TH2I("h_OStruthMatch", "Truth-matched ee (OS)", 25, -12, 13, 25, -12, 34);
  TH2I *histSS = new TH2I("h_SStruthMatch", "Truth-matched ee (SS)", 25, -12, 13, 25, -12, 34);

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
	histOS->Fill((*pdgID1_branch)[j],(*genMatch_branch1)[j]);
	histOS->Fill((*pdgID2_branch)[j],(*genMatch_branch2)[j]);
      }
      if ((*pdgID1_branch)[j] * (*pdgID2_branch)[j] > 0) { // filter SS
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
  outFile->Delete(Form("%s;*","h_OStruthMatch")); // Delete any existing histograms with the same name
  outFile->Delete(Form("%s;*","h_SStruthMatch")); // Delete any existing histograms with the same name
  histOS->Write();
  histSS->Write();
  outFile->Close();

} // end makeTruthMatchHists


void make_phiEta_hists(string inFileName, string outFileName) {
  // input file, branch:
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  // output hist:
  TH2D *histPhiEta = new TH2D("h_EtaPhi", "all leptons eta vs. phi", 40,-4,4, 40,-3,3);

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
  histPhiEta->GetYaxis()->SetTitle("Eta");
  // Create output file:
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  outFile->Delete(Form("%s;*","h_EtaPhi")); // Delete any existing histograms with the same name

  histPhiEta->Write();
  outFile->Close();
} // end make_phiEta_hists


void make_flipRate_hists(string inFileName, string outFileName) {
  // input file, branch:
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  // output hist:
  TH2D *histPtEta_num = new TH2D("h_EtaPt_flips", "flipped leptons eta vs. pt", 6,15,300, 3,0,2.5);
  TH2D *histPtEta_den = new TH2D("h_EtaPt_matched", "matched leptons eta vs. pt", 6,15,300, 3,0,2.5);

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
  histPtEta_num->GetYaxis()->SetTitle("Eta");
  histPtEta_den->GetXaxis()->SetTitle("Pt (GeV/c)");
  histPtEta_den->GetYaxis()->SetTitle("Eta");

  TH2D* histPtEta_flRate = (TH2D*) histPtEta_num->Clone("h_EtaPt_flipRate");
  histPtEta_flRate->Divide(histPtEta_den);
  histPtEta_flRate->SetTitle("flip rate (flipped over matched leptons)");

  // Create output file:
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  // Delete any existing histograms with the same name:
  outFile->Delete(Form("%s;*","h_EtaPt_flips")); 
  outFile->Delete(Form("%s;*","h_EtaPt_matched")); 
  outFile->Delete(Form("%s;*","h_EtaPt_flipRate"));

  histPtEta_num->Write();
  histPtEta_den->Write();
  histPtEta_flRate->Write();
  outFile->Close();

} // end make_flipRate_hists



void make1D_SShists(string inFileName, string outFileName, string branchName_lead,string branchName_trail, int nbins, double xMin, double xMax, string XaxisTitle, bool use_abs) {

  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  vector<double> *branch_lead = 0;// initialize branch
  vector<double> *branch_trail = 0;// initialize branch
  SkimTree->SetBranchAddress(branchName_lead.c_str(), &branch_lead);
  SkimTree->SetBranchAddress(branchName_trail.c_str(), &branch_trail);
  
  // This is horrible, I should combine some functions
  vector<int> *pdgID1_branch = 0;
  vector<int> *pdgID2_branch = 0;
  vector<int> *genMatch_branch1 = 0;
  vector<int> *genMatch_branch2 = 0;
  SkimTree->SetBranchAddress("ee_genmatch_leadingPdgId", &genMatch_branch1);
  SkimTree->SetBranchAddress("ee_genmatch_trailingPdgId", &genMatch_branch2);
  SkimTree->SetBranchAddress("ee_leadingPdgId", &pdgID1_branch);
  SkimTree->SetBranchAddress("ee_trailingPdgId", &pdgID2_branch);


  string strhist = removeSubstring(branchName_lead.c_str(), "leading_");
  strhist = removeSubstring(strhist.c_str(), "leading");
  strhist = removeSubstring(strhist.c_str(), "ee_");
  string strhist_matchSS = "h_matchedToSS_" + strhist;
  string strhist_matchOS = "h_matchedToOS_" + strhist;
  string strhist_noMatch = "h_noMatch_" + strhist;
  string strcanv = strhist + "_SShists";

  TH1F *hist_matchSS = new TH1F(strhist_matchSS.c_str(), strhist_matchSS.c_str(), nbins, xMin, xMax);
  TH1F *hist_matchOS = new TH1F(strhist_matchOS.c_str(), strhist_matchOS.c_str(), nbins, xMin, xMax);
  TH1F *hist_noMatch = new TH1F(strhist_noMatch.c_str(), strhist_noMatch.c_str(), nbins, xMin, xMax);


  // loop to fill hist w/ branch's values
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < branch_lead->size(); j++) {
      if ((*pdgID1_branch)[j] * (*pdgID2_branch)[j] > 0) { // SS
	// matched to same sign electron:
	if ((*pdgID1_branch)[j] * (*genMatch_branch1)[j] == 121) {
	  if (use_abs) {
	    hist_matchSS->Fill(std::abs((*branch_lead)[j]));}
	  else {
	    hist_matchSS->Fill((*branch_lead)[j]);}
	}

	if ((*pdgID2_branch)[j] * (*genMatch_branch2)[j] == 121) {
	  if (use_abs) {
	    hist_matchSS->Fill(std::abs((*branch_trail)[j]));}
	  else {
	    hist_matchSS->Fill((*branch_trail)[j]);}
	}

	// matched to opposite sign electron:
	if ((*pdgID1_branch)[j] * (*genMatch_branch1)[j] == -121) {
	  if (use_abs) {
	    hist_matchOS->Fill(std::abs((*branch_lead)[j]));}
	  else {
	    hist_matchOS->Fill((*branch_lead)[j]);}
	}

	if ((*pdgID2_branch)[j] * (*genMatch_branch2)[j] == -121) {
	  if (use_abs) {
	    hist_matchOS->Fill(std::abs((*branch_trail)[j]));}
	  else {
	    hist_matchOS->Fill((*branch_trail)[j]);}
	}

	// not matched to electron:
	if (std::abs((*pdgID1_branch)[j] * (*genMatch_branch1)[j]) == 363) {
	  if (use_abs) {
	    hist_noMatch->Fill(std::abs((*branch_lead)[j]));}
	  else {
	    hist_noMatch->Fill((*branch_lead)[j]);}
	}

	if (std::abs((*pdgID2_branch)[j] * (*genMatch_branch2)[j]) == 363) {
	  if (use_abs) {
	    hist_noMatch->Fill(std::abs((*branch_trail)[j]));}
	  else {
	    hist_noMatch->Fill((*branch_trail)[j]);}
	}

      }	// end SS
    }// end for j
  }// end for i
  

  hist_matchSS->SetLineColor(kRed);
  hist_matchOS->SetLineColor(kBlue);
  hist_noMatch->SetLineColor(kGreen);
  
  TCanvas* c1 = new TCanvas(strcanv.c_str(),strcanv.c_str(),800,600);
  // Create output file
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  // Delete any existing canvas with the same name:
  outFile->Delete(Form("%s;*",strcanv.c_str()));

  TLegend *legend = new TLegend(0.7,0.7,.9,.9);
  legend->AddEntry(hist_matchSS, "matched to SS", "l");
  legend->AddEntry(hist_matchOS, "matched to OS", "l");
  legend->AddEntry(hist_noMatch, "matched to nothing", "l");


  float max_bin_content = std::max({hist_matchSS->GetMaximum(), hist_matchOS->GetMaximum(), hist_noMatch->GetMaximum()});

  // Sort the histograms based on their maximum bin content
  std::vector<TH1F*> hist_list = {hist_matchSS, hist_matchOS, hist_noMatch};
  std::sort(hist_list.begin(), hist_list.end(),
            [=](TH1F* h1, TH1F* h2) { return h1->GetMaximum() < h2->GetMaximum(); });

  // Draw the hists in the sorted order:
  for (auto it = hist_list.rbegin(); it != hist_list.rend(); ++it) {
    TH1F* hist = *it;
    if (hist->GetMaximum() == max_bin_content) {
      hist->Draw();
    }
    else {
      hist->SetTitle(strcanv.c_str());
      hist->Draw("same");
    }
  }

  legend->Draw();
  c1->Write();
  outFile->Close();

} // end make1D_SShists



void make2D_SShists(string inFileName, string outFileName, string branchName1_lead,string branchName1_trail, int nbins1, double xMin1, double xMax1, string XaxisTitle1, string branchName2_lead,string branchName2_trail, int nbins2, double xMin2, double xMax2, string XaxisTitle2) {

  // Open input .root file
  TFile *f = new TFile(inFileName.c_str(), "READ");
  TTree *SkimTree = (TTree*)f->Get("SkimTree"); // Get Tree from inFile
  vector<double> *branch1_lead = 0;// initialize branch
  vector<double> *branch1_trail = 0;// initialize branch
  vector<double> *branch2_lead = 0;// initialize branch
  vector<double> *branch2_trail = 0;// initialize branch
  SkimTree->SetBranchAddress(branchName1_lead.c_str(), &branch1_lead);
  SkimTree->SetBranchAddress(branchName1_trail.c_str(), &branch1_trail);
  SkimTree->SetBranchAddress(branchName2_lead.c_str(), &branch2_lead);
  SkimTree->SetBranchAddress(branchName2_trail.c_str(), &branch2_trail);

  // This is horrible, I should combine some functions
  vector<int> *pdgID1_branch = 0;
  vector<int> *pdgID2_branch = 0;
  vector<int> *genMatch_branch1 = 0;
  vector<int> *genMatch_branch2 = 0;
  SkimTree->SetBranchAddress("ee_genmatch_leadingPdgId", &genMatch_branch1);
  SkimTree->SetBranchAddress("ee_genmatch_trailingPdgId", &genMatch_branch2);
  SkimTree->SetBranchAddress("ee_leadingPdgId", &pdgID1_branch);
  SkimTree->SetBranchAddress("ee_trailingPdgId", &pdgID2_branch);


  string strhist1 = removeSubstring(branchName1_lead.c_str(), "leading_");
  strhist1 = removeSubstring(strhist1.c_str(), "leading");
  strhist1 = removeSubstring(strhist1.c_str(), "ee_");
  string strhist2 = removeSubstring(branchName2_lead.c_str(), "leading_");
  strhist2 = removeSubstring(strhist2.c_str(), "leading");
  strhist2 = removeSubstring(strhist2.c_str(), "ee_");

  string strhist = strhist2 + strhist1;
  string strhist_matchSS = "h_matchedToSS_" + strhist;
  string strhist_matchOS = "h_matchedToOS_" + strhist;
  string strhist_noMatch = "h_noMatch_" + strhist;
  string strcanv = strhist + "_SShists";

  TH2D *hist_matchSS = new TH2D(strhist_matchSS.c_str(), strhist_matchSS.c_str(), nbins1, xMin1, xMax1, nbins2, xMin2, xMax2);
  TH2D *hist_matchOS = new TH2D(strhist_matchOS.c_str(), strhist_matchOS.c_str(), nbins1, xMin1, xMax1, nbins2, xMin2, xMax2);
  TH2D *hist_noMatch = new TH2D(strhist_noMatch.c_str(), strhist_noMatch.c_str(), nbins1, xMin1, xMax1, nbins2, xMin2, xMax2);



  // loop to fill hist w/ branch's values
  for (unsigned int i = 0; i < SkimTree->GetEntries(); i++) {
    SkimTree->GetEntry(i);
    
    for (unsigned int j = 0; j < branch1_lead->size(); j++) {
      if ((*pdgID1_branch)[j] * (*pdgID2_branch)[j] > 0) { // SS
	// matched to same sign electron:
	if ((*pdgID1_branch)[j] * (*genMatch_branch1)[j] == 121)
	  hist_matchSS->Fill((*branch1_lead)[j],std::abs((*branch2_lead)[j]));
	if ((*pdgID2_branch)[j] * (*genMatch_branch2)[j] == 121)
	  hist_matchSS->Fill((*branch1_trail)[j],std::abs((*branch2_trail)[j]));

	// matched to opposite sign electron:
	if ((*pdgID1_branch)[j] * (*genMatch_branch1)[j] == -121)
	  hist_matchOS->Fill((*branch1_lead)[j],std::abs((*branch2_lead)[j]));
	if ((*pdgID2_branch)[j] * (*genMatch_branch2)[j] == -121)
	  hist_matchOS->Fill((*branch1_trail)[j],std::abs((*branch2_trail)[j]));

	// not matched to electron:
	if (std::abs((*pdgID1_branch)[j] * (*genMatch_branch1)[j]) == 363) 
	  hist_noMatch->Fill((*branch1_lead)[j],std::abs((*branch2_lead)[j]));
	if (std::abs((*pdgID2_branch)[j] * (*genMatch_branch2)[j]) == 363)
	  hist_noMatch->Fill((*branch1_trail)[j],std::abs((*branch2_trail)[j]));

      }	// end SS
    }// end for j
  }// end for i
  

  hist_matchSS->SetLineColor(kRed);
  hist_matchOS->SetLineColor(kBlue);
  hist_noMatch->SetLineColor(kGreen);
  hist_matchSS->GetXaxis()->SetTitle(XaxisTitle1.c_str());
  hist_matchSS->GetYaxis()->SetTitle(XaxisTitle2.c_str());
  hist_matchOS->GetXaxis()->SetTitle(XaxisTitle1.c_str());
  hist_matchOS->GetYaxis()->SetTitle(XaxisTitle2.c_str());
  hist_noMatch->GetXaxis()->SetTitle(XaxisTitle1.c_str());
  hist_noMatch->GetYaxis()->SetTitle(XaxisTitle2.c_str());


  // Create output file
  TFile *outFile = new TFile(outFileName.c_str(), "UPDATE");
  // Delete any existing canvas with the same name:
  outFile->Delete(Form("%s;*",strhist_matchSS.c_str()));
  outFile->Delete(Form("%s;*",strhist_matchOS.c_str()));
  outFile->Delete(Form("%s;*",strhist_noMatch.c_str()));
  
  hist_matchSS->Write();
  hist_matchOS->Write();
  hist_noMatch->Write();
  outFile->Close();
} // end make2D_SShists




int plotMaker() {
  gROOT->SetBatch(kTRUE); // prevent auto-opening of last drawn histogram
  string inFile = "/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/test/output/ExampleLooper/DYJetsToLL_M-50/2018/DY_2l_M_50.root";
  //string inFile = "/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/saved_outputs/DY_2l_M_50_20files/DY_2l_M_50.root";
  string outFile = "/home/users/aolson/tttt2/CMSSW_10_6_26/src/tttt/runDataAnalysis_scripts/output_plotMaker.root";

  // Recreate output file at beginning of script.
  TFile *output = new TFile(outFile.c_str(), "RECREATE");  
  delete output;


  make1D_hists(inFile, outFile, "ee_pt", 50,0,200, "pT (GeV/c)");
  // make1D_hists(inFile, outFile, "ee_leadingPt", 50,0,200, "pT (GeV/c)");
  // make1D_hists(inFile, outFile, "ee_trailingPt", 50,0,200, "pT (GeV/c)");
  // make1D_hists(inFile, outFile, "ee_leading_eta", 50,-3,3, "eta");
  // make1D_hists(inFile, outFile, "ee_trailing_eta", 50,-3,3, "eta");
  make1D_hists(inFile, outFile, "ee_leading_phi", 100,-4,4, "phi (rad)");
  //make1D_hists(inFile, outFile, "ee_trailing_phi", 100,-4,4, "phi (rad)");
  make1D_hists(inFile, outFile, "ee_mass", 80,0,125, "mass (GeV/c^2)");
  //make1D_hists(inFile, outFile, "ee_nJets", 15,0,15, "# jets / event");
  make1D_hists(inFile, outFile, "ee_leading_tightCharge", 24,-11,12, "");
  make1D_hists(inFile, outFile, "ee_trailing_tightCharge", 24,-11,12, "");

  // Currently not asked for by Golf
  //canvasKinematicHists(inFile,outFile, "ee_leadingPt","ee_trailingPt", 50,0,200, "pT (GeV/c)");
  //canvasKinematicHists(inFile,outFile, "ee_leading_eta","ee_trailing_eta", 50,-3,3, "eta");
  //canvasKinematicHists(inFile,outFile, "ee_leading_phi","ee_trailing_phi", 100,-4,4, "phi (rad)");

  make1D_SShists(inFile, outFile, "ee_leadingPt","ee_trailingPt", 50,0,200, "pT (GeV/c)",0);
  make1D_SShists(inFile, outFile, "ee_leading_eta","ee_trailing_eta", 20,0,3, "eta",1);
  make1D_SShists(inFile, outFile, "ee_genmatch_leadingPt","ee_genmatch_trailingPt", 50,0,200, "pT (GeV/c)",0);
  make1D_SShists(inFile, outFile, "ee_genmatch_leading_eta","ee_genmatch_trailing_eta", 20,0,3, "eta",1);
  make1D_SShists(inFile, outFile, "ee_leading_dxy","ee_trailing_dxy", 20,0,0.05, "dxy",0);
  make1D_SShists(inFile, outFile, "ee_leading_dz","ee_trailing_dz", 20,0,0.1, "dz",0);
  make1D_SShists(inFile, outFile, "ee_leading_iso","ee_trailing_iso", 20,0,0.1, "iso",0);
  make1D_SShists(inFile, outFile, "ee_leading_dR","ee_trailing_dR", 20,0,0.02, "dR",1);

  make1D_SShists(inFile, outFile, "ee_genmatch_leading_mom_PdgId","ee_genmatch_trailing_mom_PdgId",38,-12,25, "pdgId",0);

  make2D_SShists(inFile, outFile, "ee_leadingPt","ee_trailingPt", 6,15,300,"pT (GeV/c)", "ee_leading_eta","ee_trailing_eta", 3,0,2.5,"eta");

  makeTruthMatchHists(inFile, outFile);
  make_flipRate_hists(inFile, outFile);
  return 0;
}
