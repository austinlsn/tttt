#include "ScaleFactorHandlerBase.h"
#include "IvyFramework/IvyDataTools/interface/HelperFunctions.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace IvyStreamHelpers;


void ScaleFactorHandlerBase::closeFile(TFile*& f){
  if (f){
    if (f->IsOpen()) f->Close();
    else delete f;
  }
  f = nullptr;
}

void ScaleFactorHandlerBase::getAxisBinning(TAxis const* ax, ExtendedBinning& res){ HelperFunctions::getExtendedBinning(ax, res); }

template<> bool ScaleFactorHandlerBase::getHistogram<TH1F, ExtendedHistogram_1D_f>(ExtendedHistogram_1D_f& h, TDirectory* f, TString s){
  TDirectory* curdir = gDirectory;
  if (s=="") return false;
  if (!f) return false;
  TFile* ff = dynamic_cast<TFile*>(f);
  if (ff){
    if (!ff->IsOpen()) return false;
    if (ff->IsZombie()) return false;
  }

  f->cd();
  TH1F* hh = (TH1F*) f->Get(s);
  curdir->cd();

  if (hh){
    if (!HelperFunctions::checkHistogramIntegrity(hh)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hh->GetName() << " does not have integrity!" << endl;
      return false;
    }

    ExtendedBinning xbins("x"); getAxisBinning(hh->GetXaxis(), xbins);
    unsigned int nbinsx = xbins.getNbins();
    h.setBinning(xbins, 0);

    h.build(); h.resetProfiles(); h.setNameTitle(s+"_copy");
    TH1F* const& th = h.getHistogram();
    for (unsigned int ix=0; ix<=nbinsx+1; ix++){
      th->SetBinContent(ix, hh->GetBinContent(ix));
      th->SetBinError(ix, hh->GetBinError(ix));
    }
  }
  else IVYerr << "ScaleFactorHandlerBase::getHistogram: " << s << " cannot be acquired!" << endl;

  return (hh!=nullptr);
}
template<> bool ScaleFactorHandlerBase::getHistogram<TH1D, ExtendedHistogram_1D_f>(ExtendedHistogram_1D_f& h, TDirectory* f, TString s){
  TDirectory* curdir = gDirectory;
  if (s=="") return false;
  if (!f) return false;
  TFile* ff = dynamic_cast<TFile*>(f);
  if (ff){
    if (!ff->IsOpen()) return false;
    if (ff->IsZombie()) return false;
  }

  f->cd();
  TH1D* hh = (TH1D*) f->Get(s);
  curdir->cd();

  if (hh){
    if (!HelperFunctions::checkHistogramIntegrity(hh)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hh->GetName() << " does not have integrity!" << endl;
      return false;
    }

    ExtendedBinning xbins("x"); getAxisBinning(hh->GetXaxis(), xbins);
    unsigned int nbinsx = xbins.getNbins();
    h.setBinning(xbins, 0);

    h.build(); h.resetProfiles(); h.setNameTitle(s+"_copy");
    TH1F* const& th = h.getHistogram();
    for (unsigned int ix=0; ix<=nbinsx+1; ix++){
      th->SetBinContent(ix, hh->GetBinContent(ix));
      th->SetBinError(ix, hh->GetBinError(ix));
    }
  }
  else IVYerr << "ScaleFactorHandlerBase::getHistogram: " << s << " cannot be acquired!" << endl;

  return (hh!=nullptr);
}
template<> bool ScaleFactorHandlerBase::getHistogram<TH2F, ExtendedHistogram_2D_f>(ExtendedHistogram_2D_f& h, TDirectory* f, TString s){
  TDirectory* curdir = gDirectory;
  if (s=="") return false;
  if (!f) return false;
  TFile* ff = dynamic_cast<TFile*>(f);
  if (ff){
    if (!ff->IsOpen()) return false;
    if (ff->IsZombie()) return false;
  }

  f->cd();
  TH2F* hh = (TH2F*) f->Get(s);
  curdir->cd();

  if (hh){
    if (!HelperFunctions::checkHistogramIntegrity(hh)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hh->GetName() << " does not have integrity!" << endl;
      return false;
    }

    ExtendedBinning xbins("x"); getAxisBinning(hh->GetXaxis(), xbins);
    unsigned int nbinsx = xbins.getNbins();
    h.setBinning(xbins, 0);

    ExtendedBinning ybins("y"); getAxisBinning(hh->GetYaxis(), ybins);
    unsigned int nbinsy = ybins.getNbins();
    h.setBinning(ybins, 1);

    h.build(); h.resetProfiles(); h.setNameTitle(s+"_copy");
    TH2F* const& th = h.getHistogram();
    for (unsigned int ix=0; ix<=nbinsx+1; ix++){
      for (unsigned int iy=0; iy<=nbinsy+1; iy++){
        th->SetBinContent(ix, iy, hh->GetBinContent(ix, iy));
        th->SetBinError(ix, iy, hh->GetBinError(ix, iy));
      }
    }
  }
  else IVYerr << "ScaleFactorHandlerBase::getHistogram: " << s << " cannot be acquired!" << endl;

  return (hh!=nullptr);
}
template<> bool ScaleFactorHandlerBase::getHistogram<TH2D, ExtendedHistogram_2D_f>(ExtendedHistogram_2D_f& h, TDirectory* f, TString s){
  TDirectory* curdir = gDirectory;
  if (s=="") return false;
  if (!f) return false;
  TFile* ff = dynamic_cast<TFile*>(f);
  if (ff){
    if (!ff->IsOpen()) return false;
    if (ff->IsZombie()) return false;
  }

  f->cd();
  TH2D* hh = (TH2D*) f->Get(s);
  curdir->cd();

  if (hh){
    if (!HelperFunctions::checkHistogramIntegrity(hh)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hh->GetName() << " does not have integrity!" << endl;
      return false;
    }

    ExtendedBinning xbins("x"); getAxisBinning(hh->GetXaxis(), xbins);
    unsigned int nbinsx = xbins.getNbins();
    h.setBinning(xbins, 0);

    ExtendedBinning ybins("y"); getAxisBinning(hh->GetYaxis(), ybins);
    unsigned int nbinsy = ybins.getNbins();
    h.setBinning(ybins, 1);

    h.build(); h.resetProfiles(); h.setNameTitle(s+"_copy");
    TH2F* const& th = h.getHistogram();
    for (unsigned int ix=0; ix<=nbinsx+1; ix++){
      for (unsigned int iy=0; iy<=nbinsy+1; iy++){
        th->SetBinContent(ix, iy, hh->GetBinContent(ix, iy));
        th->SetBinError(ix, iy, hh->GetBinError(ix, iy));
      }
    }
  }
  else IVYerr << "ScaleFactorHandlerBase::getHistogram: " << s << " cannot be acquired!" << endl;

  return (hh!=nullptr);
}

template<> bool ScaleFactorHandlerBase::getHistogramWithUncertainy<TH1F, ExtendedHistogram_1D_f>(ExtendedHistogram_1D_f& h, TDirectory* f, TString s, TString su){
  TDirectory* curdir = gDirectory;
  if (s=="") return false;
  if (!f) return false;
  TFile* ff = dynamic_cast<TFile*>(f);
  if (ff){
    if (!ff->IsOpen()) return false;
    if (ff->IsZombie()) return false;
  }

  f->cd();
  TH1F* hh = (TH1F*) f->Get(s);
  TH1F* hu = (TH1F*) f->Get(su);
  curdir->cd();

  if (hh){
    if (!HelperFunctions::checkHistogramIntegrity(hh)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hh->GetName() << " does not have integrity!" << endl;
      return false;
    }
    if (!HelperFunctions::checkHistogramIntegrity(hu)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hu->GetName() << " does not have integrity!" << endl;
      return false;
    }

    ExtendedBinning xbins("x"); getAxisBinning(hh->GetXaxis(), xbins);
    unsigned int nbinsx = xbins.getNbins();
    h.setBinning(xbins, 0);

    h.build(); h.resetProfiles(); h.setNameTitle(s+"_copy");
    TH1F* const& th = h.getHistogram();
    for (unsigned int ix=0; ix<=nbinsx+1; ix++){
      th->SetBinContent(ix, hh->GetBinContent(ix));
      th->SetBinError(ix, hu->GetBinContent(ix));
    }
  }
  else IVYerr << "ScaleFactorHandlerBase::getHistogramWithUncertainy: " << s << " or " << su << " cannot be acquired!" << endl;

  return (hh!=nullptr);
}
template<> bool ScaleFactorHandlerBase::getHistogramWithUncertainy<TH1D, ExtendedHistogram_1D_f>(ExtendedHistogram_1D_f& h, TDirectory* f, TString s, TString su){
  TDirectory* curdir = gDirectory;
  if (s=="") return false;
  if (!f) return false;
  TFile* ff = dynamic_cast<TFile*>(f);
  if (ff){
    if (!ff->IsOpen()) return false;
    if (ff->IsZombie()) return false;
  }

  f->cd();
  TH1D* hh = (TH1D*) f->Get(s);
  TH1D* hu = (TH1D*) f->Get(su);
  curdir->cd();

  if (hh && hu){
    if (!HelperFunctions::checkHistogramIntegrity(hh)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hh->GetName() << " does not have integrity!" << endl;
      return false;
    }
    if (!HelperFunctions::checkHistogramIntegrity(hu)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hu->GetName() << " does not have integrity!" << endl;
      return false;
    }

    ExtendedBinning xbins("x"); getAxisBinning(hh->GetXaxis(), xbins);
    unsigned int nbinsx = xbins.getNbins();
    h.setBinning(xbins, 0);

    h.build(); h.resetProfiles(); h.setNameTitle(s+"_copy");
    TH1F* const& th = h.getHistogram();
    for (unsigned int ix=0; ix<=nbinsx+1; ix++){
      th->SetBinContent(ix, hh->GetBinContent(ix));
      th->SetBinError(ix, hu->GetBinContent(ix));
    }
  }
  else IVYerr << "ScaleFactorHandlerBase::getHistogramWithUncertainy: " << s << " or " << su << " cannot be acquired!" << endl;

  return (hh!=nullptr);
}
template<> bool ScaleFactorHandlerBase::getHistogramWithUncertainy<TH2F, ExtendedHistogram_2D_f>(ExtendedHistogram_2D_f& h, TDirectory* f, TString s, TString su){
  TDirectory* curdir = gDirectory;
  if (s=="") return false;
  if (!f) return false;
  TFile* ff = dynamic_cast<TFile*>(f);
  if (ff){
    if (!ff->IsOpen()) return false;
    if (ff->IsZombie()) return false;
  }

  f->cd();
  TH2F* hh = (TH2F*) f->Get(s);
  TH2F* hu = (TH2F*) f->Get(su);
  curdir->cd();

  if (hh && hu){
    if (!HelperFunctions::checkHistogramIntegrity(hh)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hh->GetName() << " does not have integrity!" << endl;
      return false;
    }
    if (!HelperFunctions::checkHistogramIntegrity(hu)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hu->GetName() << " does not have integrity!" << endl;
      return false;
    }

    ExtendedBinning xbins("x"); getAxisBinning(hh->GetXaxis(), xbins);
    unsigned int nbinsx = xbins.getNbins();
    h.setBinning(xbins, 0);

    ExtendedBinning ybins("y"); getAxisBinning(hh->GetYaxis(), ybins);
    unsigned int nbinsy = ybins.getNbins();
    h.setBinning(ybins, 1);

    h.build(); h.resetProfiles(); h.setNameTitle(s+"_copy");
    TH2F* const& th = h.getHistogram();
    for (unsigned int ix=0; ix<=nbinsx+1; ix++){
      for (unsigned int iy=0; iy<=nbinsy+1; iy++){
        th->SetBinContent(ix, iy, hh->GetBinContent(ix, iy));
        th->SetBinError(ix, iy, hu->GetBinContent(ix, iy));
      }
    }
  }
  else IVYerr << "ScaleFactorHandlerBase::getHistogramWithUncertainy: " << s << " or " << su << " cannot be acquired!" << endl;

  return (hh!=nullptr);
}
template<> bool ScaleFactorHandlerBase::getHistogramWithUncertainy<TH2D, ExtendedHistogram_2D_f>(ExtendedHistogram_2D_f& h, TDirectory* f, TString s, TString su){
  TDirectory* curdir = gDirectory;
  if (s=="") return false;
  if (!f) return false;
  TFile* ff = dynamic_cast<TFile*>(f);
  if (ff){
    if (!ff->IsOpen()) return false;
    if (ff->IsZombie()) return false;
  }

  f->cd();
  TH2D* hh = (TH2D*) f->Get(s);
  TH2D* hu = (TH2D*) f->Get(su);
  curdir->cd();

  if (hh && hu){
    if (!HelperFunctions::checkHistogramIntegrity(hh)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hh->GetName() << " does not have integrity!" << endl;
      return false;
    }
    if (!HelperFunctions::checkHistogramIntegrity(hu)){
      IVYerr << "ScaleFactorHandlerBase::getHistogram: " << hu->GetName() << " does not have integrity!" << endl;
      return false;
    }

    ExtendedBinning xbins("x"); getAxisBinning(hh->GetXaxis(), xbins);
    unsigned int nbinsx = xbins.getNbins();
    h.setBinning(xbins, 0);

    ExtendedBinning ybins("y"); getAxisBinning(hh->GetYaxis(), ybins);
    unsigned int nbinsy = ybins.getNbins();
    h.setBinning(ybins, 1);

    h.build(); h.resetProfiles(); h.setNameTitle(s+"_copy");
    TH2F* const& th = h.getHistogram();
    for (unsigned int ix=0; ix<=nbinsx+1; ix++){
      for (unsigned int iy=0; iy<=nbinsy+1; iy++){
        th->SetBinContent(ix, iy, hh->GetBinContent(ix, iy));
        th->SetBinError(ix, iy, hu->GetBinContent(ix, iy));
      }
    }
  }
  else IVYerr << "ScaleFactorHandlerBase::getHistogramWithUncertainy: " << s << " or " << su << " cannot be acquired!" << endl;

  return (hh!=nullptr);
}
