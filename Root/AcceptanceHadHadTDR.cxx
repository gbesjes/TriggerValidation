#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <TriggerValidation/AcceptanceHadHadTDR.h>

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"

#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODCore/AuxContainerBase.h"

// Local stuff
#include "TriggerValidation/Utils.h"

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
ClassImp(AcceptanceHadHadTDR)



AcceptanceHadHadTDR :: AcceptanceHadHadTDR ()
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}



EL::StatusCode AcceptanceHadHadTDR :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  job.useXAOD ();
  EL_RETURN_CHECK("setupJob ()", xAOD::Init());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHadTDR :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.

  hists["cutflow"] = new TH1F("cutflow", "cutflow", 10, 0, 10);
  hists["cutflow"]->GetXaxis()->SetBinLabel(1, "init");
  hists["cutflow"]->GetXaxis()->SetBinLabel(2, "taus");
  hists["cutflow"]->GetXaxis()->SetBinLabel(3, "taus_pt");
  hists["cutflow"]->GetXaxis()->SetBinLabel(4, "dr_tau_tau");
  hists["cutflow"]->GetXaxis()->SetBinLabel(5, "jets");
  hists["cutflow"]->GetXaxis()->SetBinLabel(6, "jets_pt");
  hists["cutflow"]->GetXaxis()->SetBinLabel(7, "deta_jets");
  hists["cutflow"]->GetXaxis()->SetBinLabel(8, "l1taus");

  hists["l1_symmetric"] = new TH1F("l1_symmetric", "l1_symmetric", 
				   l1_nsteps, l1_min, l1_min + l1_step * l1_nsteps);
  hists["off_symmetric"] = new TH1F("off_symmetric", "off_symmetric", 
				    off_nsteps, tau1_pt, tau1_pt + off_step * off_nsteps);

  map_l1 =  new TH2F("l1_asymmetric", "l1_asymmetric", 
		     l1_nsteps, l1_min, l1_min + l1_step * l1_nsteps,
		     l1_nsteps, l1_min, l1_min + l1_step * l1_nsteps);

  for (int i = 0; i < l1_nsteps; i++) {
    int thresh = (int) (l1_min / 1000 + l1_step * i / 1000);
    hists["l1_symmetric"]->GetXaxis()->SetBinLabel(i + 1, Form("2TAU%d", thresh));
    map_l1->GetXaxis()->SetBinLabel(i + 1, Form("TAU%d", thresh));
    map_l1->GetYaxis()->SetBinLabel(i + 1, Form("TAU%d", thresh));
  }

  map_off =  new TH2F("off_asymmetric", "off_asymmetric", 
		      off_nsteps, tau1_pt, tau1_pt + off_step * off_nsteps,
		      off_nsteps, tau2_pt, tau2_pt + off_step * off_nsteps);
  
  for (int i = 0; i < off_nsteps; i++) {
    float thresh_1 = tau1_pt / 1000. + off_step * i / 1000.;
    float thresh_2 = tau1_pt / 1000. + off_step * i / 1000.;
    hists["off_symmetric"]->GetXaxis()->SetBinLabel(i + 1, Form("2tau%d", (int)thresh_1));
    map_off->GetXaxis()->SetBinLabel(i + 1, Form("tau%d", (int)thresh_1));
    map_off->GetYaxis()->SetBinLabel(i + 1, Form("tau%d", (int)thresh_2));
  }

  hists["tau1_pt"]      = new TH1F("h_tau1_pt", "tau1_pt", 40, 20, 100);
  hists["tau2_pt"]      = new TH1F("h_tau2_pt", "tau2_pt", 40, 20, 100);
  hists["tau1_eta"]     = new TH1F("h_tau1_eta", "tau1_eta", 10, -2.5, 2.5);
  hists["tau2_eta"]     = new TH1F("h_tau2_eta", "tau2_eta", 10, -2.5, 1.5);
  hists["tau1_phi"]     = new TH1F("h_tau1_phi", "tau1_phi", 10, -3.15, 3.15);
  hists["tau2_phi"]     = new TH1F("h_tau2_phi", "tau2_phi", 10, -3.15, 3.15);
  hists["tau1_ntracks"] = new TH1F("h_tau1_ntracks", "tau1_ntracks", 5, 0, 5);
  hists["tau2_ntracks"] = new TH1F("h_tau2_ntracks", "tau2_ntracks", 5, 0, 5);

  hists["jet1_pt"] = new TH1F("h_jet1_pt", "jet1_pt", 40, 20, 100);
  hists["jet2_pt"] = new TH1F("h_jet2_pt", "jet2_pt", 40, 20, 100);
  hists["jet1_eta"] = new TH1F("h_jet1_eta", "jet1_eta", 90, -4.5, 4.5);
  hists["jet2_eta"] = new TH1F("h_jet2_eta", "jet2_eta", 90, -4.5, 4.5);
  hists["jet1_phi"] = new TH1F("h_jet1_phi", "jet1_phi", 10, -3.15, 3.15);
  hists["jet2_phi"] = new TH1F("h_jet2_phi", "jet2_phi", 10, -3.15, 3.15);

  map_l1taus = new TH2F("map_l1taus", "map_l1taus", 100, 0, 100000, 100, 0, 100000);
  wk()->addOutput (map_l1taus);
  wk()->addOutput (map_l1);
  wk()->addOutput (map_off);
				     
  for (const auto it: hists) 
    wk()->addOutput(it.second);

 
 // for (auto trig: triggers) {
 //    ATH_MSG_INFO(trig);
 //    m_curves_tools_nopt[trig] = new EffCurvesTool(trig + "_nopt");
 //    m_curves_tools_nodr[trig] = new EffCurvesTool(trig + "_nodr");
 //    m_curves_tools_final[trig] = new EffCurvesTool(trig + "_final");
 //  }


  // for (auto it: m_curves_tools_nopt) 
  //   for (auto tool: (it.second)->Efficiencies()) 
  //     wk()->addOutput(tool.second);

  // for (auto it: m_curves_tools_nodr) 
  //   for (auto tool: (it.second)->Efficiencies()) 
  //     wk()->addOutput(tool.second);

  // for (auto it: m_curves_tools_final) {
  //   it.second->record(wk());
  // }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHadTDR :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHadTDR :: changeInput (bool/* firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHadTDR :: initialize ()
{
  // Initialize and configure trigger tools
  if (asg::ToolStore::contains<TrigConf::xAODConfigTool>("xAODConfigTool")) {
    std::cout << "Does it happen ?" << std::endl;
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
    EL_RETURN_CHECK( "initialize", m_trigDecisionTool->setProperty( "ConfigTool", trigConfigHandle ) ); // connect the TrigDecisionTool to the ConfigTool
    EL_RETURN_CHECK( "initialize", m_trigDecisionTool->setProperty( "TrigDecisionKey", "xTrigDecision" ) );
    EL_RETURN_CHECK( "initialize", m_trigDecisionTool->initialize() );
  }
  xAOD::TEvent* event = wk()->xaodEvent();
  ATH_MSG_INFO("Number of events = " << event->getEntries());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHadTDR :: execute ()
{

  xAOD::TEvent* event = wk()->xaodEvent();
  ATH_MSG_DEBUG("execute next event");
  if ((wk()->treeEntry() % 200) == 0)
    ATH_MSG_INFO("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());


  hists["cutflow"]->Fill("init", 1);

  // retrieve the EDM objects
  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", event->retrieve(ei, "EventInfo"));

  const xAOD::EmTauRoIContainer *l1taus = 0;
  EL_RETURN_CHECK("execute", event->retrieve(l1taus, "LVL1EmTauRoIs"));

  const xAOD::TauJetContainer* taus = 0;
  EL_RETURN_CHECK("execute", event->retrieve(taus, "TauJets"));
  
  const xAOD::JetContainer* jets = 0;
  EL_RETURN_CHECK("execute", event->retrieve(jets, "AntiKt4LCTopoJets"));

  xAOD::TauJetContainer* selected_taus = new xAOD::TauJetContainer();
  xAOD::AuxContainerBase* selected_taus_aux = new xAOD::AuxContainerBase();
  selected_taus->setStore(selected_taus_aux);

  select_taus(selected_taus, taus);

  if (selected_taus->size() < 2)
    return EL::StatusCode::SUCCESS;

  hists["cutflow"]->Fill("taus", 1);

  xAOD::TauJet* tau1 = selected_taus->at(0);
  xAOD::TauJet* tau2 = selected_taus->at(1);

  // Leading tau pt cut
  if (tau1->pt() < tau1_pt)
    return EL::StatusCode::SUCCESS;

  // Subleading tau pt cut
  if (tau2->pt() < tau2_pt)
    return EL::StatusCode::SUCCESS;

  hists["cutflow"]->Fill("taus_pt", 1);

  // DR(TAU, TAU) cut
  if (tau1->p4().DeltaR(tau2->p4()) < min_dr_tautau)
    return EL::StatusCode::SUCCESS;

  if (tau1->p4().DeltaR(tau2->p4()) > max_dr_tautau)
    return EL::StatusCode::SUCCESS;

  hists["cutflow"]->Fill("dr_tau_tau", 1);

  // ATH_MSG_INFO("DR(tau1, tau2) = " << tau1->p4().DeltaR(tau2->p4()));
  
  xAOD::JetContainer* selected_jets = new xAOD::JetContainer();
  xAOD::AuxContainerBase* selected_jets_aux = new xAOD::AuxContainerBase();
  selected_jets->setStore(selected_jets_aux);
  select_jets(selected_jets, jets, tau1, tau2);

  // ATH_MSG_INFO("Number of jets = " << selected_jets->size());
  if ((int)selected_jets->size() < n_jets)
    return EL::StatusCode::SUCCESS;

  hists["cutflow"]->Fill("jets", 1);


  xAOD::Jet * jet1 = nullptr;
  xAOD::Jet * jet2 = nullptr;

  if ((int)selected_jets->size() > 0) {
    jet1  = selected_jets->at(0);

    if (jet1->pt() < jet1_pt)
      return EL::StatusCode::SUCCESS;

    if ((int)selected_jets->size() > 1) { 
      jet2  = selected_jets->at(1);

      if (jet2->pt() < jet2_pt)
	return EL::StatusCode::SUCCESS;
      hists["cutflow"]->Fill("jets_pt", 1);

      double delta_eta = fabs(jet1->eta() - jet2->eta());
      if (delta_eta < delta_eta_jj)
	return EL::StatusCode::SUCCESS;
      hists["cutflow"]->Fill("deta_jets", 1);

    } else {
      hists["cutflow"]->Fill("jets_pt", 1);
    }      
      
    // for (auto trig: triggers) {
    //   ATH_MSG_INFO(trig);
    //   bool pass = m_trigDecisionTool->isPassed(trig);
    //   m_curves_tools_final[trig]->fill_hadhad(pass, tau1, tau2, selected_jets->at(0));
    // }

  }

  
  xAOD::EmTauRoIContainer* selected_l1taus = new xAOD::EmTauRoIContainer();
  xAOD::AuxContainerBase* selected_l1taus_aux = new xAOD::AuxContainerBase();
  selected_l1taus->setStore(selected_l1taus_aux);
  
  for (const auto l1tau: *l1taus) {
    if (l1tau->roiType() != xAOD::EmTauRoI::TauRoIWord)
      continue;
    xAOD::EmTauRoI * new_l1tau = new xAOD::EmTauRoI();
    new_l1tau->makePrivateStore(*l1tau);
    selected_l1taus->push_back(new_l1tau);
  }

  selected_l1taus->sort(Utils::compareTauClus);

  if (selected_l1taus->size() < 2)
    return EL::StatusCode::SUCCESS;
  hists["cutflow"]->Fill("l1taus", 1);
  
  xAOD::EmTauRoI* l1tau1 = selected_l1taus->at(0);
  xAOD::EmTauRoI* l1tau2 = selected_l1taus->at(1);

  ATH_MSG_DEBUG("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());

  for (int i = 0; i < l1_nsteps; i++) {
    int thresh = (int) (l1_min / 1000 + l1_step * i / 1000);
    if (l1tau1->tauClus() >= 1000 * thresh and l1tau2->tauClus() >= 1000 * thresh)
      hists["l1_symmetric"]->Fill(Form("2TAU%d", thresh), 1);
    for (int j = i; j < l1_nsteps; j++){
      int sub_thresh = (int) (l1_min / 1000 + l1_step * j / 1000);
      if (l1tau1->tauClus() >= 1000 * thresh and l1tau2->tauClus() >= 1000 * sub_thresh)
    	map_l1->Fill(Form("TAU%d", thresh), Form("TAU%d", sub_thresh), 1);
    }
  }

  for (int i = 0; i < off_nsteps; i++) {
    float thresh_1 = tau1_pt / 1000. + off_step * i / 1000.;
    if (tau1->pt() >= 1000. * thresh_1 and tau2->pt() >= 1000. * thresh_1)
      hists["off_symmetric"]->Fill(Form("2tau%d", (int)thresh_1), 1);
    for (int j = i; j < off_nsteps; j++){
      float thresh_2 = tau2_pt / 1000. + off_step * i / 1000.;
      if (tau1->pt() >= 1000. * thresh_1 and tau2->pt() >= 1000. * thresh_2)
    	map_off->Fill(Form("tau%d", (int)thresh_1), Form("tau%d", (int)thresh_2), 1);
    }
  }


  ATH_MSG_DEBUG("Fill kinematics histograms:");

  hists["tau1_pt"]->Fill(tau1->pt() / 1000.);
  hists["tau2_pt"]->Fill(tau2->pt() / 1000.);
  hists["tau1_eta"]->Fill(tau1->eta());
  hists["tau2_eta"]->Fill(tau2->eta());
  hists["tau1_phi"]->Fill(tau1->phi());
  hists["tau2_phi"]->Fill(tau2->phi());
  hists["tau1_ntracks"]->Fill(tau1->nTracks());
  hists["tau2_ntracks"]->Fill(tau2->nTracks());
  hists["jet1_pt"]->Fill(jet1->pt() / 1000.);
  hists["jet2_pt"]->Fill(jet2->pt() / 1000.);
  hists["jet1_eta"]->Fill(jet1->eta());
  hists["jet2_eta"]->Fill(jet2->eta());
  hists["jet1_phi"]->Fill(jet1->phi());
  hists["jet2_phi"]->Fill(jet2->phi());

  map_l1taus->Fill(l1tau1->tauClus(), l1tau2->tauClus());
  
  // if (l1tau1->tauClus() < 11000)
  //   return EL::StatusCode::SUCCESS;
  // if (l1tau2->tauClus() < 11000)
  //   return EL::StatusCode::SUCCESS;


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHadTDR :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHadTDR :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.
  if (m_trigDecisionTool) 
    delete m_trigDecisionTool;

  if (m_trigConfigTool)
    delete m_trigConfigTool;

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHadTDR :: histFinalize ()
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


EL::StatusCode AcceptanceHadHadTDR :: select_taus(xAOD::TauJetContainer *selected_taus, const xAOD::TauJetContainer * taus)


{

  for (const auto tau: *taus) {

    // pt cut
    if (tau->pt() < 20000.) {
      ATH_MSG_DEBUG("Reject tau " << tau->index() << " with pt = " << tau->pt());
      continue;
    }

    // eta cut
    if (fabs(tau->eta()) > 2.5) {
      ATH_MSG_DEBUG("Reject tau " << tau->index() << " with eta = " << tau->eta());
      continue;
    }

    if (fabs(tau->eta()) > 1.37 and fabs(tau->eta()) < 1.52) {
      ATH_MSG_DEBUG("Reject tau " << tau->index() << " with eta = " << tau->eta());
      continue;
    }
      
    // 1 or 3 tracks
    if (tau->nTracks() != 1 and tau->nTracks() != 3) {
      ATH_MSG_DEBUG("Reject tau " << tau->index() << " with nTracks = " << tau->nTracks());
      continue;
    }
    // ID cut
    if (not tau->isTau(xAOD::TauJetParameters::JetBDTSigMedium)) {
      ATH_MSG_DEBUG("Reject tau " << tau->index() << " with medium ID = " << tau->isTau(xAOD::TauJetParameters::JetBDTSigMedium));
      continue;
    }      
      // selectDec(*tau) = true;
    xAOD::TauJet * new_tau = new xAOD::TauJet();
    new_tau->makePrivateStore(*tau);
    selected_taus->push_back(new_tau);
  }

  // sort by pt
  selected_taus->sort(Utils::comparePt);

  return EL::StatusCode::SUCCESS;

}

EL::StatusCode AcceptanceHadHadTDR :: select_jets(xAOD::JetContainer *selected_jets, 
						  const xAOD::JetContainer *jets, 
						  const xAOD::TauJet *tau1, 
						  const xAOD::TauJet *tau2)
{
  for (const auto jet: *jets) {
      
    // pt cut
    if (jet->pt() < 30000.) 
      continue;
    
    // eta cut
    if (fabs(jet->eta()) > jet_eta)
      continue;

    // ORL with first tau
    if (jet->p4().DeltaR(tau1->p4()) < 0.4)
      continue;

    // ORL with second tau
    if (jet->p4().DeltaR(tau2->p4()) < 0.4)
      continue;

    xAOD::Jet* new_jet = new xAOD::Jet();
    new_jet->makePrivateStore(*jet);
    selected_jets->push_back(new_jet);
  }
  // sort them by pT
  selected_jets->sort(Utils::comparePt);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AcceptanceHadHadTDR :: select_l1taus(xAOD::EmTauRoIContainer *selected_l1taus, const xAOD::EmTauRoIContainer *l1taus)
{
  for (const auto l1tau: *l1taus) {

    if (l1tau->roiType() != xAOD::EmTauRoI::TauRoIWord)
      continue;

    xAOD::EmTauRoI * new_l1tau = new xAOD::EmTauRoI();
    new_l1tau->makePrivateStore(*l1tau);
    selected_l1taus->push_back(new_l1tau);
  }

  // sort by tauClus
  selected_l1taus->sort(Utils::compareTauClus);

  return EL::StatusCode::SUCCESS;
}
