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
#include "xAODCore/AuxContainerBase.h"
#include "xAODTrigger/JetRoIContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "xAODTrigger/EnergySumRoI.h"
#include "xAODTracking/TrackParticle.h"

// ROOT ACCESS Includes
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

// tools
#include "AssociationUtils/OverlapRemovalTool.h"
#include "TauAnalysisTools/TauTruthMatchingTool.h"
#include "TrigTauMatching/TrigTauMatching.h"

// Trigger Decision Tool
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

// Local stuff
#include "TriggerValidation/EffCurvesTool.h"
#include "TriggerValidation/Utils.h"


using namespace TrigConf;
using namespace Trig;



int main(int argc, char **argv) {


  // Get the name of the application:
  const char* APP_NAME = "acceptance";

  // Initialise the environment:
  RETURN_CHECK(APP_NAME, xAOD::Init(APP_NAME));

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

  //Set up TDT for testing
  //add config tool
  TrigConf::xAODConfigTool configTool("TrigConf::xAODConfigTool");
  ToolHandle<TrigConf::ITrigConfigTool> configHandle(&configTool);
  CHECK(configHandle->initialize());

  // The decision tool
  TrigDecisionTool trigDecTool("TrigDecTool");
  CHECK(trigDecTool.setProperty("ConfigTool",configHandle));
  //  trigDecTool.setProperty("OutputLevel", MSG::VERBOSE);
  CHECK(trigDecTool.setProperty("TrigDecisionKey","xTrigDecision"));
  CHECK(trigDecTool.initialize());



  // Create and configure the tool
  OverlapRemovalTool orTool("OverlapRemovalTool");
  // Initialize the tool
  CHECK( orTool.initialize() );

  // Tau Truth Matching Tool
  TauAnalysisTools::TauTruthMatchingTool truthMatchTool("truthMatchTool");
  CHECK(truthMatchTool.initialize());


  Trig::TrigTauMatchingTool trigTauMatchingTool("TrigTauMatchingTool");
  CHECK(trigTauMatchingTool.setProperty("TrigDecisionTool", ToolHandle<Trig::TrigDecisionTool>(&trigDecTool)));
  CHECK(trigTauMatchingTool.setProperty("HLTLabel", "TrigTauRecMerged"));
  CHECK(trigTauMatchingTool.initialize());

  std::vector<std::string> triggers;
  // triggers.push_back("L1_TAU12");
  // triggers.push_back("L1_TAU60");
  // triggers.push_back("L1_TAU20_2TAU12");
  // triggers.push_back("L1_TAU20IM_2TAU12IM");
  // triggers.push_back("L1_TAU20ITAU12I-J25");
  // triggers.push_back("L1_TAU20IM_2TAU12IM_J25_2J20_3J12");
  // triggers.push_back("L1_J25_2J20_3J12_DR-TAU20ITAU12I");
  // triggers.push_back("L1_DR-TAU20ITAU12I");
  // triggers.push_back("L1_DR-TAU20ITAU12I-J25");
  triggers.push_back("HLT_tau35_perf_ptonly_tau25_perf_ptonly_L1TAU20IM_2TAU12IM");
  triggers.push_back("HLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo_L1TAU20IM_2TAU12IM");
  triggers.push_back("HLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo");
  triggers.push_back("HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM");
  triggers.push_back("HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1DR-TAU20ITAU12I-J25");
  triggers.push_back("HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo");
  triggers.push_back("HLT_tau35_tight1_tracktwo_tau25_tight1_tracktwo");
  
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

    event.getEntry(entry);

    // retrieve the EDM objects
    const xAOD::EventInfo * ei = 0;
    CHECK(event.retrieve(ei, "EventInfo"));

    const xAOD::TauJetContainer* taus = 0;
    CHECK(event.retrieve(taus, "TauJets"));

    const xAOD::JetContainer* jets = 0;
    CHECK(event.retrieve(jets, "AntiKt4LCTopoJets"));


    CHECK(truthMatchTool.initializeEvent());

    xAOD::TauJetContainer* selected_taus = new xAOD::TauJetContainer();
    xAOD::AuxContainerBase* selected_taus_aux = new xAOD::AuxContainerBase();
    selected_taus->setStore(selected_taus_aux);

    for (const auto tau: *taus) {

      // // pt cut
      // if (tau->pt() < 30000.) 
      // 	continue;
      // eta cut
      if (fabs(tau->eta()) > 2.5)
	continue;

      if (fabs(tau->eta()) > 1.37 and fabs(tau->eta()) < 1.52)
	continue;
      
      // 1 or 3 tracks
      if (tau->nTracks() != 1 and tau->nTracks() != 3)
	continue;

      // ID cut
      if (not tau->isTau(xAOD::TauJetParameters::JetBDTSigMedium))
	continue;
      
      selectDec(*tau) = true;
      xAOD::TauJet * new_tau = new xAOD::TauJet();
      new_tau->makePrivateStore(*tau);
      selected_taus->push_back(new_tau);
    }

    if (selected_taus->size() < 2)
      continue;

    selected_taus->sort(Utils::comparePt);

    xAOD::TauJet* tau1 = selected_taus->at(0);
    xAOD::TauJet* tau2 = selected_taus->at(1);

    auto* truth_tau1 = truthMatchTool.applyTruthMatch(*tau1);
    auto* truth_tau2 = truthMatchTool.applyTruthMatch(*tau2);

    if (truth_tau1 == NULL or truth_tau2 == NULL)
      continue;

    for (const auto jet: *jets)
      selectDec(*jet) = true;
    // tau - jet overlap removal
    CHECK(orTool.removeTauJetOverlap(*selected_taus, *jets));

    xAOD::JetContainer* selected_jets = new xAOD::JetContainer();
    xAOD::AuxContainerBase* selected_jets_aux = new xAOD::AuxContainerBase();
    selected_jets->setStore(selected_jets_aux);

    for (const auto jet: *jets) {
      
      // // pt cut
      // if (jet->pt() < 50000.) 
      // 	continue;

      if (overlapAcc(*jet))
	continue;

      xAOD::Jet* new_jet = new xAOD::Jet();
      new_jet->makePrivateStore(*jet);
      selected_jets->push_back(new_jet);
    }

    if (selected_jets->size() < 1)
      continue;

    selected_jets->sort(Utils::comparePt);

    for (auto trig: triggers) {
      bool pass = trigDecTool.isPassed(trig) and trigTauMatchingTool.match(tau1, trig) and trigTauMatchingTool.match(tau2, trig);
      curves_tools_nopt[trig]->fill_hadhad(pass, tau1, tau2, selected_jets->at(0));
    }
    
    if (tau1->pt() < 40000. or tau2->pt() < 30000. or selected_jets->at(0)->pt() < 60000.)
      continue;

    for (auto trig: triggers) {
      bool pass = trigDecTool.isPassed(trig) and trigTauMatchingTool.match(tau1, trig) and trigTauMatchingTool.match(tau2, trig);
	curves_tools_nodr[trig]->fill_hadhad(pass, tau1, tau2, selected_jets->at(0));
    }

    double delta_r = tau1->p4().DeltaR(tau2->p4());
    if (delta_r < 0.8 or delta_r > 2.4)
      continue;

    h.Fill("notrigger", 1);
    for (auto trig: triggers) {
      bool pass = trigDecTool.isPassed(trig) and trigTauMatchingTool.match(tau1, trig) and trigTauMatchingTool.match(tau2, trig);
      curves_tools_final[trig]->fill_hadhad(pass, tau1, tau2, selected_jets->at(0));
      if (pass) 
	h.Fill(trig.c_str(), 1);
    }


    // if (not trigDecTool.isPassed("HLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo_L1TAU20IM_2TAU12IM") and \
    // 	trigDecTool.isPassed("HLT_tau35_perf_ptonly_tau25_perf_ptonly_L1TAU20IM_2TAU12IM")) {
    //   std::cout << " " <<  std::endl;
    //   std::cout << "-----------------------" <<  std::endl;
    //   std::cout << "Entry = " << entry 
    // 		<< ", RunNumber = " << ei->runNumber() 
    // 		<< " , EventNumber = " << ei->eventNumber() << std::endl;
    //   std::cout << "Not pass HLT_tau35_loose1_tracktwo_tau25_loose1_tracktwo_L1TAU20IM_2TAU12IM but pass " \
    // 	"HLT_tau35_perf_ptonly_tau25_perf_ptonly_L1TAU20IM_2TAU12IM" << std::endl;
    //   std::cout << "pt | eta | phi | nTracks | mediumid" << std::endl;
    //   std::cout << "-- offline -- " << std::endl;
    //   std::cout << tau1->pt() << " | " 
    // 		<< tau1->eta() << " | " 
    // 		<< tau1->phi() << " | " 
    // 		<< tau1->nTracks() << " | "
    // 		<< tau1->isTau(xAOD::TauJetParameters::JetBDTSigMedium)
    // 		<< std::endl;
    //   std::cout << tau2->pt() << " | " 
    // 		<< tau2->eta() << " | " 
    // 		<< tau2->phi() << " | " 
    // 		<< tau2->nTracks() << " | "
    // 		<< tau2->isTau(xAOD::TauJetParameters::JetBDTSigMedium)
    // 		<< std::endl;
    //   std::cout << "-- truth -- " << std::endl;
    //   std::cout << truth_tau1->auxdataConst<double>("pt_vis") << " | " 
    // 		<< truth_tau1->auxdataConst<double>("eta_vis") << " | " 
    // 		<< truth_tau1->auxdataConst<double>("phi_vis") << " | " 
    // 		// << tau1->nTracks() << " | "
    // 		// << tau1->isTau(xAOD::TauJetParameters::JetBDTSigMedium)
    // 		<< std::endl;
    //   std::cout << truth_tau2->auxdataConst<double>("pt_vis") << " | " 
    // 		<< truth_tau2->auxdataConst<double>("eta_vis") << " | " 
    // 		<< truth_tau2->auxdataConst<double>("phi_vis") << " | " 
    // 		// << tau1->nTracks() << " | "
    // 		// << tau1->isTau(xAOD::TauJetParameters::JetBDTSigMedium)
    // 		<< std::endl;
    //   std::cout <<  " -- hlt features -- " << std::endl;
    //   auto cg = trigDecTool.getChainGroup("HLT_tau35_perf_ptonly_tau25_perf_ptonly_L1TAU20IM_2TAU12IM");
    //   auto features = cg->features();
    //   auto tauHltFeatures = features.containerFeature<xAOD::TauJetContainer>("TrigTauRecMerged");
    //   for (auto &tauContainer: tauHltFeatures) {
    // 	if (tauContainer.cptr()) {
    // 	  for (auto tau: *tauContainer.cptr()) {
    // 	    std::cout << tau->pt() << " | " 
    // 		      << tau->eta() << " | " 
    // 		      << tau->phi() << " | " 
    // 		      << tau->nTracks() << " | "
    // 		      << tau->isTau(xAOD::TauJetParameters::JetBDTSigMedium)
    // 		      << std::endl;
    // 	  }
    // 	}
    //   }
    //   std::cout << "-----------------------" <<  std::endl;
    //   std::cout << " " <<  std::endl;

    // }

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

