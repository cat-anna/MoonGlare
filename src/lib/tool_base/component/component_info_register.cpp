#define WANTS_TYPE_INFO
#define WANTS_SERIALIZATION

#include "component_info_register.hpp"
#include "attribute_map_builder.hpp"
#include "component/camera.hpp"
#include "component/light_source.hpp"
#include "component/mesh.hpp"
#include "component/name.hpp"
#include "component/rect/rect_image.hpp"
#include "component/rect/rect_transform.hpp"
#include "component/transform.hpp"
#include "eigen_math_type_info.hpp"


// #include <component/revision.hpp>
#include <component_register.hpp>
#include <editable_type_provider.hpp>

namespace MoonGlare::Tools::Component {

void RegisterAllBaseComponents(SharedModuleManager manager) {
    auto provider = manager->QueryModule<iEditableTypeProvider>();
    auto reg = manager->QueryModule<iComponentRegister>();

    {
        using namespace MoonGlare::math;
        provider->RegisterTypes<fvec2, fvec3, fvec4>("Math");
        provider->RegisterTypes<ivec2, ivec3, ivec4>("Math");
    }

    {
        using namespace MoonGlare::Component;
        provider->RegisterTypes<Name>("component.base");
        reg->RegisterComponents<Name>("component.base");

        provider->RegisterTypes<Transform, Mesh, LightSource, Camera>("component.3d");
        reg->RegisterComponents<Transform, Mesh, LightSource, Camera>("component.3d");
    }
    {
        using namespace MoonGlare::Component::Rect;
        provider->RegisterTypes<RectTransform, RectImage>("component");
        reg->RegisterComponents<RectTransform, RectImage>("component");
    }
}

} // namespace MoonGlare::Tools::Component
