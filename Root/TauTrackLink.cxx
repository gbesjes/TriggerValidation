#include <EventLoop/Job.h>

#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <TriggerValidation/TauTrackLink.h>

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"

#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

#include <xAODEventInfo/EventInfo.h>
#include <xAODTau/TauJetContainer.h>

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
ClassImp(TauTrackLink)



TauTrackLink :: TauTrackLink ()
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}



EL::StatusCode TauTrackLink :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob ()", xAOD::Init());
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauTrackLink :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauTrackLink :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauTrackLink :: changeInput (bool firstFile)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauTrackLink :: initialize ()
{
  xAOD::TEvent* event = wk()->xaodEvent();
  Info("initialize()", "Number of events = %lli", event->getEntries());
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauTrackLink :: execute ()
{
  xAOD::TEvent* event = wk()->xaodEvent();
  ATH_MSG_INFO("--------------------------") ;
  ATH_MSG_INFO("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());
  ATH_MSG_INFO("--------------------------") ;
  
  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", event->retrieve(ei, "EventInfo"));

  const xAOD::TauJetContainer * taus = 0;
  EL_RETURN_CHECK("execute", event->retrieve(taus, "TauJets"));

  const xAOD::TauJetContainer * hlt_taus = 0;
  EL_RETURN_CHECK("execute", event->retrieve(hlt_taus, "HLT_xAOD__TauJetContainer_TrigTauRecMerged"));


  ATH_MSG_INFO("Number of offline taus = "<< taus->size());
  for (const auto* tau: *taus) {
    ATH_MSG_INFO("Offline tau (index/pt/eta/ntracks) = " <<
  		 tau->index() << " / " <<
  		 tau->pt() / 1000. << " / " <<
  		 tau->eta() << " / " <<
  		 tau->nTracks());
    for (unsigned int i = 0; i < tau->nTracks(); i++) {
      ATH_MSG_INFO("\t track (index/pt/eta) = " <<
  		   tau->track(i)->index() << " / " <<
  		   tau->track(i)->pt() / 1000. << " / " <<
  		   tau->track(i)->eta());
    }
  }

  // ATH_MSG_INFO("Number of hlt taus = "<< hlt_taus->size());
  // for (const auto* tau: *hlt_taus) {
  //   ATH_MSG_INFO("HLT tau (index/pt/eta/ntracks) = " <<
  // 		 tau->index() << " / " <<
  // 		 tau->pt() / 1000. << " / " <<
  // 		 tau->eta() << " / " <<
  // 		 tau->nTracks());
  //   for (unsigned int i = 0; i < tau->nTracks(); i++) {
  //     ATH_MSG_INFO("\t track (index/pt/eta) = " <<
  // 		   tau->track(i)->index() << " / " <<
  // 		   tau->track(i)->pt() / 1000. << " / " <<
  // 		   tau->track(i)->eta());
  //   }
  // }

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauTrackLink :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauTrackLink :: finalize ()
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
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode TauTrackLink :: histFinalize ()
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
