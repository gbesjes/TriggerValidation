#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <TriggerValidation/L1EmulationLoop.h>

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"

#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTrigger/JetRoIContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "xAODTrigger/EnergySumRoI.h"

#include "TrigTauEmulation/ToolsRegistry.h"

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
ClassImp(L1EmulationLoop)



L1EmulationLoop :: L1EmulationLoop () 
{ 
}



EL::StatusCode L1EmulationLoop :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob ()", xAOD::Init());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode L1EmulationLoop :: histInitialize ()
{

  h_TDT_EMU_diff = new TH1F("h_TDT_Emulation_differences", "TDT_Emulation_differences", l1_chains.size(), 0, l1_chains.size());
  h_TDT_fires = new TH1F("h_TDT_fires", "TDT_fires_total_number", l1_chains.size(), 0, l1_chains.size());
  h_EMU_fires = new TH1F("h_EMU_fires", "EMU_fires_total_number", l1_chains.size(), 0, l1_chains.size());

  wk()->addOutput (h_TDT_EMU_diff);
  wk()->addOutput (h_TDT_fires);
  wk()->addOutput (h_EMU_fires);

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode L1EmulationLoop :: fileExecute ()
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode L1EmulationLoop :: changeInput (bool /*firstFile*/)
{
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode L1EmulationLoop :: initialize ()
{


  // Initialize and configure trigger tools
  if (asg::ToolStore::contains<TrigConf::xAODConfigTool>("xAODConfigTool")) {
    std::cout << "Does it happen ?" << std::endl;
    m_trigConfigTool = asg::ToolStore::get<TrigConf::xAODConfigTool>("xAODConfigTool");
  } else {
    m_trigConfigTool = new TrigConf::xAODConfigTool("xAODConfigTool"); // gives us access to the meta-data
  }
  EL_RETURN_CHECK( "initialize", m_trigConfigTool->initialize() );

  if (asg::ToolStore::contains<Trig::TrigDecisionTool>("TrigDecisionTool")) {
    m_trigDecisionTool =  asg::ToolStore::get<Trig::TrigDecisionTool>("TrigDecisionTool");
  } else {
    ToolHandle< TrigConf::ITrigConfigTool > trigConfigHandle( m_trigConfigTool );
    m_trigDecisionTool = new Trig::TrigDecisionTool("TrigDecisionTool");
    EL_RETURN_CHECK( "initialize", m_trigDecisionTool->setProperty( "ConfigTool", trigConfigHandle ) ); // connect the TrigDecisionTool to the ConfigTool
    EL_RETURN_CHECK( "initialize", m_trigDecisionTool->setProperty( "TrigDecisionKey", "xTrigDecision" ) );
  }

  EL_RETURN_CHECK( "initialize", m_trigDecisionTool->initialize() );

  if(asg::ToolStore::contains<ToolsRegistry>("ToolsRegistry")) {
    m_registry = asg::ToolStore::get<ToolsRegistry>("ToolsRegistry");
  } else {
    m_registry = new ToolsRegistry("ToolsRegistry");
    EL_RETURN_CHECK("initialize", m_registry->initialize());
  }

  if (asg::ToolStore::contains<TrigTauEmul::Level1EmulationTool>("TrigTauEmulator")) {
    m_emulationTool = asg::ToolStore::get<TrigTauEmul::Level1EmulationTool>("TrigTauEmulator");
  } else {
    m_emulationTool = new TrigTauEmul::Level1EmulationTool("TrigTauEmulator");
    EL_RETURN_CHECK("initialize", m_emulationTool->setProperty("l1_chains", l1_chains));
    EL_RETURN_CHECK("initialize", m_emulationTool->setProperty("JetTools", m_registry->GetL1JetTools()));
    EL_RETURN_CHECK("initialize", m_emulationTool->setProperty("EmTauTools", m_registry->GetL1TauTools()));
    EL_RETURN_CHECK("initialize", m_emulationTool->setProperty("XeTools", m_registry->GetL1XeTools()));
    EL_RETURN_CHECK("initialize", m_emulationTool->setProperty("MuonTools", m_registry->GetL1MuonTools()));
    EL_RETURN_CHECK("initialize", m_emulationTool->initialize());
  }

  xAOD::TEvent* event = wk()->xaodEvent();

  // ATH_MSG_INFO("Number of events = " << event->getEntries());
  Info("initialize()", "Number of events = %lli", event->getEntries());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode L1EmulationLoop :: execute ()
{

  xAOD::TEvent* event = wk()->xaodEvent();

  const xAOD::EventInfo* ei = 0;
  EL_RETURN_CHECK("execute", event->retrieve(ei, "EventInfo"));  

  const xAOD::EmTauRoIContainer *l1taus = 0;
  EL_RETURN_CHECK("execute", event->retrieve(l1taus, "LVL1EmTauRoIs"));

  const xAOD::JetRoIContainer *l1jets = 0;
  EL_RETURN_CHECK("execute", event->retrieve(l1jets, "LVL1JetRoIs"));
  
  const xAOD::MuonRoIContainer* l1muons = 0;
  EL_RETURN_CHECK("execute", event->retrieve(l1muons, "LVL1MuonRoIs"));
  
  const xAOD::EnergySumRoI* l1xe = 0;
  EL_RETURN_CHECK("execute", event->retrieve(l1xe, "LVL1EnergySumRoI"));

  StatusCode code = m_emulationTool->calculate(l1taus, l1jets, l1muons, l1xe);
  if (code == StatusCode::FAILURE)
    return EL::StatusCode::FAILURE;

  for (auto it: l1_chains) {
    // emulation decision
    bool emul_passes_event = m_emulationTool->decision(it);
    
    // TDT decision
    auto chain_group = m_trigDecisionTool->getChainGroup(it);
    bool cg_passes_event = chain_group->isPassedBits() & TrigDefs::L1_isPassedBeforePrescale;
   
    if(cg_passes_event)
      h_TDT_fires->Fill(it.c_str(), 1);
    
    if (emul_passes_event)
      h_EMU_fires->Fill(it.c_str(), 1);

    if (emul_passes_event != cg_passes_event){
      Warning("execute", "CHAIN %s: event number %d -- lumi block %d", it.c_str(), (int)ei->eventNumber(), (int) ei->lumiBlock());
      Warning("execute", "CHAIN %s: TDT: %d -- EMULATION: %d", it.c_str(), (int)cg_passes_event, (int)emul_passes_event);
      EL_RETURN_CHECK("execute", m_emulationTool->PrintReport(it, l1taus, l1jets, l1muons, l1xe));
      h_TDT_EMU_diff->Fill(it.c_str(), 1);
      // Validator.fill_histograms(ei, l1taus, "TAU12");

      }
    }


    // clear the decorations
    l1taus->clearDecorations();
    l1jets->clearDecorations();
    l1muons->clearDecorations();
    l1xe->clearDecorations();




  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode L1EmulationLoop :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode L1EmulationLoop :: finalize ()
{
  // xAOD::TEvent* event = wk()->xaodEvent();

  // cleaning up trigger tools
  if( m_trigConfigTool ) {
    delete m_trigConfigTool;
    m_trigConfigTool = 0;
  }
  if( m_trigDecisionTool ) {
    delete m_trigDecisionTool;
    m_trigDecisionTool = 0;
  }

  if( m_registry ) {
    delete m_registry;
    m_registry = 0;
  }

  if( m_emulationTool ) {
    delete m_emulationTool;
    m_emulationTool = 0;
  }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode L1EmulationLoop :: histFinalize ()
{
  return EL::StatusCode::SUCCESS;
}
