#ifndef TriggerValidation_Utils_H
#define TriggerValidation_Utils_H

#include <EventLoop/StatusCode.h>
#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

#include "xAODBase/IParticle.h"
#include "xAODTrigger/EmTauRoI.h"

#include "AssociationUtils/OverlapRemovalTool.h"

#include <string>
#include <vector>

/// Helper macro for checking xAOD::TReturnCode return values
#define EL_RETURN_CHECK(CONTEXT, EXP)                                    \
    do {                                                                 \
        if (!EXP.isSuccess()) {                                          \
            Error(CONTEXT, XAOD_MESSAGE("Failed to execute: %s"), #EXP); \
            return EL::StatusCode::FAILURE;                              \
        }                                                                \
    } while (false)

// Error checking macro
#define CHECK(ARG)                                                \
    do {                                                          \
        const bool result = ARG;                                  \
        if (!result) {                                            \
            ::Error(APP_NAME, "Failed to execute: \"%s\"", #ARG); \
            return 1;                                             \
        }                                                         \
    } while (false)

// Overlap removal accessor/decorators
ort::inputAccessor_t selectAcc("selected");
ort::inputDecorator_t selectDec("selected");
ort::outputAccessor_t overlapAcc("overlaps");

namespace Utils {

    bool comparePt(const xAOD::IParticle* t1, const xAOD::IParticle* t2) {
        return (t1->pt() > t2->pt() ? true : false);
    }

    bool compareTauClus(const xAOD::EmTauRoI* t1, const xAOD::EmTauRoI* t2) {
        return (t1->tauClus() > t2->tauClus() ? true : false);
    }

    std::vector<std::string> splitNames(const std::string& files, std::string sep = ",") {
        std::vector<std::string> fileList;
        for (size_t i = 0, n; i <= files.length(); i = n + 1) {
            n = files.find_first_of(sep, i);
            if (n == std::string::npos) {
                n = files.length();
            }
            std::string tmp = files.substr(i, n - i);
            std::string ttmp;
            for (unsigned int j = 0; j < tmp.size(); j++) {
                if (tmp[j] == ' ' || tmp[j] == '\n') {
                    continue;
                }
                ttmp += tmp[j];
            }
            fileList.push_back(ttmp);
        }
        return fileList;
    }
}

#endif
