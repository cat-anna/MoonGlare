#pragma once

#define ASSERT_HANDLE_TYPE(TYPE, HANDLE) /* TBD */

namespace MoonGlare {

	namespace Configuration {

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
				TinyBuffer = 64,
				SmallBuffer = 256,
				MediumBuffer = 1024,
				HugeBuffer = 4096, 

				EntityBuffer = HugeBuffer,
				ComponentBuffer = SmallBuffer,//there is no need for more now
			};

			struct Static {
				enum {
					TransformComponent	= HugeBuffer,

					ObjectBuffer = HugeBuffer,
					EntityStorage = HugeBuffer,
				};
			};
		};

	} //namespace Configuration

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

using HandleSet = std::vector<Handle>;

} //namespace MoonGlare
