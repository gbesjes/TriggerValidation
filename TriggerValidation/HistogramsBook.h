#ifndef TRIGGERVALIDATION_HISTOGRAMSBOOK_H
#define TRIGGERVALIDATION_HISTOGRAMSBOOK_H

#include <map>

#include "TH1F.h"

#include "xAODJet/Jet.h"
#include "xAODTau/TauJet.h"
#include "xAODTruth/TruthParticle.h"

#include "EventLoop/Worker.h"

class HistogramsBook

{
  public:
    HistogramsBook(const std::string& name);
    virtual ~HistogramsBook(){};

    void book();

    void fill_tau(const xAOD::TauJet* tau1, const xAOD::TauJet* tau2, const double& weight = 1.0);
    void fill_jet(const xAOD::Jet* j1, const xAOD::Jet* j2, const double& weight = 1.0);
    void fill_truth(const xAOD::TruthParticle* tau1, const xAOD::TruthParticle* tau2, const double& weight = 1.0);

    void record(EL::Worker* wk);

  private:
    std::string m_name;
    std::map<std::string, TH1F*> m_h1d;
};

#endif
