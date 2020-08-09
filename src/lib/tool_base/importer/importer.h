#pragma once

#include <filesystem>
#include <map>
#include <runtime_modules.h>
#include <set>
#include <string>
#include <variant>

namespace MoonGlare::Tools::Importer {

struct ImportResult {
    enum class Result {
        Failure,
        Success,
    };

    struct TempFile {
        std::string outputName;
        std::string systemPath;
        ~TempFile() { std::filesystem::remove(systemPath); }
    };

    struct MemoryFile {
        std::string outputName;
        std::string fileData;
    };

    struct ExistingFile {
        std::string outputName;
        std::string systemPath;
    };

    using ImportedFile = std::variant<std::nullptr_t, TempFile, MemoryFile, ExistingFile>;

    Result result = Result::Failure;
    std::vector<ImportedFile> files;
};

class iImporter {
public:
    struct ImporterCreateData {
        std::string fileUri;
    };

    iImporter(SharedModuleManager smm, ImporterCreateData createData)
        : moduleManager(std::move(smm)), createData(std::move(createData)) {}

    virtual ~iImporter() {}
    bool Finished() const { return finished; }
    std::shared_ptr<ImportResult> Result() const;

    void Execute();

protected:
    bool finished = false;

    SharedModuleManager GetModuleManager() const { return moduleManager; }
    const ImporterCreateData &GetCreateData() const { return createData; }

    void EmitOutputFile(ImportResult::ImportedFile file);
    void EmitOutputFileData(std::string name, std::string data);

    virtual void StartImport(std::shared_ptr<ImportResult> result) = 0;

private:
    SharedModuleManager moduleManager;
    std::shared_ptr<ImportResult> result;
    ImporterCreateData createData;
};

class ImporterFactory {
public:
    virtual std::shared_ptr<iImporter> CreateImporter(SharedModuleManager smm,
                                                      iImporter::ImporterCreateData createData = {}) = 0;
};

class ImporterProvider : public iModule {
public:
    static void RegisterModule();

    ImporterProvider(SharedModuleManager modmgr);
    bool PostInit() override;

    struct ImporterInfo {
        std::shared_ptr<ImporterFactory> factory;
        std::string icon;
        std::set<std::string> fileExtensions;
    };

    virtual const std::shared_ptr<ImporterInfo> GetImporter(const std::string &ext) const;

private:
    std::unordered_map<std::string, std::shared_ptr<ImporterInfo>> importerMap;

    void AddImporter(ImporterInfo ii);
};

} // namespace MoonGlare::Tools::Importer
