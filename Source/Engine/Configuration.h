#pragma once

#define ASSERT_HANDLE_TYPE(TYPE, HANDLE) /* TBD */

namespace MoonGlare {

	namespace Configuration {

		using HashID = uint32_t;

		struct Entity {
			enum {
				GenerationBits = 14,
				IndexBits = 14,
				TypeBits = 4, 
				ValueBits = 0,

				GenerationLimit = 1 << GenerationBits,
				IndexLimit = 1 << IndexBits,
				TypeLimit = 1 << TypeBits,
			};

			struct Types {
				enum {
					//TBD
					Unknown,
					Object,
				};
			};
		};

		struct Handle {
			enum {
				GenerationBits = 12,
				IndexBits = 12,
				TypeBits = 8,
				ValueBits = 0,

				GenerationLimit = 1 << GenerationBits,
				IndexLimit = 1 << IndexBits,
				TypeLimit = 1 << TypeBits,
			};

			struct Types {
				enum {
					Invalid,
					Object,
				};
			};
		};

		struct Storage {
			enum {
				TinyBuffer = 32,
				SmallBuffer = 256,
				MediumBuffer = 1024,
				HugeBuffer = 4096, 

				EntityBuffer = HugeBuffer,
				ComponentBuffer = HugeBuffer,

				MaxComponentCount = TinyBuffer,
			};

			struct Static {
				enum {
					ObjectBuffer = HugeBuffer,
					EntityStorage = HugeBuffer,
				};
			};
		};

		namespace Input {
			enum {
				MaxKeyCode = 512,
				MaxInputStates = 32,
				MaxMouseButton = 8,
				MaxMouseAxes = 4,
			};
			static const float StaticMouseSensivity = 0.005f;
			static const float StaticMouseScrollSensivity = 1.0f;
		};

		struct Console {
			enum {
				MaxConsoleLines = 20,
			};
		};
	} //namespace Configuration

using Configuration::HashID;

using Entity = Space::Memory::TripleHandle32<
			Configuration::Entity::GenerationBits,
			Configuration::Entity::IndexBits,
			Configuration::Entity::TypeBits//,
		//	Configuration::Entity::ValueBits
		>;

using Handle = Space::Memory::TripleHandle32<
			Configuration::Handle::GenerationBits,
			Configuration::Handle::IndexBits,
			Configuration::Handle::TypeBits
		>;

using HandleIndex = Handle::Index_t;
using HandleSet = std::vector<Handle>;

} //namespace MoonGlare

inline std::ostream& operator<<(std::ostream &o, MoonGlare::Entity e) {
	char buf[128];
	sprintf_s(buf, "(Entity; Index:%d; Type:%d; Generation:%d)", e.GetIndex(), e.GetType(), e.GetGeneration());
	return o << buf;
}

inline std::ostream& operator<<(std::ostream &o, MoonGlare::Handle h) {
	char buf[128];
	sprintf_s(buf, "(Handle; Index:%d; Type:%d; Generation:%d)", h.GetIndex(), h.GetType(), h.GetGeneration());
	return o << buf;
}
