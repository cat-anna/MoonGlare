#include <pch.h>
#include "Graphic.h"
//#include <MoonGlare.h>

namespace Graphic { 

void VAO::Finalize() {
	auto swp = new VAO();
	swp->swap(*this);

	GetRenderDevice()->RequestContextManip([swp]() {
		swp->Free();
		delete swp;
	});
}
 
void VAO::Initialize(VertexVector &Verticles, TexCoordVector &TexCoords, NormalVector &Normals, IndexVector &Index) {
	struct RequestData {
		VertexVector Verticles;
		TexCoordVector TexCoords;
		NormalVector Normals;
		IndexVector Index;
	};

	auto data = std::make_shared<RequestData>();

	data->Verticles.swap(Verticles);
	data->TexCoords.swap(TexCoords);
	data->Normals.swap(Normals);
	data->Index.swap(Index);

	GetRenderDevice()->RequestContextManip([data, this]() {
		New();
		Bind();
		if (!data->Verticles.empty())
			SetDataChannel<3, float>(CoordChannel, &data->Verticles[0][0], data->Verticles.size());
		if (!data->TexCoords.empty())
			SetDataChannel<2, float>(UVChanel, &data->TexCoords[0][0], data->TexCoords.size());
		if (!data->Normals.empty())
			SetDataChannel<3, float>(NormalChannel, &data->Normals[0][0], data->Normals.size());
		WriteIndexes<>(&data->Index[0], data->Index.size());
		UnBind();
	});
}

} //namespace Graphic
