
#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

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

template<>
const std::map<aiLightSourceType, const char*> & getFields<aiLightSourceType>() {
    static const std::map<aiLightSourceType, const char*> values = {
    { aiLightSource_UNDEFINED, "undefined" },
    { aiLightSource_DIRECTIONAL, "directional" },
    { aiLightSource_POINT, "point" },
    { aiLightSource_SPOT, "spot" },
    { aiLightSource_AMBIENT, "ambient" },
    { aiLightSource_AREA, "area" },
    };
    return values;
}

template<>
const std::map<aiMetadataType, const char*> & getFields<aiMetadataType>() {
    static const std::map<aiMetadataType, const char*> values = {
    { AI_BOOL, "bool" },
    { AI_INT32, "int32" },
    { AI_UINT64, "uint32" },
    { AI_FLOAT, "float" },
    { AI_DOUBLE, "double" },
    { AI_AISTRING, "string" },
    { AI_AIVECTOR3D, "vec3d" },
    };
    return values;
}

template<>
const std::map<aiTextureMapping, const char*> & getFields<aiTextureMapping>() {
    static const std::map<aiTextureMapping, const char*> values = {
        { aiTextureMapping_UV, "UV" },
        { aiTextureMapping_SPHERE, "SPHERE" },
        { aiTextureMapping_CYLINDER, "CYLINDER" },
        { aiTextureMapping_BOX, "BOX" },
        { aiTextureMapping_PLANE, "PLANE" },
        { aiTextureMapping_OTHER, "OTHER" },
    };
    return values;
}

template<>
const std::map<aiTextureMapMode, const char*> & getFields<aiTextureMapMode>() {
    static const std::map<aiTextureMapMode, const char*> values = {
        { aiTextureMapMode_Wrap, "Wrap" },
        { aiTextureMapMode_Clamp, "Clamp" },
        { aiTextureMapMode_Decal, "Decal" },
        { aiTextureMapMode_Mirror, "Mirror" },
    };
    return values;
}

template<>
const std::map<aiTextureOp, const char*> & getFields<aiTextureOp>() {
    static const std::map<aiTextureOp, const char*> values = {
        { aiTextureOp_Multiply, "Multiply" },
        { aiTextureOp_Add, "Add" },
        { aiTextureOp_Subtract, "Subtract" },
        { aiTextureOp_Divide, "Divide" },
        { aiTextureOp_SmoothAdd, "SmoothAdd" },
        { aiTextureOp_SignedAdd, "SignedAdd" },
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

template<typename T>
std::string convertEnum(T t) {
    std::stringstream ss;
    ss << tohex(t) << " [";

    auto &fields = getFields<T>();
    
    auto it = fields.find(t);
    if (it == fields.end()) {
        ss << "?";
    } else {
        ss << it->second;
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

struct DumpCfg {
    bool dumpTextures = false;
    std::string path;
    std::string output;
    std::string dumpPath;
};

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
		for (int i = 0; i < level; ++i)
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
        if (node->mMetaData && node->mMetaData->mNumProperties > 0) {
            out << nodeline << propline << "PROPERTIES[" << node->mMetaData->mNumProperties << "]:";
            for (unsigned mdi = 0; mdi < node->mMetaData->mNumProperties; ++mdi)
                out << " " << node->mMetaData->mKeys[mdi].data 
                    << "<" << convertEnum(node->mMetaData->mValues[mdi].mType) << ">";
            out << "\n";
        }

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
	for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
		auto mesh = scene->mMeshes[i];

		out << i << "\n";
        out << "\tName: " << mesh->mName.data << "\n";
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

void DumpMaterialProps(DumpCfg & cfg, std::ostream & out, aiMaterial * mat) {
    out << "\tProperties[" << mat->mNumProperties << "]:\n";
    for (unsigned p = 0; p < mat->mNumProperties; ++p) {
        auto prop = mat->mProperties[p];
        out << "\t\t";
        out << prop->mKey.data << " = ";

        const char *pname = prop->mKey.data;
        unsigned int type = 0;
        unsigned int idx = 0;

        try {
            unsigned cnt = 1;
            if (aiColor4D v;  mat->Get(pname, type, idx, &v, &cnt) == aiReturn_SUCCESS) {
                out << v.r << " " << v.g << " " << v.b << " " << v.a << " [" << cnt << "]" << "\n";
                continue;
            }
        }
        catch (...) {}

        try {
            unsigned cnt = 1;
            if (aiColor3D v;  mat->Get(pname, type, idx, &v, &cnt) == aiReturn_SUCCESS) {
                out << v.r << " " << v.g << " " << v.b << " [" << cnt << "]" << "\n";
                continue;
            }
        }
        catch (...) {}

        try {
            unsigned cnt = 16;
            if (float v[16];  mat->Get(pname, type, idx, v, &cnt) == aiReturn_SUCCESS) {
                auto *vp = v;
                for (; cnt; --cnt, ++vp) {
                    out << *vp << ";";
                }
                out << "\n";
                continue;
            }
        }
        catch (...) {}

        try {
            unsigned cnt = 16;
            if (int v[16];  mat->Get(pname, type, idx, v, &cnt) == aiReturn_SUCCESS) {
                auto *vp = v;
                for (; cnt; --cnt, ++vp) {
                    out << *vp << ";";
                }
                out << "\n";
                continue;
            }
        }
        catch (...) {}

        try {
            if (aiString v;  mat->Get(pname, type, idx, v) == aiReturn_SUCCESS) {
                out << v.data << "\n";
                continue;
            }
        }
        catch (...) {}
        try {
            unsigned cnt = 1024;
            if (char v[1024];  mat->Get(pname, type, idx, v, &cnt) == aiReturn_SUCCESS) {
                out << v << "[" << cnt << "]" << "\n";
                continue;
            }
        }
        catch (...) {}
        out << "\n";
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
        aiTextureMapMode mapmode[2] = {};

		if (mat->GetTexture(textype, texidx, &path, &mapping, &uvindex, &blend, &op, mapmode) != AI_SUCCESS) {
			out << "\t\t\tError!\n";
			return;
		}

		out << "\t\t\tpath: " << path.data << "\n";
		out << "\t\t\tmapping: " << convertEnum<aiTextureMapping>(mapping) << "\n";
		out << "\t\t\tuvindex: " << uvindex << "\n";
		out << "\t\t\tblend: " << blend << "\n";
		out << "\t\t\top: " << convertEnum<aiTextureOp>(op) << "\n";
        out << "\t\t\tmapmode: " << convertEnum<aiTextureMapMode>(mapmode[0]) << " ; " << convertEnum<aiTextureMapMode>(mapmode[1]) << "\n";
	};

	out << "MATERIALS[" << scene->mNumMaterials << "]:\n";
	for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
		auto mat = scene->mMaterials[i];

		out << i << "\n";
        DumpMaterialProps(cfg, out, mat);

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
			for (unsigned i2 = 0; i2 < c; ++i2) {
				out << "\t\t" << i2 << "\n";
				dumptexture(mat, type.first, i2);
			}
		}
	}
}

void DumpTextures(DumpCfg & cfg, const aiScene *scene, std::ostream &out) {
	if (!scene->HasTextures())
		return;
    if (cfg.dumpTextures) {
        boost::filesystem::create_directories(cfg.dumpPath);
    }

	out << "TEXTURES[" << scene->mNumTextures << "]:\n";
	for (unsigned i = 0; i < scene->mNumTextures; ++i) {
		auto tex = scene->mTextures[i];
		if (tex->mHeight == 0) {
			out << i << " - binary data\n";
			out << "\tSize: " << tex->mWidth << "\n";
			out << "\tType: " << tex->achFormatHint << "\n";
        if (cfg.dumpTextures) {
            std::ofstream of(cfg.dumpPath + "tex" + std::to_string(i) + "." + tex->achFormatHint, std::ios::binary | std::ios::out);
            of.write((char*)tex->pcData, tex->mWidth);
            of.close();
        }
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
	for (unsigned i = 0; i < scene->mNumLights; ++i) {
		auto light = scene->mLights[i];
		out << i << "\n";

		out << "\tName: " << light->mName.data << "\n";
		out << "\tType: " << convertEnum(light->mType) << "\n";
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
	for (unsigned i = 0; i < scene->mNumAnimations; ++i) {
		auto anim = scene->mAnimations[i];

		out << i << "\n";
		out << "\tName: " << anim->mName.data << "\n";
		out << "\tTicks: " << anim->mDuration << "\n";
		out << "\tTicksPerSec: " << anim->mTicksPerSecond << "\n";
		out << "\tChannels: " << anim->mNumChannels << "\n";
		out << "\tMeshChannels: " << anim->mNumMeshChannels << "\n";

		for (unsigned j = 0; j < anim->mNumChannels; ++j) {
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
    std::ofstream of(cfg.output);

    std::ostream &out = of;
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

    namespace po = boost::program_options;

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce this help message")
        ("dump-textures", "dump textures")
        ("input", po::value<std::string>(), "input file")
        ("output", po::value<std::string>(), "output")
        ;
    
    po::positional_options_description p;
    p.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
        options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") || !vm.count("input")) {
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("dump-textures")) {
        cfg.dumpTextures = true;
    }

    cfg.path = vm["input"].as<std::string>();
    cfg.output = cfg.path + ".mdmp";
    cfg.dumpPath = cfg.path + ".dump/";

    if (vm.count("output")) {
        cfg.output = vm["output"].as<std::string>();
    }

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(cfg.path.c_str(), 0
											//| aiProcess_JoinIdenticalVertices /* aiProcess_PreTransformVertices | */
                                            | aiProcess_Triangulate 
                                            //| aiProcess_GenUVCoords 
                                            //| aiProcess_SortByPType
                                        );

	if (!scene) {
		std::cout << "Unable to open file!\n";
		return 1;
	}

	try {
		DumpScene(cfg, scene);
	}
	catch (std::exception e) {
        std::cout << e.what();
	}

	return 0;
}
