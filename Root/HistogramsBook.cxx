#include "TriggerValidation/HistogramsBook.h"

HistogramsBook::HistogramsBook(const std::string & name): m_name(name)
{}


void HistogramsBook::book()
{
  // tau1 
  m_h1d["tau1_pt"]      = new TH1F((m_name + "/h_tau1_pt").c_str(),  "tau1_pt", 40, 0, 100);
  m_h1d["tau1_eta"]     = new TH1F((m_name + "/h_tau1_eta").c_str(), "tau1_eta", 10, -2.5, 2.5);
  m_h1d["tau1_phi"]     = new TH1F((m_name + "/h_tau1_phi").c_str(), "tau1_phi", 10, -3.15, 3.15);
  m_h1d["tau1_ntracks"] = new TH1F((m_name + "/h_tau1_ntracks").c_str(), "tau1_ntracks", 5, 0, 5);
  m_h1d["tau1_bdt"]     = new TH1F((m_name + "/h_tau1_bdt").c_str(), "tau1_bdt", 10, 0, 1);

  // tau 2
  m_h1d["tau2_pt"]      = new TH1F((m_name + "/h_tau2_pt").c_str(), "tau2_pt", 40, 0, 100);
  m_h1d["tau2_eta"]     = new TH1F((m_name + "/h_tau2_eta").c_str(), "tau2_eta", 10, -2.5, 2.5);
  m_h1d["tau2_phi"]     = new TH1F((m_name + "/h_tau2_phi").c_str(), "tau2_phi", 10, -3.15, 3.15);
  m_h1d["tau2_ntracks"] = new TH1F((m_name + "/h_tau2_ntracks").c_str(), "tau2_ntracks", 5, 0, 5);
  m_h1d["tau2_bdt"]     = new TH1F((m_name + "/h_tau2_bdt").c_str(), "tau2_bdt", 10, 0, 1);

  // jet1 
  m_h1d["jet1_pt"]      = new TH1F((m_name + "/h_jet1_pt").c_str(),  "jet1_pt", 40, 0, 100);
  m_h1d["jet1_eta"]     = new TH1F((m_name + "/h_jet1_eta").c_str(), "jet1_eta", 10, -2.5, 2.5);
  m_h1d["jet1_phi"]     = new TH1F((m_name + "/h_jet1_phi").c_str(), "jet1_phi", 10, -3.15, 3.15);

  // jet 2
  m_h1d["jet2_pt"]      = new TH1F((m_name + "/h_jet2_pt").c_str(), "jet2_pt", 40, 0, 100);
  m_h1d["jet2_eta"]     = new TH1F((m_name + "/h_jet2_eta").c_str(), "jet2_eta", 10, -2.5, 2.5);
  m_h1d["jet2_phi"]     = new TH1F((m_name + "/h_jet2_phi").c_str(), "jet2_phi", 10, -3.15, 3.15);

  // truth_tau1 
  m_h1d["truth_tau1_pt"]      = new TH1F((m_name + "/h_truth_tau1_pt").c_str(),  "truth_tau1_pt", 40, 0, 100);
  m_h1d["truth_tau1_eta"]     = new TH1F((m_name + "/h_truth_tau1_eta").c_str(), "truth_tau1_eta", 10, -2.5, 2.5);
  m_h1d["truth_tau1_phi"]     = new TH1F((m_name + "/h_truth_tau1_phi").c_str(), "truth_tau1_phi", 10, -3.15, 3.15);

  // truth_tau 2
  m_h1d["truth_tau2_pt"]      = new TH1F((m_name + "/h_truth_tau2_pt").c_str(), "truth_tau2_pt", 40, 0, 100);
  m_h1d["truth_tau2_eta"]     = new TH1F((m_name + "/h_truth_tau2_eta").c_str(), "truth_tau2_eta", 10, -2.5, 2.5);
  m_h1d["truth_tau2_phi"]     = new TH1F((m_name + "/h_truth_tau2_phi").c_str(), "truth_tau2_phi", 10, -3.15, 3.15);


  // tau-tau system
  m_h1d["tautau_dr"]    = new TH1F((m_name + "/h_tautau_dr").c_str(), "dr_tau_tau", 16, 0, 3.2); 

}


void HistogramsBook::fill_tau(const xAOD::TauJet * tau1, const xAOD::TauJet * tau2, const double & weight)
{

  if (tau1 != NULL) {
    m_h1d["tau1_pt"]->Fill(tau1->pt() / 1000., weight);
    m_h1d["tau1_eta"]->Fill(tau1->eta(), weight);
    m_h1d["tau1_phi"]->Fill(tau1->phi(), weight);
    m_h1d["tau1_ntracks"]->Fill(tau1->nTracks(), weight);
    m_h1d["tau1_bdt"]->Fill(tau1->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);
  }

  if (tau2 != NULL) {
    m_h1d["tau2_pt"]->Fill(tau2->pt() / 1000., weight);
    m_h1d["tau2_eta"]->Fill(tau2->eta(), weight);
    m_h1d["tau2_phi"]->Fill(tau2->phi(), weight);
    m_h1d["tau2_ntracks"]->Fill(tau2->nTracks(), weight);
    m_h1d["tau2_bdt"]->Fill(tau2->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);
  }

  if (tau1 != NULL and tau2 != NULL) 
    m_h1d["tautau_dr"]->Fill(tau1->p4().DeltaR(tau2->p4()), weight);
}


void HistogramsBook::fill_jet(const xAOD::Jet * j1, const xAOD::Jet * j2, const double & weight)
{

  if (j1 != NULL) {
    m_h1d["jet1_pt"]->Fill(j1->pt() / 1000., weight);
    m_h1d["jet1_eta"]->Fill(j1->eta(), weight);
    m_h1d["jet1_phi"]->Fill(j1->phi(), weight);
  }

  if (j2 != NULL) {
    m_h1d["jet2_pt"]->Fill(j2->pt() / 1000., weight);
    m_h1d["jet2_eta"]->Fill(j2->eta(), weight);
    m_h1d["jet2_phi"]->Fill(j2->phi(), weight);
  }    

}

void HistogramsBook::fill_truth(const xAOD::TruthParticle * tau1, const xAOD::TruthParticle * tau2, const double & weight)
{

  if (tau1 != NULL) {
    m_h1d["truth_tau1_pt"] ->Fill(tau1->auxdata<double>("pt_vis") / 1000., weight);
    m_h1d["truth_tau1_eta"]->Fill(tau1->auxdata<double>("eta_vis"), weight);
    m_h1d["truth_tau1_phi"]->Fill(tau1->auxdata<double>("phi_vis"), weight);
  }

  if (tau2 != NULL) {
    m_h1d["truth_tau2_pt"] ->Fill(tau2->auxdata<double>("pt_vis") / 1000., weight);
    m_h1d["truth_tau2_eta"]->Fill(tau2->auxdata<double>("eta_vis"), weight);
    m_h1d["truth_tau2_phi"]->Fill(tau2->auxdata<double>("phi_vis"), weight);
  }    

}





void HistogramsBook::record(EL::Worker* wk)
{
  for (auto h: m_h1d) {
    std::cout << h.second->GetName() << std::endl;
    wk->addOutput(h.second);
  }
}
