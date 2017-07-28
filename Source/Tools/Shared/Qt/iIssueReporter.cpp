#include PCH_HEADER
#include "iIssueReporter.h"

namespace MoonGlare {
namespace QtShared {

ModuleClassRgister::Register<IssueReporter> IssueReporterReg("IssueReporter");

IssueReporter::IssueReporter(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {
    qRegisterMetaType<Issue>("Issue");
}

void IssueReporter::DeleteIssue(const std::string &InternalId) {
    auto it = issues.find(InternalId);
    if (it != issues.end()) {
        auto ptr = it->second;
        issues.erase(it);
        IssueRemoved(*ptr);
    }
}

const std::shared_ptr<Issue> IssueReporter::ReportIssue(Issue issue) {
    auto sptr = std::make_shared<Issue>(std::move(issue));

    if (sptr->internalID.empty()) {
        // if no id is provided, then default to issue poiter address
        sptr->internalID = std::to_string(reinterpret_cast<intptr_t>(sptr.get()));
    }

    DeleteIssue(sptr->internalID);
    issues[sptr->internalID] = sptr;
    IssueCreated(*sptr);
    return sptr;
}

std::vector<std::shared_ptr<Issue>> IssueReporter::CurrentIssues() const {
    std::vector<std::shared_ptr<Issue>> r;
    r.reserve(issues.size());
    for (auto &item : issues)
        r.emplace_back(item.second);
    return std::move(r);
}
                      
} //namespace QtShared 
} //namespace MoonGlare 

