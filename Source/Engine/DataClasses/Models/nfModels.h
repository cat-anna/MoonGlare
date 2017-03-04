#ifndef NFMODELSH
#define NFMODELSH

namespace MoonGlare {
namespace DataClasses {
	namespace Models {
		class iModel;
		using ModelPtr = std::shared_ptr<iModel>;
	}
	using Models::ModelPtr;
}
} //namespace MoonGlare 

using MoonGlare::DataClasses::Models::iModel;

#endif // NFMODELSH 
