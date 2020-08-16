#pragma once

#include <QMetaType>
#include <QObject>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <optional>
#include <runtime_modules.h>
#include <string>
#include <vector>

namespace MoonGlare::Tools {

class Issue {
public:
    Issue(const std::string &reporter_id) : reporter_id(std::move(reporter_id)) {}
    Issue() : reporter_id(boost::uuids::to_string(id)) {}

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

    std::optional<std::string> file_name;
    std::optional<int> source_line;
    std::optional<int> source_column;

    const boost::uuids::uuid id = boost::uuids::random_generator()();
    const std::string reporter_id;
};

class iIssueReporter : public QObject {
    Q_OBJECT
public:
    virtual void DeleteIssue(boost::uuids::uuid issue_id) = 0;
    virtual const std::shared_ptr<Issue> ReportIssue(Issue issue) = 0;
    virtual std::vector<std::shared_ptr<Issue>> CurrentIssues() const = 0;

signals:
    void IssueCreated(Issue);
    void IssueRemoved(Issue);

protected:
    virtual ~iIssueReporter() = default;
};

class IssueProducer {
public:
    IssueProducer(SharedModuleManager manager) {
        issue_reporter = manager->QueryModule<iIssueReporter>();
    };
    ~IssueProducer() { ClearAllIssues(); }

    std::shared_ptr<Issue> ReportIssue(Issue issue) {
        auto reporter = issue_reporter.lock();
        if (!reporter) {
            return nullptr;
        }
        reported_issues[issue.reporter_id] = issue.id;
        return reporter->ReportIssue(std::move(issue));
    }

    void ClearIssue(const std::string &reporter_id) {
        auto it = reported_issues.find(reporter_id);
        if (it != reported_issues.end()) {
            reported_issues.erase(it);
        }
    }

    void ClearIssue(boost::uuids::uuid id) {
        for (auto it = reported_issues.begin(); it != reported_issues.end(); ++it) {
            if (it->second == id) {
                reported_issues.erase(it);
                return;
            }
        }
    }

    void ClearAllIssues() {
        auto reporter = issue_reporter.lock();
        if (!reporter) {
            return;
        }

        for (auto &item : reported_issues) {
            reporter->DeleteIssue(item.second);
        }

        reported_issues.clear();
    }

private:
    std::weak_ptr<iIssueReporter> issue_reporter;
    std::unordered_map<std::string, boost::uuids::uuid> reported_issues;
};

Q_DECLARE_METATYPE(Issue);

} // namespace MoonGlare::Tools
