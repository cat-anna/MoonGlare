/*
  * Generated by cppsrc.sh
  * On 2016-01-09  9:20:56,72
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef RDCMetaModule_H
#define RDCMetaModule_H

#include <memory>
#include <core/nStarVFS.h>

namespace StarVFS {
namespace Containers {
class RDCContainer;
}
}


class RDCMetaModule : public StarVFS::Modules::iModule {
public:
	RDCMetaModule(StarVFS::StarVFS *svfs, StarVFS::Containers::RDCContainer *RDC);
	virtual ~RDCMetaModule();

	virtual bool Enable() override;
	virtual bool Disable() override;
protected:
	//TODO: add stuff there
private:
	struct Impl;
	std::unique_ptr<Impl> m_Impl;
};

#endif
