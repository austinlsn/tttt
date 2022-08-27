#include <cassert>
#include "ParticleObjectHelpers.h"
#include "DileptonHandler.h"
#include "IvyFramework/IvyDataTools/interface/IvyStreamHelpers.hh"


using namespace std;
using namespace IvyStreamHelpers;


DileptonHandler::DileptonHandler() :
  verbosity(MiscUtils::ERROR)
{}

bool DileptonHandler::constructDileptons(
  std::vector<MuonObject*> const* muons,
  std::vector<ElectronObject*> const* electrons
){
  clear();

  bool res = (constructOSDileptons(muons, electrons) && constructSSDileptons(muons, electrons));
  // Sort particles here
  if (res) ParticleObjectHelpers::sortByGreaterScalarSumPt_ImmediateDaughters(productList);
  setDileptonFlags();
  return res;
}
bool DileptonHandler::constructSSDileptons(
  std::vector<MuonObject*> const* muons,
  std::vector<ElectronObject*> const* electrons
){
  if (verbosity==MiscUtils::DEBUG) IVYout << "DileptonHandler::constructSSDileptons: Constructing SS pairs..." << endl;

  std::vector<ParticleObject*> lepMinusPlus[2][2]; // l-, l+

  if (electrons){
    for (std::vector<ElectronObject*>::const_iterator it = electrons->begin(); it!=electrons->end(); it++){ // Electrons
      int iFirst = 0;
      int iSecond = ((*it)->pdgId()<0 ? 1 : 0);
      lepMinusPlus[iFirst][iSecond].push_back(*it);
    }
  }
  if (muons){
    for (std::vector<MuonObject*>::const_iterator it = muons->begin(); it!=muons->end(); it++){ // Muons
      int iFirst = 1;
      int iSecond = ((*it)->pdgId()<0 ? 1 : 0);
      lepMinusPlus[iFirst][iSecond].push_back(*it);
    }
  }
  for (int s=0; s<2; s++){
    // SSSF
    for (int c=0; c<2; c++){
      for (ParticleObject* F1:lepMinusPlus[c][s]){
        for (ParticleObject* F2:lepMinusPlus[c][s]){
          if (F1==F2) continue;
          if (verbosity==MiscUtils::DEBUG) IVYout << "\t- Cdd of SSSF pair from " << F1->pdgId() << ", " << F2->pdgId() << endl;
          if (ParticleObject::checkDeepDaughtership(F1, F2)){
            if (verbosity==MiscUtils::DEBUG) IVYout << "\t\t- Cdd failed." << endl;
            continue;
          }
          if (verbosity==MiscUtils::DEBUG) IVYout << "\t- Found SSSF pair from " << F1->pdgId() << ", " << F2->pdgId() << endl;
          ParticleObject::LorentzVector_t pV = F1->p4() + F2->p4();
          DileptonObject* V = new DileptonObject(0, pV);
          V->addDaughter(F1);
          V->addDaughter(F2);
          productList.push_back(V);
        }
      }
    }
    // SSDF
    for (int c=0; c<1; c++){ // Index only reaches 1 becuase e+mu+ and mu+e+ are the same collections.
      for (ParticleObject* F1:lepMinusPlus[c][s]){
        for (ParticleObject* F2:lepMinusPlus[1-c][s]){
          if (verbosity==MiscUtils::DEBUG) IVYout << "\t- Cdd of SSDF pair from " << F1->pdgId() << ", " << F2->pdgId() << endl;
          if (ParticleObject::checkDeepDaughtership(F1, F2)){
            if (verbosity==MiscUtils::DEBUG) IVYout << "\t\t- Cdd failed." << endl;
            continue;
          }
          if (verbosity==MiscUtils::DEBUG) IVYout << "\t- Found SSDF pair from " << F1->pdgId() << ", " << F2->pdgId() << endl;
          ParticleObject::LorentzVector_t pV = F1->p4() + F2->p4();
          DileptonObject* V = new DileptonObject(0, pV);
          V->addDaughter(F1);
          V->addDaughter(F2);
          productList.push_back(V);
        }
      }
    }
  }

  if (verbosity==MiscUtils::DEBUG) IVYout << "DileptonHandler::constructSSDileptons: Number of pairs after SS pairs: " << productList.size() << endl;
  return true;
}
bool DileptonHandler::constructOSDileptons(
  std::vector<MuonObject*> const* muons,
  std::vector<ElectronObject*> const* electrons
){
  if (verbosity==MiscUtils::DEBUG) IVYout << "DileptonHandler::constructOSDileptons: Constructing OS pairs..." << endl;

  std::vector<ParticleObject*> lepMinusPlus[2][2]; // l-, l+

  if (electrons){
    for (std::vector<ElectronObject*>::const_iterator it = electrons->begin(); it!=electrons->end(); it++){ // Electrons
      int iFirst = 0;
      int iSecond = ((*it)->pdgId()<0 ? 1 : 0);
      lepMinusPlus[iFirst][iSecond].push_back(*it);
    }
  }
  if (muons){
    for (std::vector<MuonObject*>::const_iterator it = muons->begin(); it!=muons->end(); it++){ // Muons
      int iFirst = 1;
      int iSecond = ((*it)->pdgId()<0 ? 1 : 0);
      lepMinusPlus[iFirst][iSecond].push_back(*it);
    }
  }
  // OSSF
  for (int c=0; c<2; c++){
    for (ParticleObject* F1:lepMinusPlus[c][0]){
      for (ParticleObject* F2:lepMinusPlus[c][1]){
        if (verbosity==MiscUtils::DEBUG) IVYout << "\t- Cdd of OSSF pair from " << F1->pdgId() << ", " << F2->pdgId() << endl;
        if (ParticleObject::checkDeepDaughtership(F1, F2)){
          if (verbosity==MiscUtils::DEBUG) IVYout << "\t\t- Cdd failed." << endl;
          continue;
        }
        if (verbosity==MiscUtils::DEBUG) IVYout << "\t- Found OSSF pair from " << F1->pdgId() << ", " << F2->pdgId() << endl;
        ParticleObject::LorentzVector_t pV = F1->p4() + F2->p4();
        DileptonObject* V = new DileptonObject(23, pV);
        V->addDaughter(F1);
        V->addDaughter(F2);
        productList.push_back(V);
      }
    }
  }
  // OSDF
  for (int c=0; c<2; c++){
    for (ParticleObject* F1:lepMinusPlus[c][0]){
      for (ParticleObject* F2:lepMinusPlus[1-c][1]){
        if (verbosity==MiscUtils::DEBUG) IVYout << "\t- Cdd of OSDF pair from " << F1->pdgId() << ", " << F2->pdgId() << endl;
        if (ParticleObject::checkDeepDaughtership(F1, F2)){
          if (!F1 || !F1) IVYout << "\t\t- Cdd fail: !" << F1 << " || !" << F2 << endl;
          if (F1 == F2) IVYout << "\t\t- Cdd fail: " << F1 << "==" << F2 << endl;

          auto const& daughters1 = F1->getDaughters();
          auto const& daughters2 = F2->getDaughters();
          if (HelperFunctions::hasCommonElements(daughters1, daughters2)) IVYout << "\t\t- Cdd fail: " << daughters1 << " common to " << daughters2 << endl;

          continue;
        }
        if (verbosity==MiscUtils::DEBUG) IVYout << "\t- Found OSDF pair from " << F1->pdgId() << ", " << F2->pdgId() << endl;
        ParticleObject::LorentzVector_t pV = F1->p4() + F2->p4();
        DileptonObject* V = new DileptonObject(0, pV);
        V->addDaughter(F1);
        V->addDaughter(F2);
        productList.push_back(V);
      }
    }
  }

  if (verbosity==MiscUtils::DEBUG) IVYout << "DileptonHandler::constructOSDileptons: Number of pairs after OS pairs: " << productList.size() << endl;
  return true;
}
