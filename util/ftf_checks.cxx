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
#include <TH1F.h>
#include <TH2F.h>

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

bool match_cont_to_tau(const xAOD::TauJet * tau, const xAOD::TrackParticleContainer * cont)
{
  for(auto track: *cont) {
    if (track->p4().DeltaR(tau->p4()) < 0.4)
      return true;
  }
  return false;
}

const xAOD::TrackParticleContainer * get_matched_cont(const xAOD::TauJet * tau, std::vector< Trig::AsgFeature<xAOD::TrackParticleContainer> >  feat)
{
  for(auto &trackContainer: feat) {
    if(!trackContainer.cptr())
      continue;
    bool match =  match_cont_to_tau(tau, trackContainer.cptr());
    if (match)
      return trackContainer.cptr();
  }
  return 0;
}


int main(int argc, char **argv) {

  auto* map = new TH2F("iso_core", "iso_core", 10, 0, 10, 10, 0, 10);
  auto* tau_pt = new TH1F("tau_pt", "tau_pt", 20, 0, 100);
  auto* tau_eta = new TH1F("tau_eta", "tau_eta", 50, -2.5, 2.5);
  auto* tau_phi = new TH1F("tau_phi", "tau_phi", 50, -3.15, 3.15);
  auto* tau_ntracks = new TH1F("tau_ntracks", "tau_ntracks", 7, 0, 7);
  auto* tau_nprongs = new TH1F("tau_nprongs", "tau_nprongs", 5, 0, 5);
  auto* tau_nftf_core = new TH1F("tau_nftf_core", "tau_nftf_core", 15, 0, 15);
  // Get the name of the application:
  const char* APP_NAME = "ftf_check";

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
  Trig::TrigDecisionTool trigDecTool("TrigDecTool");
  CHECK(trigDecTool.setProperty("ConfigTool",configHandle));
  //  trigDecTool.setProperty("OutputLevel", MSG::VERBOSE);
  CHECK(trigDecTool.setProperty("TrigDecisionKey","xTrigDecision"));
  CHECK(trigDecTool.initialize());


  // Tau Truth Matching Tool
  TauAnalysisTools::TauTruthMatchingTool truthMatchTool("truthMatchTool");
  CHECK(truthMatchTool.initialize());


  Trig::TrigTauMatchingTool trigTauMatchingTool("TrigTauMatchingTool");
  CHECK(trigTauMatchingTool.setProperty("TrigDecisionTool", ToolHandle<Trig::TrigDecisionTool>(&trigDecTool)));
  CHECK(trigTauMatchingTool.setProperty("HLTLabel", "TrigTauRecMerged"));
  CHECK(trigTauMatchingTool.initialize());


  std::string trig = "HLT_tau25_idperf_tracktwo";

  Long64_t entries = event.getEntries();
  for (Long64_t entry = 0; entry < entries; entry++) {
    if ((entry%200)==0)
      ::Info(APP_NAME, "Start processing event %d", (int)entry);
    // ::Info(APP_NAME, "---------- Start processing event %d ----------", (int)entry);

    event.getEntry(entry);

    // retrieve the EDM objects
    const xAOD::EventInfo * ei = 0;
    CHECK(event.retrieve(ei, "EventInfo"));

    const xAOD::TauJetContainer* taus = 0;
    CHECK(event.retrieve(taus, "TauJets"));


    CHECK(truthMatchTool.initializeEvent());

    auto cg = trigDecTool.getChainGroup(trig);
    auto features = cg->features();

    auto HltTauFeatures = features.containerFeature<xAOD::TauJetContainer>("TrigTauRecMerged");
    auto preselTracksIsoFeatures  = features.containerFeature<xAOD::TrackParticleContainer>("InDetTrigTrackingxAODCnv_TauIso_FTF");
    auto preselTracksCoreFeatures = features.containerFeature<xAOD::TrackParticleContainer>("InDetTrigTrackingxAODCnv_TauCore_FTF");

    if (HltTauFeatures.size() != 1)
      continue;


    for (const auto tau: *taus) {

      auto* truth_tau = truthMatchTool.applyTruthMatch(*tau);
      if (truth_tau == NULL)
        continue;

      auto* hlt_tau = trigTauMatchingTool.getHLT(tau, trig);

      if (hlt_tau == 0)
        continue;

      // ::Info(APP_NAME, "tau %d: pt = %f, eta = %f, phi = %f", 
      // 	     (int)tau->index(), (float)tau->pt(), (float)tau->eta(), (float)tau->phi());


      auto ftf_core_cont = preselTracksCoreFeatures[0].cptr();
      auto ftf_iso_cont = preselTracksIsoFeatures[0].cptr();


      // // ::Info(APP_NAME, "TrackIso scan, features vector size = %d", (int)preselTracksIsoFeatures.size());
      // for(auto &trackContainer: preselTracksIsoFeatures) {
      // 	if(!trackContainer.cptr())
      // 	  continue;
      // 	// ::Info(APP_NAME, "Container size = %d", (int)trackContainer.cptr()->size());
      // 	bool match = match_cont_to_tau(tau, trackContainer.cptr());
      // 	if (match) {
      // 	  ::Info(APP_NAME, "found a TrackIso matching");
      // 	  // for(auto track: *trackContainer.cptr()) {
      // 	  //   ::Info(APP_NAME, "track %d: pt = %f, eta = %f, phi = %f", (int)track->index(), (float)track->pt(), (float)track->eta(), (float)track->phi());
      // 	  // }
      // 	}
      // }

      // // ::Info(APP_NAME, "TrackCore scan, features vector size = %d", (int)preselTracksCoreFeatures.size());
      // for(auto &trackContainer: preselTracksCoreFeatures) {
      // 	if(!trackContainer.cptr())
      // 	  continue;
      // 	// ::Info(APP_NAME, "Container size = %d", (int)trackContainer.cptr()->size());
      // 	bool match = match_cont_to_tau(tau, trackContainer.cptr());
      // 	if (match) {
      // 	  ::Info(APP_NAME, "found a TrackCore matching");
      // 	  // for(auto track: *trackContainer.cptr()) {
      // 	  //   ::Info(APP_NAME, "track %d: pt = %f, eta = %f, phi = %f", (int)track->index(), (float)track->pt(), (float)track->eta(), (float)track->phi());
      // 	  // }
      // 	}
      // }

      // auto ftf_iso_cont = get_matched_cont(tau, preselTracksIsoFeatures);
      // if (ftf_iso_cont == NULL)
      // 	continue;
      // auto ftf_core_cont = get_matched_cont(tau, preselTracksCoreFeatures);
      // if (ftf_core_cont == NULL)
      // 	continue;

      // ::Info(APP_NAME, "FTF Core features = %d / FTF Iso features = %d", (int)preselTracksCoreFeatures.size(), (int)preselTracksIsoFeatures.size());
      // ::Info(APP_NAME, "FTF Core size = %d / FTF Iso size = %d", (int)ftf_core_cont->size(), (int)ftf_iso_cont->size());
      if (ftf_iso_cont->size() == 0) {
        ::Info(APP_NAME, "FTF Core size = %d / FTF Iso size = %d", (int)ftf_core_cont->size(), (int)ftf_iso_cont->size());
        ::Info(APP_NAME, "Truth tau %d: nprongs = %d", (int)truth_tau->index(), (int)truth_tau->auxdata<size_t>("numCharged"));
        ::Info(APP_NAME, "Tau %d: ntracks = %d", (int)tau->index(), (int)tau->nTracks());
        for (auto tr: *ftf_core_cont) 
          ::Info(APP_NAME, "track %d: pt = %f, eta = %f, phi = %f, d0 = %f", (int)tr->index(), (float)tr->pt(), (float)tr->eta(), (float)tr->phi(), (float)tr->d0());
        tau_pt->Fill(tau->pt() / 1000.);
        tau_eta->Fill(tau->eta());
        tau_phi->Fill(tau->phi());
        tau_ntracks->Fill(tau->nTracks());
        tau_nprongs->Fill(truth_tau->auxdata<size_t>("numCharged"));
        tau_nftf_core->Fill(ftf_core_cont->size());
      }
      map->Fill((int)ftf_core_cont->size(), (int)ftf_iso_cont->size());

    } // end the loop over taus
  } // loop over all the events

  TFile fout("iso_core.root", "recreate");
  map->Write();
  tau_pt->Write();
  tau_eta->Write();
  tau_phi->Write();
  tau_ntracks->Write();
  tau_nprongs->Write();
  tau_nftf_core->Write();
  fout.Close();



}

