#pragma once

#include "Module.h"

namespace MoonGlare {
namespace QtShared {

class Issue {
public:
    enum class Type {
        Unknown,
        Error,
        Warning,
        Hint,
        Notice,
    };

    std::string message;
    std::string group;
    Type type;

    std::optional<std::string> fileName;
    std::optional<int> sourceLine;

    std::string internalID;
};

class IssueReporter :public QObject, public iModule {
    Q_OBJECT;
public:
    IssueReporter(SharedModuleManager modmgr);
 
    void DeleteIssue(const std::string &InternalId);
    const std::shared_ptr<Issue> ReportIssue(Issue issue);
    std::vector<std::shared_ptr<Issue>> CurrentIssues() const;
private:
    std::unordered_map<std::string, std::shared_ptr<Issue>> issues;

signals:
    void IssueCreated(Issue);
    void IssueRemoved(Issue);
};

Q_DECLARE_METATYPE(Issue);

} //namespace QtShared 
} //namespace MoonGlare 

