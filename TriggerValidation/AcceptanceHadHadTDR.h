#ifndef TriggerValidation_AcceptanceHadHadTDR_H
#define TriggerValidation_AcceptanceHadHadTDR_H


#include <EventLoop/Algorithm.h>
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"

// tools
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TauAnalysisTools/TauTruthMatchingTool.h"

// Local stuff
#include "TriggerValidation/HistogramsBook.h"

#include <map>
#include "TH1F.h"
#include "TH2F.h"
#include "TEfficiency.h"

class AcceptanceHadHadTDR : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
 private:
  HistogramsBook m_book;

 public:
  // float cutValue;
  //
  int l1_min;
  int l1_step;
  int l1_nsteps;

  int off_step;
  int off_nsteps;

  // Cuts 
  float tau1_pt;
  float tau2_pt;

  float min_dr_tautau;
  float max_dr_tautau;

  int n_jets;
  float jet1_pt;
  float jet2_pt;
  float jet_eta;
  bool do_vbf_sel;
  float delta_eta_jj;

  std::vector<std::string> triggers;

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
 public:
  // Tree *myTree; //!
  // TH1 *myHist; //!
  TH2F * map_l1; //!
  TH2F * map_off; //!
  TH2F * map_l1taus; //!

  std::map<std::string, TH1F*> hists; //!

  Trig::TrigDecisionTool *m_trigDecisionTool; //!
  TrigConf::xAODConfigTool *m_trigConfigTool; //!
  TauAnalysisTools::TauTruthMatchingTool *m_t2mt; //!

  // this is a standard constructor
  AcceptanceHadHadTDR ();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  virtual EL::StatusCode select_taus(xAOD::TauJetContainer *selected_taus, const xAOD::TauJetContainer *taus);
  virtual EL::StatusCode select_jets(xAOD::JetContainer *selected_jets, const xAOD::JetContainer *jets, const xAOD::TauJet *tau1, const xAOD::TauJet *tau2);
  virtual EL::StatusCode select_l1taus(xAOD::EmTauRoIContainer *selected_l1taus, const xAOD::EmTauRoIContainer *l1taus);


  // this is needed to distribute the algorithm to the workers
  ClassDef(AcceptanceHadHadTDR, 1);
};

#endif
