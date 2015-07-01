#ifndef EFFCURVESTOOLS_EFFCURVESTOOL_H
#define EFFCURVESTOOLS_EFFCURVESTOOL_H

#include "TEfficiency.h"
#include <map>
#include <string>

#include "xAODBase/IParticle.h"
#include "xAODTau/TauJet.h"
#include "xAODJet/Jet.h"

class EffCurvesTool

{

 public:
  EffCurvesTool(const std::string & name);
  virtual ~EffCurvesTool() {};

  bool fill_hadhad(bool pass, const xAOD::TauJet * t1, const xAOD::TauJet * t2, const xAOD::Jet * j1);

  bool fill_lephad(bool pass, const xAOD::IParticle * t1, const xAOD::TauJet * t2, const xAOD::Jet * j1);

  std::map<std::string, TEfficiency*> Efficiencies () {return m_eff;}
 private:
  
  std::map<std::string, TEfficiency*> m_eff;

};


#endif
