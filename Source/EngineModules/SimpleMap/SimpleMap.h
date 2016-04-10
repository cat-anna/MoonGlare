#ifndef CSIMPLEMAPH
#define CSIMPLEMAPH

namespace MoonGlare {
namespace Modules {
namespace SimpleMap {

using namespace DataClasses::Maps;

DECLARE_SCRIPT_EVENT_VECTOR(SimpleMapScriptEvents, MapScriptEvents,
		SCRIPT_EVENT_ADD((OnModelCreate)),
		SCRIPT_EVENT_REMOVE()
	);

class SimpleMap : public iMap {
	GABI_DECLARE_STATIC_CLASS(SimpleMap, iMap)
	DECLARE_SCRIPT_HANDLERS(SimpleMapScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	DISABLE_COPY();
public:
	SimpleMap(const string& Name);
	virtual ~SimpleMap();

	virtual bool LoadMeta();
	virtual bool LoadMapObjects(::Core::Objects::ObjectRegister& where); 
	virtual std::unique_ptr<Object> LoadMapObject();

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	::DataClasses::ModelPtr m_MapModel;
	std::unique_ptr<::Core::Objects::Object> m_MapObject;

	bool DoInitialize() override;
	bool DoFinalize() override;

	virtual int InvokeOnInitialize() override;
	virtual int InvokeOnFinalize() override;
	virtual int InvokeAfterLoad() override;

	virtual std::unique_ptr<Object> LoadStaticModel();

	vec3 m_UnitSize;
};

#define xmlSimpleMap_StaticSpritesList		"Sprites"
#define xmlSimpleMap_Sprite					"Sprite"
#define xmlSimpleMap_MaterialList			"Materials"
#define xmlSimpleMap_Material				"Material"

} // namespace SimpleMap
} // namespace Modules
} // namespace MoonGlare

#endif // CFPSMAP_H
