#pragma once

namespace StarVFS {
namespace Exporters {

class FSExporter : public iExporter {
public:
    FSExporter(StarVFS *svfs);
 	virtual ~FSExporter();

	virtual std::unique_ptr<AttributeMapInstance> GetAttributeMapInstance() const {
		auto atm = CreateAttributeMapInstance<FSExporter>();
		//atm->AddAttrib("CompressionMode", &FSExporter::GetCompressionMode, &FSExporter::SetCompressionMode);
		//atm->AddAttrib("CompressionLevel", &FSExporter::GetCompressionLevel, &FSExporter::SetCompressionLevel);

		return std::unique_ptr<AttributeMapInstance>(atm.release());
	}

private: 
	virtual ExportResult WriteLocalFile(const String &LocalFileName) override;
};

} //namespace Exporters 
} //namespace StarVFS 
