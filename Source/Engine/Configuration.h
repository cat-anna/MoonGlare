#pragma once

#define ASSERT_HANDLE_TYPE(TYPE, HANDLE) /* TBD */

namespace MoonGlare {

	namespace Configuration {


		struct Handle {
			enum {
				GenerationBits = 16,
				IndexBits = 16,
				TypeBits = 32,//can be divided if needed (some flags?)
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
					ObjectBuffer = HugeBuffer,
				};
			};
		};

	} //namespace Configuration

using Handle = Space::Memory::TripleHandle64<
			Configuration::Handle::GenerationBits,
			Configuration::Handle::IndexBits,
			Configuration::Handle::TypeBits
		>;


using HandleSet = std::vector<Handle>;

} //namespace MoonGlare
