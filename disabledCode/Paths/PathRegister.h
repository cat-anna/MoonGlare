/*
 * cPathsManager.h
 *
 *  Created on: 18-10-2013
 *      Author: Paweu
 */

#ifndef PATHREGISTER_H_
#define PATHREGISTER_H_
#if 0

namespace MoonGlare {
namespace DataClasses {
namespace Paths {
class PathRegister : public cRootClass {
	SPACERTTI_DECLARE_STATIC_CLASS(PathRegister, cRootClass);
public:
	PathRegister(iMap *Owner);
	virtual ~PathRegister();

	DefineREADAccesPTR(Owner, iMap);

	bool Initialize();
	bool Finalize();

	iPathSharedPtr GetPath(const string& Name) const;
private: 
	struct PathEntry {
		NameClassPair Info;
		mutable iPathWeakPtr Ptr;
		xml_node MetaNode;
	};
	using PathMap = std::unordered_map<string, PathEntry>;
	PathMap m_PathMap;
	FileSystem::XMLFile m_PathMetaDocument;
};
} // namespace Paths 
} // namespace DataClasses 
} //namespace MoonGlare 

#endif  PATHREGISTER_H_ 
#endif
