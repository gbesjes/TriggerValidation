
#include "TriggerValidation/EffCurvesTool.h"

EffCurvesTool::EffCurvesTool(const std::string & name)
{
  m_eff["leading_tau_pt"] = new TEfficiency(("lead_tau_pt_" + name).c_str() , "leading_tau", 16, 20, 100);
  m_eff["subleading_tau_pt"] = new TEfficiency(("sublead_tau_pt_" + name).c_str() , "subleading_tau", 16, 20, 100);
  m_eff["leading_tau_ntracks"] = new TEfficiency(("lead_tau_ntracks_" + name).c_str() , "leading_tau", 5, 0, 5);
  m_eff["subleading_tau_ntracks"] = new TEfficiency(("sublead_tau_ntracks_" + name).c_str() , "subleading_tau", 5, 0, 5);

  int Neta = 8;
  double bins_eta[] = {-2.4, -1.52, -1.37, -0.6, 0, 0.6, 1.37, 1.52, 2.4};
  m_eff["leading_tau_eta"] = new TEfficiency(("lead_tau_eta_" + name).c_str() , "leading_tau", Neta, bins_eta);
  m_eff["subleading_tau_eta"] = new TEfficiency(("sublead_tau_eta_" + name).c_str() , "subleading_tau", 20, -5, 5);

  m_eff["jet_pt"] = new TEfficiency(("jet_pt_" + name).c_str() , "leading_jet", 8, 20, 100);
  m_eff["jet_eta"] = new TEfficiency(("jet_eta_" + name).c_str() , "leading_jet", 10, -5, 5);

  m_eff["delta_r"] = new TEfficiency(("delta_r_" + name).c_str(), "delta_r", 8, 0, 3.2);

}

bool EffCurvesTool::fill_hadhad(bool pass, const xAOD::TauJet * t1, const xAOD::TauJet * t2, const xAOD::Jet * j1)

{

  m_eff["leading_tau_pt"]->Fill(pass, t1->pt() / 1000.);
  m_eff["leading_tau_eta"]->Fill(pass, t1->eta());
  m_eff["leading_tau_ntracks"]->Fill(pass, t1->nTracks());

  m_eff["subleading_tau_pt"]->Fill(pass, t2->pt() / 1000.);
  m_eff["subleading_tau_eta"]->Fill(pass, t2->eta());
  m_eff["subleading_tau_ntracks"]->Fill(pass, t2->nTracks());

  m_eff["jet_pt"]->Fill(pass, j1->pt() / 1000.);
  m_eff["jet_eta"]->Fill(pass, j1->eta());

  m_eff["delta_r"]->Fill(pass, t1->p4().DeltaR(t2->p4()));

  return true;
}


bool EffCurvesTool::fill_lephad(bool pass, const xAOD::TauJet * t1)
{

  m_eff["leading_tau_pt"]->Fill(pass, t1->pt() / 1000.);
  m_eff["leading_tau_eta"]->Fill(pass, t1->eta());
  m_eff["leading_tau_ntracks"]->Fill(pass, t1->nTracks());

  return true;
}


void EffCurvesTool::record(EL::Worker* wk)
{
  for (auto eff: m_eff) {
    std::cout << eff.second->GetName() << std::endl;
    // wk->addOutput((TH1F*)eff.second->GetPassedHistogram());
    // wk->addOutput((TH1F*)eff.second->GetTotalHistogram());
  }
}
