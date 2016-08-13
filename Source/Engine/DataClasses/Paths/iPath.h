/*
 * cPath.h
 *
 *  Created on: 18-10-2013
 *      Author: Paweu
 */

#ifndef CPATH_H_
#define CPATH_H_
#if 0

namespace MoonGlare {
namespace DataClasses {
namespace Paths {
struct PathCalcState {
	/** Current Location at Position */
	Physics::vec3 Location = Physics::vec3(0, 0, 0);
	/** Previous result */
	Physics::vec3 PreviousLocation = Physics::vec3(0, 0, 0);
	/** just derative at Location */
	Physics::vec3 Direction = Physics::vec3(0, 0, 0);

	/** Normalized position at the path */
	float Position = 0.0f;
	/** Speed of object */
	float Speed = 0.0f;

	/** Do repeat path? */
	bool Repeated = false;
};

class iPath : public DataClass, public std::enable_shared_from_this<iPath> {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(iPath, DataClass);
public:
	iPath();
	virtual ~iPath();

	/** Get path length */
	virtual float Length() const = 0;
	/** Just calculate position at t */
	virtual Physics::vec3 PositionAt(float t) const = 0;
	/** Attention: function may not work large speeds! */
	virtual bool UpdatePosition(PathCalcState &state) const = 0;

	DefineREADAcces(Flags, unsigned);
	DefineREADAccesPTR(Owner, PathRegister);

	virtual bool LoadMeta(const xml_node Node);

	/** Set path owner. This function may be called only once, next calls do nothing. */
	void SetOwner(PathRegister *Owner);

	struct Flags {
		enum {
			Repetitive		= 0x0001,
		};
	};
	DefineFlagGetter(m_Flags, Flags::Repetitive, Repetitive)
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	PathRegister *m_Owner;
	unsigned m_Flags;

	DefineFlagSetter(m_Flags, Flags::Repetitive, Repetitive)
};

using PathClassRegister = Space::DynamicClassRegister < iPath >;

} // namespace Paths 
} // namespace DataClasses 
} //namespace MoonGlare 

#endif // CPATH_H_ 
#endif