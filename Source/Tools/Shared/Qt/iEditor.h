/*
  * Generated by cppsrc.sh
  * On 2016-09-11 17:09:28,63
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef iEditor_H
#define iEditor_H

#include "Module.h"

namespace MoonGlare {
namespace QtShared {

struct EditorNotFoundException { };

struct iEditor;
using SharedEditor = std::shared_ptr<iEditor>;

struct iEditorInfo {
	struct FileHandleMethodInfo {
		std::string m_Ext;
		std::string m_Icon;
		std::string m_Caption;
		std::string m_MethodID;
	};

	virtual std::vector<FileHandleMethodInfo> GetCreateFileMethods() const { return{}; }
	virtual std::vector<FileHandleMethodInfo> GetOpenFileMethods() const { return{}; }

	//virtual bool IsMultiInstanceAllowed() const { return false; }
protected:
	virtual ~iEditorInfo() {};
};

struct iEditorFactory {
	struct EditorRequestOptions {
	};

	virtual SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method, const EditorRequestOptions&options) const {
		return nullptr;
	}
protected:
	virtual ~iEditorFactory() {};
};

struct EditorProvider :	iModule {
	EditorProvider(SharedModuleManager modmgr);
	bool PostInit() override;

	struct EditorActionInfo {
		SharedModule m_Module;
		std::shared_ptr<iEditorFactory> m_EditorFactory;
		iEditorInfo::FileHandleMethodInfo m_FileHandleMethod;
	};

	//SharedModule GetExtensionHandler(const std::string &ext) const;
	const EditorActionInfo FindOpenEditor(std::string ext) throw (EditorNotFoundException);
	
	const std::vector<EditorActionInfo>& GetCreateMethods() const { return m_CreateMethods; }
public:
	std::vector<EditorActionInfo> m_CreateMethods;
	//std::unordered_map<std::string, SharedModule> m_ExtensionHandlers;
};

struct iEditor {
	virtual bool Create(const std::string &LocationURI,const iEditorInfo::FileHandleMethodInfo& method) { return false; }

	virtual bool OpenData(const std::string &URI) { return false; }
	virtual bool OpenData(const std::string &URI, const iEditorInfo::FileHandleMethodInfo& method) { return OpenData(URI); }

	virtual bool SaveData() { return false; }
	virtual bool TryCloseData() { return false; }
protected:
	virtual ~iEditor() {};
};

} //namespace QtShared
} //namespace MoonGlare

#endif
