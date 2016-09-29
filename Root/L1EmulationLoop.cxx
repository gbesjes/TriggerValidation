#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <TriggerValidation/L1EmulationLoop.h>

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODTrigger/EnergySumRoI.h"
#include "xAODTrigger/JetRoIContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"

#include "TrigTauEmulation/MsgStream.h"
#include "TrigTauEmulation/ToolsRegistry.h"

/// Helper macro for checking xAOD::TReturnCode return values
#define EL_RETURN_CHECK(CONTEXT, EXP)                                    \
    do {                                                                 \
        if (!EXP.isSuccess()) {                                          \
            Error(CONTEXT, XAOD_MESSAGE("Failed to execute: %s"), #EXP); \
            return EL::StatusCode::FAILURE;                              \
        }                                                                \
    } while (false)

// this is needed to distribute the algorithm to the workers
ClassImp(L1EmulationLoop)

    L1EmulationLoop::L1EmulationLoop() {}

EL::StatusCode L1EmulationLoop::setupJob(EL::Job& job) {
    job.useXAOD();
    EL_RETURN_CHECK("setupJob ()", xAOD::Init());

    return EL::StatusCode::SUCCESS;
}

EL::StatusCode L1EmulationLoop::histInitialize() {
    h_TDT_EMU_diff = new TH1F("h_TDT_Emulation_differences", "TDT_Emulation_differences", l1_chains.size(), 0, l1_chains.size());
    h_TDT_fires = new TH1F("h_TDT_fires", "TDT_fires_total_number", l1_chains.size(), 0, l1_chains.size());
    h_EMU_fires = new TH1F("h_EMU_fires", "EMU_fires_total_number", l1_chains.size(), 0, l1_chains.size());

    for (unsigned int ich = 0; ich < l1_chains.size(); ich++) {
        auto chain = l1_chains[ich];
        h_TDT_EMU_diff->GetXaxis()->SetBinLabel(ich + 1, chain.c_str());
        h_TDT_fires->GetXaxis()->SetBinLabel(ich + 1, chain.c_str());
        h_EMU_fires->GetXaxis()->SetBinLabel(ich + 1, chain.c_str());
    }

    wk()->addOutput(h_TDT_EMU_diff);
    wk()->addOutput(h_TDT_fires);
    wk()->addOutput(h_EMU_fires);

    return EL::StatusCode::SUCCESS;
}

EL::StatusCode L1EmulationLoop::fileExecute() {
    return EL::StatusCode::SUCCESS;
}

EL::StatusCode L1EmulationLoop::changeInput(bool /*firstFile*/) {
    return EL::StatusCode::SUCCESS;
}

EL::StatusCode L1EmulationLoop::initialize() {
    // Initialize and configure trigger tools
    if (asg::ToolStore::contains<TrigConf::xAODConfigTool>("xAODConfigTool")) {
        std::cout << "Does it happen ?" << std::endl;
        m_trigConfigTool = asg::ToolStore::get<TrigConf::xAODConfigTool>("xAODConfigTool");
    } else {
        m_trigConfigTool = new TrigConf::xAODConfigTool("xAODConfigTool");  // gives us access to the meta-data
        EL_RETURN_CHECK("initialize", m_trigConfigTool->initialize());
    }

    if (asg::ToolStore::contains<Trig::TrigDecisionTool>("TrigDecTool")) {
        m_trigDecisionTool = asg::ToolStore::get<Trig::TrigDecisionTool>("TrigDecTool");
    } else {
        ToolHandle<TrigConf::ITrigConfigTool> trigConfigHandle(m_trigConfigTool);
        m_trigDecisionTool = new Trig::TrigDecisionTool("TrigDecTool");
        EL_RETURN_CHECK("initialize", m_trigDecisionTool->setProperty(
                                          "ConfigTool", trigConfigHandle));  // connect the TrigDecisionTool to the ConfigTool
        EL_RETURN_CHECK("initialize", m_trigDecisionTool->setProperty("TrigDecisionKey", "xTrigDecision"));
        EL_RETURN_CHECK("initialize", m_trigDecisionTool->initialize());
    }

    if (asg::ToolStore::contains<ToolsRegistry>("ToolsRegistry")) {
        m_registry = asg::ToolStore::get<ToolsRegistry>("ToolsRegistry");
    } else {
        m_registry = new ToolsRegistry("ToolsRegistry");
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
        m_l1_emulationTool->msg().setLevel(this->msg().level());
    }

    xAOD::TEvent* event = wk()->xaodEvent();

    MY_MSG_INFO("Number of events = " << event->getEntries());

    return EL::StatusCode::SUCCESS;
}

EL::StatusCode L1EmulationLoop::execute() {
    xAOD::TEvent* event = wk()->xaodEvent();
    MY_MSG_VERBOSE("--------------------------");
    MY_MSG_VERBOSE("Read event number " << wk()->treeEntry() << " / " << event->getEntries());
    MY_MSG_VERBOSE("--------------------------");

    const xAOD::EventInfo* ei = 0;
    EL_RETURN_CHECK("execute", event->retrieve(ei, "EventInfo"));

    const xAOD::EmTauRoIContainer* l1taus = 0;
    EL_RETURN_CHECK("execute", event->retrieve(l1taus, "LVL1EmTauRoIs"));

    const xAOD::JetRoIContainer* l1jets = 0;
    EL_RETURN_CHECK("execute", event->retrieve(l1jets, "LVL1JetRoIs"));

    const xAOD::MuonRoIContainer* l1muons = 0;
    EL_RETURN_CHECK("execute", event->retrieve(l1muons, "LVL1MuonRoIs"));

    const xAOD::EnergySumRoI* l1xe = 0;
    EL_RETURN_CHECK("execute", event->retrieve(l1xe, "LVL1EnergySumRoI"));

    StatusCode code = m_l1_emulationTool->calculate(l1taus, l1jets, l1muons, l1xe);
    if (code == StatusCode::FAILURE) return EL::StatusCode::FAILURE;

    bool at_least_one_diff = false;
    std::vector<std::string> decision_lines;
    for (auto it : l1_chains) {
        // emulation decision
        bool emul_passes_event = m_l1_emulationTool->decision(it);

        // TDT decision
        auto chain_group = m_trigDecisionTool->getChainGroup(it);
        bool cg_passes_event = chain_group->isPassedBits() & TrigDefs::L1_isPassedBeforePrescale;
        bool cg_passes_event_1 = chain_group->isPassedBits() & TrigDefs::L1_isPassedAfterVeto;

        if (cg_passes_event or cg_passes_event_1) {
            h_TDT_fires->Fill(it.c_str(), 1);
        }

        if (emul_passes_event) {
            h_EMU_fires->Fill(it.c_str(), 1);
        }

        if (emul_passes_event != cg_passes_event) {
            at_least_one_diff = true;
            h_TDT_EMU_diff->Fill(it.c_str(), 1);
            std::ostringstream decision_line;
            decision_line << "\t |" << std::setw(43) << it;
            decision_line << " |  " << std::setw(5) << cg_passes_event;
            decision_line << "|   " << std::setw(7) << emul_passes_event;
            decision_line << "|";
            decision_lines.push_back(decision_line.str());
        }
    }
    // print-outs
    if (at_least_one_diff) {
        Warning("execute", "event number %d -- lumi block %d", (int)ei->eventNumber(), (int)ei->lumiBlock());
        EL_RETURN_CHECK("execute", m_l1_emulationTool->PrintReport(l1taus, l1jets, l1muons, l1xe));
        // EL_RETURN_CHECK("execute", m_l1_emulationTool->PrintCounters());
        MY_MSG_INFO("\t -- Chains with differences --");
        MY_MSG_INFO("\t +--------------------------------------------+-------+-----------+");
        MY_MSG_INFO("\t |                                      Chain |  TDT  | EMULATION |");
        for (auto line : decision_lines) MY_MSG_INFO(line);
        MY_MSG_INFO("\t +--------------------------------------------+-------+-----------+");
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

EL::StatusCode L1EmulationLoop::postExecute() {
    // Here you do everything that needs to be done after the main event
    // processing.  This is typically very rare, particularly in user
    // code.  It is mainly used in implementing the NTupleSvc.
    return EL::StatusCode::SUCCESS;
}

EL::StatusCode L1EmulationLoop::finalize() {
    if (m_trigConfigTool) {
        m_trigConfigTool = nullptr;
        delete m_trigConfigTool;
    }
    if (m_trigDecisionTool) {
        m_trigDecisionTool = nullptr;
        delete m_trigDecisionTool;
    }

    if (m_registry) {
        m_registry = nullptr;
        delete m_registry;
    }

    if (m_l1_emulationTool) {
        m_l1_emulationTool = nullptr;
        delete m_l1_emulationTool;
    }

    return EL::StatusCode::SUCCESS;
}

EL::StatusCode L1EmulationLoop::histFinalize() {
    return EL::StatusCode::SUCCESS;
}
