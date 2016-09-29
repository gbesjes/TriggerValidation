#ifndef EFFCURVESTOOLS_EFFCURVESTOOL_H
#define EFFCURVESTOOLS_EFFCURVESTOOL_H

#include "TEfficiency.h"
#include "TH1F.h"

#include <map>
#include <string>

#include <EventLoop/Worker.h>
#include "xAODBase/IParticle.h"
#include "xAODJet/Jet.h"
#include "xAODTau/TauJet.h"

class EffCurvesTool

{
  public:
    EffCurvesTool(const std::string& name);
    virtual ~EffCurvesTool(){};

    bool fill_hadhad(bool pass, const xAOD::TauJet* t1, const xAOD::TauJet* t2, const xAOD::Jet* j1);

    bool fill_lephad(bool pass, const xAOD::TauJet* t1);

    std::map<std::string, TEfficiency*> Efficiencies() {
        return m_eff;
    }

    void record(EL::Worker* wk);

  private:
    std::map<std::string, TEfficiency*> m_eff;  //!
};

#endif
