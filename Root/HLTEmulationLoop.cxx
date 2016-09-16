// vim: ts=2 sw=2

#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <TriggerValidation/HLTEmulationLoop.h>

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"

#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM includes
// Core EDM include(s):
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTrigger/JetRoIContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "xAODTrigger/EnergySumRoI.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

#include "TrigTauEmulation/EmTauSelectionTool.h"

#include "TrigTauEmulation/DecoratedHltTau.h"
#include "TrigTauEmulation/ToolsRegistry.h"
#include "TrigTauEmulation/MsgStream.h"

/// Helper macro for checking xAOD::TReturnCode return values
#define EL_RETURN_CHECK( CONTEXT, EXP )                     \
  do {                                                     \
    if( ! EXP.isSuccess() ) {                             \
      Error( CONTEXT,                                    \
          XAOD_MESSAGE( "Failed to execute: %s" ),    \
#EXP );                                     \
      return EL::StatusCode::FAILURE;                    \
    }                                                     \
  } while( false )


// this is needed to distribute the algorithm to the workers
ClassImp(HLTEmulationLoop)

// trim from start
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

static inline std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}

// helper to clear xAOD containers
template <typename T> void clearContainer(T &c){
  for(auto it: *c){
    it->clearDecorations();
    if(it->usingPrivateStore()){
      it->releasePrivateStore();
    }
  }
}


HLTEmulationLoop :: HLTEmulationLoop ()
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}



EL::StatusCode HLTEmulationLoop :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob ()", xAOD::Init());
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTEmulationLoop :: histInitialize ()
{

  h_TDT_EMU_diff = new TH1F("h_TDT_Emulation_differences", "TDT_Emulation_differences", chains_to_test.size(), 0, chains_to_test.size());
  h_TDT_fires = new TH1F("h_TDT_fires", "TDT_fires_total_number", chains_to_test.size(), 0, chains_to_test.size());
  h_EMU_fires = new TH1F("h_EMU_fires", "EMU_fires_total_number", chains_to_test.size(), 0, chains_to_test.size());

  for (unsigned int ich = 0; ich < chains_to_test.size(); ich++) {
    auto chain = chains_to_test[ich];
    h_TDT_EMU_diff->GetXaxis()->SetBinLabel(ich + 1, chain.c_str());
    h_TDT_fires->GetXaxis()->SetBinLabel(ich + 1, chain.c_str());
    h_EMU_fires->GetXaxis()->SetBinLabel(ich + 1, chain.c_str());
  }

  wk()->addOutput (h_TDT_EMU_diff);
  wk()->addOutput (h_TDT_fires);
  wk()->addOutput (h_EMU_fires);
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTEmulationLoop :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTEmulationLoop :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTEmulationLoop :: initialize ()
{

  trigger_condition = TrigDefs::Physics | TrigDefs::allowResurrectedDecision;

  // Initialize and configure trigger tools
  if (asg::ToolStore::contains<TrigConf::xAODConfigTool>("xAODConfigTool")) {
    m_trigConfigTool = asg::ToolStore::get<TrigConf::xAODConfigTool>("xAODConfigTool");
  } else {
    m_trigConfigTool = new TrigConf::xAODConfigTool("xAODConfigTool"); // gives us access to the meta-data
    EL_RETURN_CHECK("initialize", m_trigConfigTool->initialize());
  }

  if (asg::ToolStore::contains<Trig::TrigDecisionTool>("TrigDecTool")) {
    m_trigDecisionTool =  asg::ToolStore::get<Trig::TrigDecisionTool>("TrigDecTool");
  } else {
    ToolHandle< TrigConf::ITrigConfigTool > trigConfigHandle( m_trigConfigTool );
    m_trigDecisionTool = new Trig::TrigDecisionTool("TrigDecTool");
    EL_RETURN_CHECK("initialize", m_trigDecisionTool->setProperty("ConfigTool", trigConfigHandle));
    EL_RETURN_CHECK("initialize", m_trigDecisionTool->setProperty("TrigDecisionKey", "xTrigDecision"));
    EL_RETURN_CHECK("initialize", m_trigDecisionTool->initialize());
  }
  
  if(asg::ToolStore::contains<ChainRegistry>("ChainRegistry")) {
    m_chainRegistry = asg::ToolStore::get<ChainRegistry>("ChainRegistry");
  } else {
    m_chainRegistry = new ChainRegistry("ChainRegistry");
    m_chainRegistry->msg().setLevel(this->msg().level());
    EL_RETURN_CHECK("initialize", m_chainRegistry->initialize());
  }

  if(asg::ToolStore::contains<ToolsRegistry>("ToolsRegistry")) {
    m_registry = asg::ToolStore::get<ToolsRegistry>("ToolsRegistry");
  } else {
    m_registry = new ToolsRegistry("ToolsRegistry");
    EL_RETURN_CHECK("initialize", m_registry->setProperty("RecalculateBDTscore", false));
    m_registry->msg().setLevel(this->msg().level());
    EL_RETURN_CHECK("initialize", m_registry->initialize());
  }

  if (asg::ToolStore::contains<TrigTauEmul::Level1EmulationTool>("Level1TrigTauEmulator")) {
    m_l1_emulationTool = asg::ToolStore::get<TrigTauEmul::Level1EmulationTool>("Level1TrigTauEmulator");
  } else {
    m_l1_emulationTool = new TrigTauEmul::Level1EmulationTool("Level1TrigTauEmulator");
    EL_RETURN_CHECK("initialize", m_l1_emulationTool->setProperty("l1_chains", l1_chains));
    EL_RETURN_CHECK("initialize", m_l1_emulationTool->setProperty("useShallowCopies", false));
    EL_RETURN_CHECK("initialize", m_l1_emulationTool->initialize());
    m_l1_emulationTool->msg().setLevel(MSG::VERBOSE);
  }

  if (asg::ToolStore::contains<TrigTauEmul::HltEmulationTool>("HltTrigTauEmulator")) {
    m_hlt_emulationTool = asg::ToolStore::get<TrigTauEmul::HltEmulationTool>("HltTrigTauEmulator");
  } else {
    ToolHandle<TrigTauEmul::ILevel1EmulationTool> handle(m_l1_emulationTool);
    m_hlt_emulationTool = new TrigTauEmul::HltEmulationTool("HltTrigTauEmulator");
    EL_RETURN_CHECK("initialize", m_hlt_emulationTool->setProperty("hlt_chains", chains_to_test));
    EL_RETURN_CHECK("initialize", m_hlt_emulationTool->setProperty("PerformL1Emulation", true));
    EL_RETURN_CHECK("initialize", m_hlt_emulationTool->setProperty("Level1EmulationTool", handle));
    //EL_RETURN_CHECK("initialize", m_hlt_emulationTool->setProperty("HltTauTools", m_registry->GetHltTauTools()));
    EL_RETURN_CHECK("initialize", m_hlt_emulationTool->setProperty("TrigDecTool", "TrigDecTool"));
    EL_RETURN_CHECK("initialize", m_hlt_emulationTool->setProperty("L1TriggerCondition", trigger_condition));
    EL_RETURN_CHECK("initialize", m_hlt_emulationTool->setProperty("HLTTriggerCondition", trigger_condition));
    EL_RETURN_CHECK("initialize", m_hlt_emulationTool->initialize());
    m_hlt_emulationTool->msg().setLevel(MSG::VERBOSE);
  }

  xAOD::TEvent* event = wk()->xaodEvent();

  // MY_MSG_INFO("Number of events = " << event->getEntries());
  Info("initialize()", "Number of events = %lli", event->getEntries());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTEmulationLoop :: execute ()
{
  xAOD::TEvent* event = wk()->xaodEvent();
  MY_MSG_VERBOSE("--------------------------") ;
  MY_MSG_VERBOSE("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());
  MY_MSG_VERBOSE("--------------------------") ;
  
  //for (auto extension : m_registry->selectExtensions<EmTauSelectionTool*>()) {
    //std::cout << "GOT EXTENSION " << extension->name() << std::endl; 
  //}
  //for (auto extension : m_registry->selectExtensionsOfBaseType<IEmTauSelectionTool*>()) { 
    //std::cout << "GOT BASE EXTENSION " << extension->name() << std::endl; 
  //} 

  // retrieve the EDM objects
  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", event->retrieve(ei, "EventInfo"));

  const xAOD::EmTauRoIContainer *l1taus = 0;
  EL_RETURN_CHECK("execute", event->retrieve(l1taus, "LVL1EmTauRoIs"));

  const xAOD::JetRoIContainer *l1jets = 0;
  EL_RETURN_CHECK("execute", event->retrieve(l1jets, "LVL1JetRoIs"));

  const xAOD::MuonRoIContainer* l1muons = 0;
  EL_RETURN_CHECK("execute", event->retrieve(l1muons, "LVL1MuonRoIs"));

  const xAOD::EnergySumRoI* l1xe = 0;
  EL_RETURN_CHECK("execute", event->retrieve(l1xe, "LVL1EnergySumRoI"));


  if (not m_trigDecisionTool->isPassed(reference_chain, trigger_condition)) {
    return EL::StatusCode::SUCCESS;
  }

  auto cg = m_trigDecisionTool->getChainGroup(reference_chain);
  auto features = cg->features(trigger_condition);

  xAOD::TauJetContainer* presel_taus = new xAOD::TauJetContainer();
  xAOD::TauJetAuxContainer* presel_taus_aux = new xAOD::TauJetAuxContainer();
  presel_taus->setStore(presel_taus_aux);
  auto tauPreselFeatures = features.containerFeature<xAOD::TauJetContainer>("TrigTauRecPreselection");
  for (auto &tauContainer: tauPreselFeatures) {
    if (!tauContainer.cptr()) { continue; }
    for (auto tau: *tauContainer.cptr()) {
      xAOD::TauJet *new_tau = new xAOD::TauJet();
      new_tau->makePrivateStore(tau);
      presel_taus->push_back(new_tau);
    }
  }

  if (tauPreselFeatures.size() != 1)
    return EL::StatusCode::SUCCESS;

  //get the tracking info
  // NOTE: we should fix this in two ways
  // 1) only ask for this when needed (an idperf chain is the reference)
  // 2) ask it directly for the reference chain
  auto preselTracksIsoFeatures  = features.containerFeature<xAOD::TrackParticleContainer>("InDetTrigTrackingxAODCnv_TauIso_FTF");
  auto preselTracksCoreFeatures = features.containerFeature<xAOD::TrackParticleContainer>("InDetTrigTrackingxAODCnv_TauCore_FTF");

  xAOD::TrackParticleContainer *preselTracksIso  = new xAOD::TrackParticleContainer();
  xAOD::TrackParticleContainer *preselTracksCore = new xAOD::TrackParticleContainer();
  xAOD::TrackParticleAuxContainer *preselTracksIsoAux  = new xAOD::TrackParticleAuxContainer();
  xAOD::TrackParticleAuxContainer *preselTracksCoreAux = new xAOD::TrackParticleAuxContainer();

  preselTracksIso->setStore(preselTracksIsoAux);
  preselTracksCore->setStore(preselTracksCoreAux);

  MY_MSG_VERBOSE("Core Tracks containers size = " << preselTracksCoreFeatures.size());
  MY_MSG_VERBOSE("Iso Tracks containers size = " << preselTracksIsoFeatures.size());

  // iso tracks
  for(auto &trackContainer: preselTracksIsoFeatures) {
    if(!trackContainer.cptr()) { continue; }
    for(auto track: *trackContainer.cptr()) {
      xAOD::TrackParticle *new_track = new xAOD::TrackParticle();
      new_track->makePrivateStore(track);
      preselTracksIso->push_back(new_track);
    }
  }

  // core tracks
  for(auto &trackContainer: preselTracksCoreFeatures) {
    if(!trackContainer.cptr()) { continue; }
    for(auto track: *trackContainer.cptr()) {
      xAOD::TrackParticle *new_track = new xAOD::TrackParticle();
      new_track->makePrivateStore(track);
      preselTracksCore->push_back(new_track);
    }
  }

  xAOD::TauJetContainer* hlt_taus = new xAOD::TauJetContainer();
  xAOD::TauJetAuxContainer* hlt_taus_aux = new xAOD::TauJetAuxContainer();
  hlt_taus->setStore(hlt_taus_aux);

  // TODO: should be a flag
  std::string hltTauContainerName = "TrigTauRecMerged";
  auto tauHltFeatures = features.containerFeature<xAOD::TauJetContainer>(hltTauContainerName);
  MY_MSG_VERBOSE("HLT Tau containers size = " << tauHltFeatures.size());
  for (auto &tauContainer: tauHltFeatures) {
    if (!tauContainer.cptr()) { continue; }
    for (auto tau: *tauContainer.cptr()) {
      xAOD::TauJet *new_tau = new xAOD::TauJet();
      new_tau->makePrivateStore(tau);
      hlt_taus->push_back(new_tau);
    }
  }
  
  std::string caloOnlyTauContainerName = "HLT_xAOD__TauJetContainer_TrigTauRecCaloOnly";
  decltype(features.containerFeature<xAOD::TauJetContainer>()) tauCaloOnlyFeatures;
  // std::vector<Trig::AsgFeature<xAOD::TauJetContainer> > tauCaloOnlyFeatures;

  bool hasCaloOnlyTaus = event->contains<xAOD::TauJetContainer>(caloOnlyTauContainerName);
  MY_MSG_VERBOSE("hasCaloOnlyTaus = " << hasCaloOnlyTaus);
  if(hasCaloOnlyTaus){
    tauCaloOnlyFeatures = features.containerFeature<xAOD::TauJetContainer>(caloOnlyTauContainerName);
    MY_MSG_VERBOSE("CaloOnly Tau containers size = " << tauHltFeatures.size());
  }

  // make a bunch of decorated HLT taus
  std::vector<DecoratedHltTau> decoratedTaus;
  for (auto &tauContainer: tauHltFeatures) {
    if (!tauContainer.cptr()) { continue; }

    for(auto tau: *tauContainer.cptr()){
      xAOD::TauJet *new_tau = new xAOD::TauJet();
      new_tau->makePrivateStore(tau);
      DecoratedHltTau d(new_tau);

      //find the iso and core tracks for this guy
      for(auto &trackContainer: preselTracksIsoFeatures) {
        if( HLT::TrigNavStructure::haveCommonRoI(tauContainer.te(), trackContainer.te()) ){
          //std::cout << "GOT AN Iso MATCH" << std::endl;
          if(!trackContainer.cptr()) { continue; }
          d.addPreselTracksIso(trackContainer.cptr());
        }
      }
      
      for(auto &trackContainer: preselTracksCoreFeatures) {
        if( HLT::TrigNavStructure::haveCommonRoI(tauContainer.te(), trackContainer.te()) ){
          //std::cout << "GOT AN Core MATCH" << std::endl;
          if(!trackContainer.cptr()) { continue; }
          d.addPreselTracksCore(trackContainer.cptr());
        }
      }

      if(hasCaloOnlyTaus){
        for (auto &caloOnlyTauContainer: tauCaloOnlyFeatures) {
          if(!caloOnlyTauContainer.cptr()) { continue; }
          if(not HLT::TrigNavStructure::haveCommonRoI(tauContainer.te(), caloOnlyTauContainer.te()) ){
            continue;
          }
          
          for (auto caloOnlyTau: *caloOnlyTauContainer.cptr()) {
            //NOTE: we assume this is of size 1
            xAOD::TauJet *new_caloOnly_tau = new xAOD::TauJet();
            new_caloOnly_tau->makePrivateStore(caloOnlyTau);
            d.setCaloOnyTau(new_caloOnly_tau);
            break;
          }
        }
      }

      std::cout << d << std::endl;
      decoratedTaus.push_back(d);
    }
  }

  //EL_RETURN_CHECK("execute", m_hlt_emulationTool->execute(l1taus, l1jets, l1muons, l1xe, hlt_taus, preselTracksIso, preselTracksCore));
  EL_RETURN_CHECK("execute", m_hlt_emulationTool->execute(l1taus, l1jets, l1muons, l1xe, decoratedTaus));

  //for (auto it: chains_to_test) {
  for(auto &ch: m_hlt_emulationTool->getHltChains()) {
    auto name = ch.first;
    bool emulation_decision = m_hlt_emulationTool->decision(name);

    auto chain_group = m_trigDecisionTool->getChainGroup(trim(name));
    bool cg_passes_event = chain_group->isPassed(trigger_condition);  
    if(cg_passes_event) {
      h_TDT_fires->Fill(name.c_str(), 1);
    }

    if (emulation_decision) {
      h_EMU_fires->Fill(name.c_str(), 1);
    }

    if (emulation_decision != cg_passes_event){
      h_TDT_EMU_diff->Fill(name.c_str(), 1);
      // if(emulation_decision) { 
      // 	++fire_difference_emu[name]; 
      // } else { 
      // 	++fire_difference_TDT[name]; 
      // }

      // BrokenEventInfo eventInfo(entry, ei->eventNumber(), ei->lumiBlock(), name);
      // brokenEvents.push_back( eventInfo );
      MY_MSG_INFO("Chain " << name << " is being tested");
      MY_MSG_INFO(Form("event number %d -- lumi block %d", (int)ei->eventNumber(), (int) ei->lumiBlock()));
      MY_MSG_INFO(Form("TDT AND EMULATION DECISION DIFFERENT. TDT: %d -- EMULATION: %d", (int)cg_passes_event, (int)emulation_decision));
      MY_MSG_INFO("TDT = " << h_TDT_fires->GetBinContent(1) 
          << " / EMU = " << h_EMU_fires->GetBinContent(1) 
          << " / difference = " << h_TDT_EMU_diff->GetBinContent(1));
    }
  }

  clearContainer(presel_taus);
  clearContainer(hlt_taus);
  clearContainer(preselTracksIso);
  clearContainer(preselTracksCore);

  delete hlt_taus;
  delete presel_taus;
  delete preselTracksIso;
  delete preselTracksCore;

  if(hlt_taus_aux) { delete hlt_taus_aux; }
  if(presel_taus_aux) { delete presel_taus_aux; }
  if(preselTracksIsoAux) { delete preselTracksIsoAux; }
  if(preselTracksCoreAux) { delete preselTracksCoreAux; }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTEmulationLoop :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTEmulationLoop :: finalize ()
{
  if( m_trigConfigTool ) {
    m_trigConfigTool = nullptr;
    delete m_trigConfigTool;
  }
  if( m_trigDecisionTool ) {
    m_trigDecisionTool = nullptr;
    delete m_trigDecisionTool;
  }

  if( m_registry ) {
    m_registry = nullptr;
    delete m_registry;
  }

  if( m_l1_emulationTool ) {
    m_l1_emulationTool = nullptr;
    delete m_l1_emulationTool;
  }

  if( m_hlt_emulationTool ) {
    m_hlt_emulationTool = nullptr;
    delete m_hlt_emulationTool;
  }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTEmulationLoop :: histFinalize ()
{
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.
  return EL::StatusCode::SUCCESS;
}

