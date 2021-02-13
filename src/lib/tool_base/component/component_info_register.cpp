#define WANTS_TYPE_INFO
#define WANTS_SERIALIZATION

#include "component_info_register.hpp"
#include "eigen_math_type_info.hpp"
#include <attribute_map_builder.hpp>
#include <component/camera.hpp>
#include <component/light_source.hpp>
#include <component/mesh.hpp>
#include <component/name.hpp>
#include <component/transform.hpp>
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
        provider->RegisterTypes<Transform, Mesh, LightSource, Name, Camera>("Component");
        reg->RegisterComponents<Transform, Mesh, LightSource, Name, Camera>("Component");
    }
}

} // namespace MoonGlare::Tools::Component
