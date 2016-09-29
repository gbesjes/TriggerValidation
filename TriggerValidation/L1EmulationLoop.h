#ifndef TriggerValidation_L1EmulationLoop_H
#define TriggerValidation_L1EmulationLoop_H

#include <EventLoop/Algorithm.h>
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigTauEmulation/ChainRegistry.h"
#include "TrigTauEmulation/Level1EmulationTool.h"
#include "TrigTauEmulation/ToolsRegistry.h"

#include "TH1F.h"

class L1EmulationLoop : public EL::Algorithm {
    // put your configuration variables here as public variables.
    // that way they can be set directly from CINT and python.
  public:
    // float cutValue;
    std::vector<std::string> l1_chains;

    Trig::TrigDecisionTool* m_trigDecisionTool;  //!
    TrigConf::xAODConfigTool* m_trigConfigTool;  //!

    TrigTauEmul::Level1EmulationTool* m_l1_emulationTool;  //!
    ToolsRegistry* m_registry;                             //!
    ChainRegistry* m_ch_registry;                          //!

    // variables that don't get filled at submission time should be
    // protected from being send from the submission node to the worker
    // node (done by the //!)
  public:
    TH1F* h_TDT_EMU_diff;  //!
    TH1F* h_TDT_fires;     //!
    TH1F* h_EMU_fires;     //!

    // Tree *myTree; //!
    // TH1 *myHist; //!

    // this is a standard constructor
    L1EmulationLoop();

    // these are the functions inherited from Algorithm
    virtual EL::StatusCode setupJob(EL::Job& job);
    virtual EL::StatusCode fileExecute();
    virtual EL::StatusCode histInitialize();
    virtual EL::StatusCode changeInput(bool firstFile);
    virtual EL::StatusCode initialize();
    virtual EL::StatusCode execute();
    virtual EL::StatusCode postExecute();
    virtual EL::StatusCode finalize();
    virtual EL::StatusCode histFinalize();

    // this is needed to distribute the algorithm to the workers
    ClassDef(L1EmulationLoop, 1);
};

#endif
