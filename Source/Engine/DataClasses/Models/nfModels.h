#ifndef NFMODELSH
#define NFMODELSH

namespace DataClasses {
	namespace Models {
		class iModel;
		class iSimpleModel;
		using ModelPtr = std::shared_ptr<iModel>;

		using ModelClassRegister = GabiLib::DynamicClassRegisterDeleter < iModel, ResourceFinalizer<iModel>, const string& >;
	}
	using Models::ModelPtr;
}

using DataClasses::Models::iModel;
using DataClasses::Models::iSimpleModel;

#endif // NFMODELSH 
