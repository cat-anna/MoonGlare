#ifndef NFMODELSH
#define NFMODELSH

namespace MoonGlare {
namespace DataClasses {
	namespace Models {
		class iModel;
		class iSimpleModel;
		using ModelPtr = std::shared_ptr<iModel>;

		using ModelClassRegister = Space::DynamicClassRegisterDeleter < iModel, ResourceFinalizer<iModel>, const string& >;
	}
	using Models::ModelPtr;
}
} //namespace MoonGlare 

using MoonGlare::DataClasses::Models::iModel;
using MoonGlare::DataClasses::Models::iSimpleModel;

#endif // NFMODELSH 
