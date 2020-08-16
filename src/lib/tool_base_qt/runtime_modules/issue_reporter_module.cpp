#include "issue_reporter_module.hpp"

namespace MoonGlare::Tools::RuntineModules {

IssueReporterModule::IssueReporterModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {
    qRegisterMetaType<Issue>("Issue");
}

void IssueReporterModule::DeleteIssue(boost::uuids::uuid issue_id) {
    auto it = issues.find(issue_id);
    if (it != issues.end()) {
        auto ptr = it->second;
        issues.erase(it);
        IssueRemoved(*ptr);
    }
}

const std::shared_ptr<Issue> IssueReporterModule::ReportIssue(Issue issue) {
    auto issue_ptr = std::make_shared<Issue>(std::move(issue));

    DeleteIssue(issue_ptr->id);
    issues[issue_ptr->id] = issue_ptr;
    IssueCreated(*issue_ptr);
    return issue_ptr;
}

std::vector<std::shared_ptr<Issue>> IssueReporterModule::CurrentIssues() const {
    std::vector<std::shared_ptr<Issue>> r;
    r.reserve(issues.size());
    for (auto &item : issues)
        r.emplace_back(item.second);
    return std::move(r);
}

} // namespace MoonGlare::Tools::RuntineModules
