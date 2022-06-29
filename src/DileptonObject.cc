#include <algorithm>
#include <utility>
#include "DileptonObject.h"
#include "ParticleSelectionHelpers.h"


DileptonVariables::DileptonVariables(){
#define DILEPTON_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=DEFVAL;
  DILEPTON_VARIABLES;
#undef DILEPTON_VARIABLE
}
DileptonVariables::DileptonVariables(DileptonVariables const& other){
#define DILEPTON_VARIABLE(TYPE, NAME, DEFVAL) this->NAME=other.NAME;
  DILEPTON_VARIABLES;
#undef DILEPTON_VARIABLE
}
void DileptonVariables::swap(DileptonVariables& other){
#define DILEPTON_VARIABLE(TYPE, NAME, DEFVAL) std::swap(this->NAME, other.NAME);
  DILEPTON_VARIABLES;
#undef DILEPTON_VARIABLE
}
DileptonVariables& DileptonVariables::operator=(const DileptonVariables& other){
  DileptonVariables tmp(other);
  swap(tmp);
  return *this;
}

DileptonObject::DileptonObject() :
  ParticleObject(),
  extras()
{}
DileptonObject::DileptonObject(int id_) :
  ParticleObject(id_),
  extras()
{}
DileptonObject::DileptonObject(int id_, LorentzVector_t const& momentum_) :
  ParticleObject(id_, momentum_),
  extras()
{}
DileptonObject::DileptonObject(const DileptonObject& other) :
  ParticleObject(other),
  extras(other.extras)
{}
void DileptonObject::swap(DileptonObject& other){
  ParticleObject::swap(other);
  extras.swap(other.extras);
}
DileptonObject& DileptonObject::operator=(const DileptonObject& other){
  DileptonObject tmp(other);
  swap(tmp);
  return *this;
}
DileptonObject::~DileptonObject(){}

void DileptonObject::configure(){
  using namespace ParticleSelectionHelpers;

  if (this->getNDaughters()<2 || !this->daughter(0) || !this->daughter(1)) return;

  int pdgIdMult = this->daughter(0)->pdgId() * this->daughter(1)->pdgId();
  unsigned int abs_pdgIdMult = std::abs(pdgIdMult);

  this->extras.isOS = (pdgIdMult<0);
  this->extras.isSF = (abs_pdgIdMult == 121 || abs_pdgIdMult == 169 || abs_pdgIdMult == 225);

  if (isTightParticle(this->daughter(0))) this->extras.nTightDaughters++;
  if (isTightParticle(this->daughter(1))) this->extras.nTightDaughters++;

  this->extras.isValid = true;
}

ParticleObject* DileptonObject::getDaughter_leadingPt() const{
  if (!extras.isValid) return nullptr;
  auto const& d0 = this->daughter(0);
  auto const& d1 = this->daughter(1);
  return dynamic_cast<ParticleObject*>((d0->pt()>d1->pt() ? d0 : d1));
}
ParticleObject* DileptonObject::getDaughter_subleadingPt() const{
  if (!extras.isValid) return nullptr;
  auto const& d0 = this->daughter(0);
  auto const& d1 = this->daughter(1);
  return dynamic_cast<ParticleObject*>((d0->pt()>d1->pt() ? d1 : d0));
}
