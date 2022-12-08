#ifndef GENPARTICLEOBJECT_H
#define GENPARTICLEOBJECT_H

#include "ParticleObject.h"


#define GENPARTICLE_ID_MOMENTUM_VARIABLES \
GENPARTICLE_VARIABLE(int, pdgId, 0) \
GENPARTICLE_VARIABLE(int, status, 0) \
GENPARTICLE_VARIABLE(int, statusFlags, 0) \
GENPARTICLE_VARIABLE(float, pt, 0) \
GENPARTICLE_VARIABLE(float, eta, 0) \
GENPARTICLE_VARIABLE(float, phi, 0) \
GENPARTICLE_VARIABLE(float, mass, 0)

#define GENPARTICLE_MOTHER_VARIABLES \
GENPARTICLE_VARIABLE(int, genPartIdxMother, -1)

#define GENPARTICLE_EXTRA_VARIABLES \
GENPARTICLE_VARIABLE(bool, isPrompt, false) \
GENPARTICLE_VARIABLE(bool, isPromptFinalState, false) \
GENPARTICLE_VARIABLE(bool, isDirectPromptTauDecayProductFinalState, false) \
GENPARTICLE_VARIABLE(bool, isHardProcess, false) \
GENPARTICLE_VARIABLE(bool, fromHardProcessFinalState, false) \
GENPARTICLE_VARIABLE(bool, isDirectHardProcessTauDecayProductFinalState, false) \
GENPARTICLE_VARIABLE(bool, fromHardProcessBeforeFSR, false) \
GENPARTICLE_VARIABLE(bool, isLastCopy, false) \
GENPARTICLE_VARIABLE(bool, isLastCopyBeforeFSR, false)

#define GENPARTICLE_NANOAOD_VARIABLES \
GENPARTICLE_ID_MOMENTUM_VARIABLES \
GENPARTICLE_MOTHER_VARIABLES


class GenParticleVariables{
public:
#define GENPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
  GENPARTICLE_EXTRA_VARIABLES;
#undef GENPARTICLE_VARIABLE

  GenParticleVariables();
  GenParticleVariables(GenParticleVariables const& other);
  GenParticleVariables& operator=(const GenParticleVariables& other);

  void swap(GenParticleVariables& other);

};

class GenParticleObject : public ParticleObject{
protected:
  int st;

public:
  GenParticleVariables extras;

  GenParticleObject();
  GenParticleObject(int id_, int st_);
  GenParticleObject(int id_, int st_, LorentzVector_t const& mom_);
  GenParticleObject(const GenParticleObject& other);
  GenParticleObject& operator=(const GenParticleObject& other);
  ~GenParticleObject();

  void swap(GenParticleObject& other);

  int& status(){ return this->st; }
  int const& status() const{ return this->st; }

  void assignStatusBits(int const& statusFlags);

};

#endif
