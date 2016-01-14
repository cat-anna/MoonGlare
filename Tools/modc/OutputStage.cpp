/*
  * Generated by cppsrc.sh
  * On 2015-03-11 22:25:08,13
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "Pipeline.h"
#include <libs/InternalFileSystem/FolderContainer.h>
#include <libs/InternalFileSystem/MoonGlareContainer/nMoonGlareContainer.h>
#include "InternalContainerExporter.h"

namespace modc {
namespace pipeline {

SettingsMap OutputStage::OutputConfig;

struct OutputStageImpl : public OutputStage {
	void StageValidation(WorkConfiguration &data) {
		auto fs = shared<OutputStageImpl>();

		if (!data.output.container->IsWritable()) {
			cout << data.StageCounter() << "Output container is not writable!\n";
			Error::Error();
		}

		data->pipeline->QueueAction([fs](WorkConfiguration &data) { fs->ProcessFiles(data); });
	}

	void OpenOutput(WorkConfiguration &data) {
		if (Verbosity::PrintSubStep())
			cout << data.StageCounter() << "Creating output container\n";

		switch (data.output.Type) {
		case ModuleFileType::CompiledModule:{
			FileSystem::MoonGlareContainer::WritterConfiguration config;
			//config.BufferingEnabled = false;
			data.output.container = std::make_shared<FileSystem::MoonGlareContainer::Writter>(data.output.FileName, &config);
			break;
		}
		case ModuleFileType::RawModule:
			data.output.container = std::make_shared<FileSystem::FolderContainer>(data.output.FileName);
			break;
		case ModuleFileType::InternalModule: {
			InternalContainerSettings s;
			s.Parse(OutputConfig);
			data.output.container = std::make_shared<InternalContainer>(data.output.FileName, s);
			break;
		}

		case ModuleFileType::None:
		case ModuleFileType::Unknown:
		default:
			Error::Error();
			cout << data.StageCounter() << "Error: No output specified\n";
			return;
		}

		if (!data.output.container->IsReady()) {
			throw "Unable to open destination!";
		}

		if (!data.output.container->IsWritable()) {
			throw "Destination is not writtable!";
		}
		data.output.RW = RWMode::W;
		auto fs = shared<OutputStageImpl>();
		data->pipeline->QueueAction([fs](WorkConfiguration &data) { fs->StageValidation(data); });
	}

	void ProcessFiles(WorkConfiguration &data) {
		auto fs = shared<OutputStageImpl>();
		for (auto &it : data.FileList) {
			if (it.Ignore) continue;
			data->pipeline->QueueAction([fs, &it](WorkConfiguration &data) { fs->ProcessFile(data, it); });
		}
		data->pipeline->QueueLastAction([fs](WorkConfiguration &data) { fs->PostDataProcess(data); });
	}

	void PostDataProcess(WorkConfiguration &data) {
		if (Verbosity::PrintInfo())
			std::cout << data.StageCounter() << "Finalizing output container\n";
		data.output.container.reset();
	}

	void ProcessFile(WorkConfiguration &data, FileInfo& fi) {
		if (Verbosity::PrintSubStep())
			cout << data.StageCounter() << "Writting file '" << fi.Name << "'\n";

		string fullpath = fi.VirtualPath + "/" + fi.Name;
		auto writter = data.output.container->GetFileWritter(fullpath);

		if (!writter) {
			cout << data.StageCounter() << "Unable to open file '" << fullpath << "' for writting!\n";
			Error::Error();
			return;
		}

		if (fi.Generated) {
			writter->SetFileData(fi.PreloadedData.get(), fi.PreloadedSize);
		} else {
			if (!fi.Preloaded)
				fi.PreloadData();
			writter->SetFileData(fi.PreloadedData.get(), fi.PreloadedSize);
			//writter->OwnData(fi.PreloadData.get(), fi.PreloadedSize);
			//fi.ClearPreloaded();
		}
	}
};

//---------------------------------------------------------------------------------------

void OutputStage::Queue(Pipeline &p) {
	auto fs = Init<OutputStageImpl>(p);
	p.QueueFirstAction([fs](WorkConfiguration &data){ fs->OpenOutput(data); });
}

} //namespace pipeline 
} //namespace modc 
