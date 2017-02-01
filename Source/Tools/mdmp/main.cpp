
#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>

#pragma warning ( push, 0 )

#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>  

#pragma warning ( pop )

const char *propline = " > ";

template<typename T>
std::string tohex(T t) {
	std::stringstream ss;
	ss << std::hex << std::showbase << t;
	return ss.str();
}

template<typename T>
const std::map<T, const char*> & getFields();

template<>
const std::map<aiPrimitiveType, const char*> & getFields<aiPrimitiveType>() {
	static const std::map<aiPrimitiveType, const char*> values = {
		{ aiPrimitiveType_POINT, "Point" },
		{ aiPrimitiveType_LINE, "Line" },
		{ aiPrimitiveType_TRIANGLE, "Triangle" },
		{ aiPrimitiveType_POLYGON, "Polygon" },
	};
	return values;
}

template<typename T>
std::string tobitfield(unsigned t) {
	std::stringstream ss;
	ss << tohex(t) << " [";

	auto &fields = getFields<T>();

	bool first = true;
	for (unsigned i = 0; i < 32; ++i) {
		if (t & (1 << i)) {
			if (!first) {
				ss << " ";
			}
			first = false;
			auto it = fields.find(static_cast<T>(1 << i));
			if (it == fields.end())
				ss << "?";
			else
				ss << it->second;
		}
	}

	ss << "]";
	return ss.str();
}

std::ostream& operator <<(std::ostream &out, const aiColor3D& vec) {
	out << "(" << vec.r << ";" << vec.g << ";" << vec.b << ")";
	return out;
}

std::ostream& operator <<(std::ostream &out, const aiVector3D& vec) {
	out << "(" << vec.x << ";" << vec.y << ";" << vec.z << ")";
	return out;
}

std::ostream& operator <<(std::ostream &out, const aiQuaternion& vec) {
	out << "(" << vec.x << ";" << vec.y << ";" << vec.z << ";" << vec.w << ")";
	return out;
}

struct DumpCfg {};

void DumpStructure(DumpCfg & cfg, const aiScene *scene, std::ostream &out) {
	std::vector<std::pair<int, const aiNode*>> queue;
	queue.emplace_back(0, scene->mRootNode);

	out << "STRUCTURE:\n";
	while (!queue.empty()) {
		auto item = queue.back();
		queue.pop_back();
		int level = item.first;
		auto node = item.second;

		std::string nodeline;
		for (unsigned i = 0; i < level; ++i)
			nodeline += "\t";

		out << nodeline << node->mName.data << "\n";

		if (node->mNumMeshes > 0) {
			out << nodeline << propline << "Meshes[" << node->mNumMeshes << "]:";
			for (unsigned i = 0; i < node->mNumMeshes; ++i) {
				out << " " << node->mMeshes[i];
			}
			out << "\n";
		}

		aiQuaternion q;
		aiVector3D pos;
		aiVector3D scale;
		node->mTransformation.Decompose(scale, q, pos);
		out << nodeline << propline << "Position: " << pos << "\n";
		out << nodeline << propline << "Quaternion: " << q << "\n";
		out << nodeline << propline << "Scale: " << scale << "\n";

		++level;
		for (int i = node->mNumChildren - 1; i >= 0; --i) {
			queue.emplace_back(level, node->mChildren[i]);
		}
	}
}

void DumpMeshes(DumpCfg & cfg, const aiScene *scene, std::ostream &out) {
	if (!scene->HasMeshes())
		return;

	out << "MESHES[" << scene->mNumMeshes << "]:\n";
	for (int i = 0; i < scene->mNumMeshes; ++i) {
		auto mesh = scene->mMeshes[i];

		out << i << "\n";
		out << "\tPrimitives: " << tobitfield<aiPrimitiveType>(mesh->mPrimitiveTypes) << "\n";
		out << "\tNumVertices: " << mesh->mNumVertices << "\n";
		out << "\tNumFaces: " << mesh->mNumFaces << "\n";
		//mVertices
		//mNormals
		//mTangents
		//mBitangents
		//aiColor4D* mColors[AI_MAX_NUMBER_OF_COLOR_SETS];
		//aiVector3D* mTextureCoords[AI_MAX_NUMBER_OF_TEXTURECOORDS];
		//unsigned int mNumUVComponents[AI_MAX_NUMBER_OF_TEXTURECOORDS];
		//aiFace* mFaces;
		out << "\tNumBones: " << mesh->mNumBones << "\n";
		//aiBone** mBones;
		out << "\tMaterialIndex: " << mesh->mMaterialIndex << "\n";
		out << "\tName: " << mesh->mName.data << "\n";
		out << "\tNumAnimMeshes: " << mesh->mNumAnimMeshes << "\n";
		//aiAnimMesh** mAnimMeshes;
	}
}

void DumpMaterials(DumpCfg & cfg, const aiScene *scene, std::ostream &out) {
	if (!scene->HasMaterials())
		return;

	auto dumptexture = [&out] (const aiMaterial *mat, aiTextureType textype, int texidx) {
		aiString path;
		aiTextureMapping mapping = (aiTextureMapping) 0;
		unsigned int uvindex = 0;
		float blend = 0;
		aiTextureOp op = (aiTextureOp) 0;
		aiTextureMapMode mapmode = (aiTextureMapMode) 0;

		if (mat->GetTexture(textype, texidx, &path, &mapping, &uvindex, &blend, &op, &mapmode) != AI_SUCCESS) {
			out << "\t\t\tError!\n";
			return;
		}

		out << "\t\t\tpath: " << path.data << "\n";
		out << "\t\t\tmapping: " << tohex((int) mapping) << "\n";
		out << "\t\t\tuvindex: " << uvindex << "\n";
		out << "\t\t\tblend: " << blend << "\n";
		out << "\t\t\top: " << tohex((int) op) << "\n";
		out << "\t\t\tmapmode: " << tohex((int) mapmode) << "\n";
	};

	out << "MATERIALS[" << scene->mNumMaterials << "]:\n";
	for (int i = 0; i < scene->mNumMaterials; ++i) {
		auto mat = scene->mMaterials[i];

		out << i << "\n";

		static const std::vector<std::pair<aiTextureType, const char *>> TexTypes{
			{ aiTextureType_DIFFUSE, "Diffuse" },
			{ aiTextureType_SPECULAR, "Specular" },
			{ aiTextureType_AMBIENT, "Ambient" },
			{ aiTextureType_EMISSIVE, "Emissive" },
			{ aiTextureType_HEIGHT, "Height" },
			{ aiTextureType_NORMALS, "Normals" },
			{ aiTextureType_SHININESS, "Shiness" },
			{ aiTextureType_OPACITY, "Opacity" },
			{ aiTextureType_DISPLACEMENT, "Displacement" },
			{ aiTextureType_LIGHTMAP, "Lightmap" },
			{ aiTextureType_REFLECTION, "Reflection" },
			{ aiTextureType_UNKNOWN, "Unknown" },
		};

		out << "\tTextures:\n";
		for (auto type : TexTypes) {
			auto c = mat->GetTextureCount(type.first);
			if (c == 0)
				continue;

			out << "\t\t" << type.second << ":\n";
			for (unsigned i = 0; i < c; ++i) {
				out << "\t\t" << i << "\n";
				dumptexture(mat, type.first, i);
			}
		}
	}
}

void DumpTextures(DumpCfg & cfg, const aiScene *scene, std::ostream &out) {
	if (!scene->HasTextures())
		return;

	out << "TEXTURES[" << scene->mNumTextures << "]:\n";
	for (int i = 0; i < scene->mNumTextures; ++i) {
		auto tex = scene->mTextures[i];
		if (tex->mHeight == 0) {
			out << i << " - binary data\n";
			out << "\tSize: " << tex->mWidth << "\n";
			out << "\tType: " << tex->achFormatHint << "\n";
		} else {
			out << i << " - Raw image\n";
			out << "\tWidth: " << tex->mWidth << "\n";
			out << "\tHeight: " << tex->mHeight << "\n";
		}
	}
}

void DumpLights(DumpCfg & cfg, const aiScene *scene, std::ostream &out) {
	if (!scene->HasLights())
		return;
	out << "LIGHTS[" << scene->mNumLights << "]:\n";
	for (int i = 0; i < scene->mNumLights; ++i) {
		auto light = scene->mLights[i];
		out << i << "\n";

		out << "\tName: " << light->mName.data << "\n";
		out << "\tType: " << (int) light->mType << "\n";
		out << "\tPosition: " << light->mPosition << "\n";
		out << "\tDirection: " << light->mDirection << "\n";
		out << "\tAttenuationConstant: " << light->mAttenuationConstant << "\n";
		out << "\tAttenuationLinear: " << light->mAttenuationLinear << "\n";
		out << "\tAttenuationQuadratic: " << light->mAttenuationQuadratic << "\n";
		out << "\tColorDiffuse: " << light->mColorDiffuse << "\n";
		out << "\tColorSpecular: " << light->mColorSpecular << "\n";
		out << "\tColorAmbient: " << light->mColorAmbient << "\n";
		out << "\tmAngleInnerCone: " << light->mAngleInnerCone << "\n";
		out << "\tmAngleOuterCone: " << light->mAngleOuterCone << "\n";
	}
}

void DumpAnimations(DumpCfg & cfg, const aiScene *scene, std::ostream &out) {
	if (!scene->HasAnimations())
		return;
	out << "ANIMATIONS[" << scene->mNumAnimations << "]:\n";
	for (int i = 0; i < scene->mNumAnimations; ++i) {
		auto anim = scene->mAnimations[i];

		out << i << "\n";
		out << "\tName: " << anim->mName.data << "\n";
		out << "\tTicks: " << anim->mDuration << "\n";
		out << "\tTicksPerSec: " << anim->mTicksPerSecond << "\n";
		out << "\tChannels: " << anim->mNumChannels << "\n";
		out << "\tMeshChannels: " << anim->mNumMeshChannels << "\n";

		for (int j = 0; j < anim->mNumChannels; ++j) {
			auto ch = anim->mChannels[j];
			out << "\t > CHANNEL " << j << "\n";
			out << "\t\tName: " << ch->mNodeName.data << "\n";
			out << "\t\tNumPositionKeys: " << ch->mNumPositionKeys << "\n";
			out << "\t\tNumRotationKeys: " << ch->mNumRotationKeys << "\n";
			out << "\t\tNumScalingKeys: " << ch->mNumScalingKeys << "\n";
			out << "\t\tPreState: " << (int) ch->mPreState << "\n";
			out << "\t\tPostState: " << (int)ch->mPostState << "\n";
		}
	}
}

void DumpCameras(DumpCfg & cfg, const aiScene *scene, std::ostream &out) {
	if (!scene->HasCameras())
		return;
	out << "CAMERAS[" << scene->mNumCameras << "]:\n";
}

void DumpScene(DumpCfg & cfg, const aiScene *scene) {
	auto &out = std::cout;
	DumpStructure(cfg, scene, out);
	DumpMeshes(cfg, scene, out);
	DumpMaterials(cfg, scene, out);
	DumpAnimations(cfg, scene, out);
	DumpTextures(cfg, scene, out);
	DumpLights(cfg, scene, out);
	DumpCameras(cfg, scene, out);
}

int main(int argc, char ** argv) {

	DumpCfg cfg;
	//TODO: load cfg

	const char *path;
	if (argc < 2) {
		path = R"(d:\Programowanie\Projekty\!gry\MoonGlare\MazeGame\modules\MazeGame\Models\KnightStatue\knight_statue.blend)";
	} else {
		path = argv[1];
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
											 aiProcess_JoinIdenticalVertices |/* aiProcess_PreTransformVertices | */aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType);

	if (!scene) {
		std::cout << "Unable to open file!\n";
		return 1;
	}

	try {
		DumpScene(cfg, scene);
	}
	catch (...) {

	}

	return 0;
}
