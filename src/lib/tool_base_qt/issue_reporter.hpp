#pragma once

#include <QMetaType>
#include <QObject>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace MoonGlare::Tools {

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

    std::optional<std::string> file_name;
    std::optional<int> source_line;
    std::optional<int> source_column;

    boost::uuids::uuid const id = boost::uuids::random_generator()();
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

Q_DECLARE_METATYPE(Issue);

} // namespace MoonGlare::Tools
