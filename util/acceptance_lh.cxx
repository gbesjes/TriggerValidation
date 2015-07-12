// Dear emacs, this is -*- c++ -*-
// vim: ts=2 sw=2
// $Id$

// System include(s):
#include <memory>
#include <set>
#include <string>

// ROOT include(s):
#include <TChain.h>
#include <TFile.h>
#include <TError.h>
#include <TSystem.h>
#include <TH1.h>

// Core EDM include(s):
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"

// EDM includes
#include "xAODBase/IParticle.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODCore/AuxContainerBase.h"

// ROOT ACCESS Includes
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

// tools
#include "AssociationUtils/OverlapRemovalTool.h"
#include "TauAnalysisTools/TauSelectionTool.h"
#include "TauAnalysisTools/TauTruthMatchingTool.h"
#include "TrigTauMatching/TrigTauMatching.h"
#include "MuonSelectorTools/MuonSelectionTool.h"
#include "ElectronPhotonSelectorTools/AsgElectronLikelihoodTool.h"

// Trigger Decision Tool
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"


// Local stuff
#include "TriggerValidation/EffCurvesTool.h"
#include "TriggerValidation/Utils.h"

int main(int argc, char **argv) {


  // Get the name of the application:
  const char* APP_NAME = "acceptance_lh";

  // Initialise the environment:
  RETURN_CHECK( APP_NAME, xAOD::Init( APP_NAME ) );

  static const char* FNAME = 
    "/afs/cern.ch/user/q/qbuat/work/public/"
    "mc15_13TeV/mc15_13TeV.341124.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_tautauhh."
    "merge.AOD.e3935_s2608_s2183_r6630_r6264/AOD.05569772._000004.pool.root.1";
 
  std::vector<std::string> filenames;
  if(argc < 2){
    filenames.push_back(std::string(FNAME));
  } else {
    filenames = Utils::splitNames(argv[1]);
  }

  // Create the TEvent object
  xAOD::TEvent event(xAOD::TEvent::kClassAccess);
  xAOD::TStore store;

  ::TChain chain1("CollectionTree");
  for(auto fname : filenames){
    chain1.Add(fname.c_str());
  }

  RETURN_CHECK(APP_NAME, event.readFrom(&chain1));

  //Set up TDT 
  TrigConf::xAODConfigTool configTool("TrigConf::xAODConfigTool");
  ToolHandle<TrigConf::ITrigConfigTool> configHandle(&configTool);
  CHECK(configHandle->initialize());

  // The decision tool
  Trig::TrigDecisionTool trigDecTool("TrigDecTool");
  CHECK(trigDecTool.setProperty("ConfigTool",configHandle));
  CHECK(trigDecTool.setProperty("TrigDecisionKey","xTrigDecision"));
  CHECK(trigDecTool.initialize());

  // Create and configure the tool
  OverlapRemovalTool orTool("OverlapRemovalTool");
  CHECK( orTool.initialize() );

  // Muon Selector Tool
  CP::MuonSelectionTool muonSelector("muonSelector");
  CHECK(muonSelector.setProperty("MaxEta", 2.5));
  // CHECK(muonSelector.setProperty("MuQuality", xAOD::Muon::Quality::Loose));
  CHECK(muonSelector.initialize());

  // Electron Selector Tool
  // AsgElectronLikelihoodTool electronSelector("electronSelector");
  // CHECK(electronSelector.initialize());

  // Tau Truth Matching Tool
  TauAnalysisTools::TauTruthMatchingTool truthMatchTool("truthMatchTool");
  CHECK(truthMatchTool.initialize());

  // Tau Selection Tool
  TauAnalysisTools::TauSelectionTool tauSelector("tauSelector");
  CHECK(tauSelector.initialize());

  // TrigTau matching tool
  Trig::TrigTauMatchingTool trigTauMatchingTool("TrigTauMatchingTool");
  CHECK(trigTauMatchingTool.setProperty("TrigDecisionTool", ToolHandle<Trig::TrigDecisionTool>(&trigDecTool)));
  CHECK(trigTauMatchingTool.setProperty("HLTLabel", "TrigTauRecMerged"));
  CHECK(trigTauMatchingTool.initialize());

  std::vector<std::string> triggers;
  triggers.push_back("HLT_tau25_perf_ptonly");
  triggers.push_back("HLT_tau25_medium1_ptonly");
  triggers.push_back("HLT_tau25_perf_tracktwo");
  triggers.push_back("HLT_tau25_medium1_tracktwo");


  TH1F h("h_acceptance", "h_acceptance", triggers.size() + 1, 0, 1);
  h.GetXaxis()->SetBinLabel(1, "notrigger");
  for (int ib=0; ib < h.GetNbinsX() - 1; ib++)
    h.GetXaxis()->SetBinLabel(ib + 2, triggers[ib].c_str());

  std::map<std::string, EffCurvesTool*> curves_tools_nopt;
  std::map<std::string, EffCurvesTool*> curves_tools_nodr;
  std::map<std::string, EffCurvesTool*> curves_tools_final;
  for (auto trig: triggers) {
    curves_tools_nopt[trig] = new EffCurvesTool(trig + "_nopt");
    curves_tools_nodr[trig] = new EffCurvesTool(trig + "_nodr");
    curves_tools_final[trig] = new EffCurvesTool(trig + "_final");
  }

  Long64_t entries = event.getEntries();
  for (Long64_t entry = 0; entry < entries; entry++) {
     if ((entry%200)==0)
       ::Info(APP_NAME, "Start processing event %d", (int)entry);
    // ::Info(APP_NAME, "Start processing event %d", (int)entry);

    event.getEntry(entry);

    // retrieve the EDM objects
    const xAOD::EventInfo * ei = 0;
    CHECK(event.retrieve(ei, "EventInfo"));

    const xAOD::TauJetContainer* taus = 0;
    CHECK(event.retrieve(taus, "TauJets"));

    const xAOD::JetContainer* jets = 0;
    CHECK(event.retrieve(jets, "AntiKt4LCTopoJets"));
    
    const xAOD::ElectronContainer* electrons = 0;
    CHECK(event.retrieve(electrons, "Electrons"));

    const xAOD::MuonContainer* muons = 0;
    CHECK(event.retrieve(muons, "Muons"));

    CHECK(truthMatchTool.initializeEvent());


    // ---->>>   Muons 
    xAOD::MuonContainer* selected_muons = new xAOD::MuonContainer();
    xAOD::AuxContainerBase* selected_muons_aux = new xAOD::AuxContainerBase();
    selected_muons->setStore(selected_muons_aux);

    for (const auto muon: *muons) {
      if (not muonSelector.accept(muon))
	continue;
      selectDec(*muon) = true;
      xAOD::Muon * new_muon = new xAOD::Muon();
      new_muon->makePrivateStore(*muon);
      selected_muons->push_back(new_muon);
    }
    selected_muons->sort(Utils::comparePt);
    // ------------------

    // ---->>> Electrons 
    xAOD::ElectronContainer* selected_electrons = new xAOD::ElectronContainer();
    xAOD::AuxContainerBase* selected_electrons_aux = new xAOD::AuxContainerBase();
    selected_electrons->setStore(selected_electrons_aux);

    for (const auto electron: *electrons) {
      // if (not electronSelector.accept(electron))
      // 	continue;
      selectDec(*electron) = true;
      xAOD::Electron * new_electron = new xAOD::Electron();
      new_electron->makePrivateStore(*electron);
      selected_electrons->push_back(new_electron);
    }
    selected_electrons->sort(Utils::comparePt);
    // ------------------

    if (selected_muons->size() < 1 or selected_electrons->size() < 1)
      continue;

    // ---->>>   Taus 
    xAOD::TauJetContainer* selected_taus = new xAOD::TauJetContainer();
    xAOD::AuxContainerBase* selected_taus_aux = new xAOD::AuxContainerBase();
    selected_taus->setStore(selected_taus_aux);

    for (const auto tau: *taus) {
      if (not tauSelector.accept(tau))
	continue;
      selectDec(*tau) = true;
      xAOD::TauJet * new_tau = new xAOD::TauJet();
      new_tau->makePrivateStore(*tau);
      selected_taus->push_back(new_tau);
    }
    if (selected_taus->size() < 1)
      continue;

    selected_taus->sort(Utils::comparePt);
    xAOD::TauJet* tau1 = selected_taus->at(0);

      
    


    auto* truth_tau1 = truthMatchTool.applyTruthMatch(*tau1);
    if (truth_tau1 == NULL)
      continue;
    // --------------

    for (auto trig: triggers) {
      bool pass = trigDecTool.isPassed(trig) and trigTauMatchingTool.match(tau1, trig);
      curves_tools_nopt[trig]->fill_lephad(pass, tau1);
    }

    if (tau1->pt() < 25000.)
      continue;
    
    h.Fill("notrigger", 1);
    for (auto trig: triggers) {
      bool pass = trigDecTool.isPassed(trig) and trigTauMatchingTool.match(tau1, trig);
      curves_tools_final[trig]->fill_lephad(pass, tau1);
      if (pass) 
	h.Fill(trig.c_str(), 1);
    }


    // // ---->>> Jets
    // xAOD::JetContainer* selected_jets = new xAOD::JetContainer();
    // xAOD::AuxContainerBase* selected_jets_aux = new xAOD::AuxContainerBase();
    // selected_jets->setStore(selected_jets_aux);
    // for (const auto jet: *jets) {
    //   selectDec(*jet) = true;
    //   xAOD::Jet* new_jet = new xAOD::Jet();
    //   new_jet->makePrivateStore(*jet);
    //   selected_jets->push_back(new_jet);
    // }

    // if (selected_jets->size() < 1)
    //   continue;

    // selected_jets->sort(Utils::comparePt);
    // // --------------

    // // tau - jet overlap removal
    // CHECK(orTool.removeOverlaps(selected_electrons, selected_muons, selected_jets, selected_taus));



  } // loop over all the events


  TFile fout("acceptance.root", "RECREATE");
  h.Write();
  for (auto it: curves_tools_nopt) 
    for (auto tool: (it.second)->Efficiencies()) 
      (tool.second)->Write();
  for (auto it: curves_tools_nodr) 
    for (auto tool: (it.second)->Efficiencies()) 
      (tool.second)->Write();
  for (auto it: curves_tools_final) 
    for (auto tool: (it.second)->Efficiencies()) 
      (tool.second)->Write();
  fout.Close();

}

