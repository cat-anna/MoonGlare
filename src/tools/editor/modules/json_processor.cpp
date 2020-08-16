#if 0

#include <boost/algorithm/string.hpp>

#include <ToolBase/Module.h>
#include <iCustomEnum.h>
#include <iFileIconProvider.h>
#include <iFileProcessor.h>
#include <iIssueReporter.h>
#include <icons.h>

#include "../FileSystem.h"

#include <json/json.h>

namespace MoonGlare::Editor::Processor {

struct JSONProcessor : public QtShared::iFileProcessor {

    JSONProcessor(SharedModuleManager modmgr, QtShared::SharedSetEnum jsonEnum, std::string URI)
        : QtShared::iFileProcessor(std::move(URI)), jsonEnum(jsonEnum), moduleManager(modmgr) {}

    ProcessResult ProcessFile() override {
        auto fs = moduleManager->QueryModule<FileSystem>();
        auto reporter = moduleManager->QueryModule<QtShared::IssueReporter>();
        StarVFS::ByteTable bt;
        if (!fs->GetFileData(m_URI, bt)) {
            // todo: log sth
            throw std::runtime_error("Unable to read file: " + m_URI);
        }
        if (bt.byte_size() == 0) {
            // todo: log sth
        }

        std::stringstream ss;
        ss.write((char *)bt.get(), bt.byte_size());

        auto uris = FindAllURI(ss.str());
        for (auto &itm : uris) {
            QtShared::Issue issue;
            issue.fileName = m_URI;
            issue.message = "File " + itm + " does not exists!";
            issue.type = QtShared::Issue::Type::Error;
            issue.group = "json";
            issue.internalID = MakeIssueId("Error", itm);
            if (fs->FileExists(itm))
                reporter->DeleteIssue(issue.internalID);
            else
                reporter->ReportIssue(std::move(issue));
        }

        Json::Value root;
        Json::CharReaderBuilder rbuilder;

        std::string errs;
        bool ok = Json::parseFromStream(rbuilder, ss, &root, &errs);

        if (ok) {
            reporter->DeleteIssue(MakeIssueId());
        } else {
            QtShared::Issue issue;
            issue.fileName = m_URI;
            boost::trim(errs);
            issue.message = errs;
            issue.type = QtShared::Issue::Type::Error;
            issue.group = "json";
            issue.internalID = MakeIssueId();

            std::regex pieces_regex(R"==(\*\s*Line\s*(\d+)\s*,\s*Column\s*(\d+)\s*(.*))==", std::regex::icase);
            std::smatch pieces_match;
            if (std::regex_match(errs, pieces_match, pieces_regex)) {
                issue.sourceLine = std::strtol(pieces_match[1].str().c_str(), nullptr, 10);
                issue.sourceColumn = std::strtol(pieces_match[2].str().c_str(), nullptr, 10);
                issue.message = pieces_match[3];
            }

            reporter->ReportIssue(std::move(issue));
        }

        jsonEnum->Add(m_URI);

        return ProcessResult::Success;
    }

private:
    QtShared::SharedSetEnum jsonEnum;
    SharedModuleManager moduleManager;
};

//----------------------------------------------------------------------------------

struct JSONProcessorModule : public iModule,
                             public QtShared::iFileProcessorInfo,
                             public QtShared::iCustomEnumSupplier,
                             public QtShared::iFileIconInfo {

    JSONProcessorModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    QtShared::SharedSetEnum jsonEnum = std::make_shared<QtShared::SetEnum>("");

    QtShared::SharedFileProcessor CreateFileProcessor(std::string URI) override {
        return std::make_shared<JSONProcessor>(GetModuleManager(), jsonEnum, std::move(URI));
    }

    std::vector<std::string> GetSupportedTypes() {
        return {
            "json",
        };
    }

    std::vector<std::shared_ptr<QtShared::iCustomEnum>> GetCustomEnums() const override { return {/* jsonEnum, */}; }

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{
            FileIconInfo{
                "json",
                ICON_16_CONFIG_FILE,
            },
        };
    }
};

ModuleClassRegister::Register<JSONProcessorModule> JSONProcessorModuleReg("JSONProcessorModule");

//----------------------------------------------------------------------------------
} // namespace MoonGlare::Editor::Processor

#endif