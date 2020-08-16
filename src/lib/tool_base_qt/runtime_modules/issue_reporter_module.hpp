#pragma once

#include <QObject>
#include <boost/functional/hash.hpp>
#include <issue_reporter.hpp>
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class IssueReporterModule : public iIssueReporter, public iModule {
    Q_OBJECT;

public:
    IssueReporterModule(SharedModuleManager modmgr);

    void DeleteIssue(boost::uuids::uuid issue_id) override;
    const std::shared_ptr<Issue> ReportIssue(Issue issue) override;
    std::vector<std::shared_ptr<Issue>> CurrentIssues() const override;

private:
    struct UuidHasher {
        size_t operator()(const boost::uuids::uuid &uid) const {
            return boost::hash<boost::uuids::uuid>()(uid);
        }
    };

    std::unordered_map<boost::uuids::uuid, std::shared_ptr<Issue>, UuidHasher> issues;
};

} // namespace MoonGlare::Tools::RuntineModules
