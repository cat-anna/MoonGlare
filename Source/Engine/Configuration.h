#pragma once

#define ASSERT_HANDLE_TYPE(TYPE, HANDLE) /* TBD */

namespace MoonGlare {

	namespace Configuration {

		using HashID = uint32_t;

		struct Entity {
			enum {
				GenerationBits = 12,
				IndexBits = 12,
				TypeBits = 8, 
				ValueBits = 0,
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
