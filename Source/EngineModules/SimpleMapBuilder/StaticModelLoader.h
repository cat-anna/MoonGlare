#pragma once 

namespace MoonGlare {
namespace Modules {
namespace SimpleMap {

struct Corridor;

using DataClasses::Models::SimpleModelConstructor;

struct StyleConfig {
	string Name;
	struct {
		string Wall, Ceiling, Floor;
	} Materials;
};

struct Junction {
	StyleConfig *Style;
	math::vec3 pos;
	string Name;
	std::list<Corridor*> Corridors;
};

struct Corridor {
	StyleConfig *Style;
	Junction *Entrance, *Exit;
	float SubDivs;
};

struct Material {
	std::string Name;
	std::string Edges;
	std::string TextureURI;
};

struct StaticModelLoader {
	using QuadArray3 = std::array<math::vec3, 4>;
	using QuadArray2 = std::array<math::vec2, 4>;

	StaticModelLoader(const StaticModelLoader& oth) {
		m_UnitSize = oth.m_UnitSize;
		m_HalfUnitSize = oth.m_HalfUnitSize;
		m_PositionFactor = oth.m_PositionFactor;
		m_FloorNormal = oth.m_FloorNormal;
		m_CeilingNormal = oth.m_CeilingNormal;
		m_TextureBase = oth.m_TextureBase;
		m_JunctionMap = oth.m_JunctionMap;
		m_StyleConfig = oth.m_StyleConfig;
		m_Materials = oth.m_Materials;
		m_MeshMap = oth.m_MeshMap;
		m_CorridorList = oth.m_CorridorList;
		m_DoubleWalls = oth.m_DoubleWalls;
		//m_Constructor = oth.m_Constructor;
	}
	StaticModelLoader()
			: m_PositionFactor(1) 
			, m_UnitSize(1)
			, m_HalfUnitSize(0.5f)
			, m_DoubleWalls(true)
	{
		for (unsigned i = 0; i < m_FloorNormal.size(); ++i) {
			m_FloorNormal[i] = math::vec3(0, 1, 0);
			m_CeilingNormal[i] = math::vec3(0, -1, 0);
		}

		m_TextureBase[0] = math::vec2(1, 0);
		m_TextureBase[1] = math::vec2(1, 1);
		m_TextureBase[2] = math::vec2(0, 1);
		m_TextureBase[3] = math::vec2(0, 0);
	}

	Handle GenerateResource() {
		if (!Validate() || !Generate()) {
			AddLogf(Error, "Failed to generate mode!");
			return Handle();
		}

		DataClasses::ModelPtr model(GetConstructor()->GenerateModel());
		auto rt = Core::GetEngine()->GetWorld()->GetResourceTable();

		Handle hout;
		rt->Allocate(std::move(model), hout);
		return hout;
	}

	std::unique_ptr<SimpleModelConstructor>& GetConstructor() { return m_Constructor; }
	
	bool Load(xml_node node) {
		LoadMaterials(node);
		LoadStyles(node);
		LoadJunctions(node);
		LoadCorridors(node);
		return true;
	}

	bool Generate() {
		try {
			m_Constructor.reset(new SimpleModelConstructor());
			m_Constructor->GenerateShape(true);

			float ws = 0.05f;
			
			for (auto &it : m_CorridorList) {
				GenerateCorridor(it, m_UnitSize, 0, false);
				if (m_DoubleWalls) {
					auto us = m_UnitSize + ws * 2.0f;
					GenerateCorridor(it, us, ws, true);
				}
			}
			for (auto &it : m_JunctionMap) {
				GenerateJunction(it.second, 0, 0);
				if(m_DoubleWalls)
					GenerateJunction(it.second, -ws, 2 * ws);
			}

		}
		catch (...) {
			return false;
		}
		return true;
	}

	void AddJunction(const char *Name, float X, float Y, float Z, const char *Style) {
		auto *style = GetStyleConfig(Style);
		string sName = Name;
		m_JunctionMap[sName] = Junction{style, math::vec3(X, Y, Z) * m_PositionFactor, sName};
	}
	void AddCorridor(const char *Entrance, const char *Exit, const char *Style, float SubDivs) {
		Junction *j1 = GetJunction(Entrance);
		Junction *j2 = GetJunction(Exit);
		if (!j1 || !j2) throw false;
		m_CorridorList.emplace_back(
				Corridor{
					GetStyleConfig(Style),
					j1, j2, SubDivs
				}
			);
		Corridor *c = &m_CorridorList.back();
		j1->Corridors.push_back(c);
		j2->Corridors.push_back(c);
	}
	void AddStyle(const char *Name, const char *Wall, const char *Ceiling, const char *Floor) {
		StyleConfig sc;
		sc.Name = Name;
		sc.Materials.Wall = Wall;
		sc.Materials.Ceiling = Ceiling;
		sc.Materials.Floor = Floor;
		m_StyleConfig[sc.Name] = sc;
	}
	void AddMaterial(const char *Name, const char *texuri, const char *Edges) {
		Material m;
		m.Name = Name;
		m.Edges = Edges;
		m.TextureURI = texuri;
		m_Materials[m.Name] = m;
	}

	math::vec3 VectorPointToCoords(const math::vec2 &pos) {
		math::vec3 v(pos.x, 0, pos.y);
		v *= m_PositionFactor;
		v *= m_UnitSize;
		AddLog(Debug, "Calculated cords from " << pos << " to " << v);
		return v;
	}

	void SetPositionFactor(float x, float y, float z) {
		m_PositionFactor = math::vec3(x, y, z);
	}

	bool Validate() {
		if (m_Materials.empty()) return false;
		if (m_JunctionMap.empty()) return false;
		if (m_StyleConfig.empty()) return false;
		if (m_CorridorList.empty()) return false;
		return true;
	}

	math::vec3 GetUnitSize() const { return m_UnitSize; }
	void SetUnitSize(const math::vec3 &n) {
		m_UnitSize = n;
		m_HalfUnitSize = m_UnitSize / 2.0f;
	}

	bool GetDoubleWalls() const { return m_DoubleWalls; }
	void SetDoubleWalls(bool v) { m_DoubleWalls = v; }
private:
	math::vec3 m_UnitSize;
	math::vec3 m_HalfUnitSize;
	math::vec3 m_PositionFactor;
	QuadArray3 m_FloorNormal, m_CeilingNormal;
	QuadArray2 m_TextureBase;

	std::unordered_map<string, Junction> m_JunctionMap;
	std::unordered_map<string, StyleConfig> m_StyleConfig;
	std::unordered_map<string, Material> m_Materials;
	std::unordered_map<string, SimpleModelConstructor::cMesh*> m_MeshMap;
	std::list<Corridor> m_CorridorList;
	std::unique_ptr<SimpleModelConstructor> m_Constructor;
	bool m_DoubleWalls;

	void GenerateJunction(Junction &it, float dY, float XZmult) {
		QuadArray3 Vertex;
		QuadArray3 VertexUp;
		Vertex.fill(it.pos);
		float m = 1.0f + XZmult;
		Vertex[0] += math::vec3( 1, 0, -1) * m_HalfUnitSize * m;
		Vertex[1] += math::vec3(-1, 0, -1) * m_HalfUnitSize * m;
		Vertex[2] += math::vec3(-1, 0,  1) * m_HalfUnitSize * m;
		Vertex[3] += math::vec3( 1, 0,  1) * m_HalfUnitSize * m;

		for (auto &it : Vertex) 
			it[1] += dY;
		GetMesh(it.Style->Materials.Floor)->PushQuad(&Vertex[0], &m_FloorNormal[0], &m_TextureBase[0]);

		for (int i = 0; i < 4; ++i)
			VertexUp[i] = Vertex[i] + math::vec3(0, m_UnitSize[1] - 2.0*dY, 0);
		GetMesh(it.Style->Materials.Ceiling)->PushQuad(&VertexUp[0], &m_FloorNormal[0], &m_TextureBase[0]);

		bool Left = true, Right = true, Front = true, Back = true;
		for (auto &c : it.Corridors) {
			math::vec3 d; 
			d = it.pos - (&it == c->Entrance ? c->Exit->pos : c->Entrance->pos);
			Left = Left && !(d[0] < 0);
			Right = Right && !(d[0] > 0);
			Front = Front && !(d[2] < 0);
			Back = Back && !(d[2] > 0);
		}

		QuadArray3 VertexWall;
		QuadArray3 NormallWall;
		QuadArray2 &TextureWall = m_TextureBase;
		auto mesh = GetMesh(it.Style->Materials.Wall);
		int outgoing = 0;
		if (Left) {
			VertexWall[0] = Vertex[0];
			VertexWall[1] = VertexUp[0];
			VertexWall[2] = VertexUp[3];
			VertexWall[3] = Vertex[3];
			mesh->PushQuad(&VertexWall[0], &NormallWall[0], &TextureWall[0]);
			++outgoing;
		}
		if (Right) {
			VertexWall[0] = Vertex[1];
			VertexWall[1] = VertexUp[1];
			VertexWall[2] = VertexUp[2];
			VertexWall[3] = Vertex[2];
			mesh->PushQuad(&VertexWall[0], &NormallWall[0], &TextureWall[0]);
			++outgoing;
		}
		if (Front) {
			VertexWall[0] = Vertex[3];
			VertexWall[1] = VertexUp[3];
			VertexWall[2] = VertexUp[2];
			VertexWall[3] = Vertex[2];
			mesh->PushQuad(&VertexWall[0], &NormallWall[0], &TextureWall[0]);
			++outgoing;
		}
		if (Back) {
			VertexWall[0] = Vertex[1];
			VertexWall[1] = VertexUp[1];
			VertexWall[2] = VertexUp[0];
			VertexWall[3] = Vertex[0];
			mesh->PushQuad(&VertexWall[0], &NormallWall[0], &TextureWall[0]);
			++outgoing;
		}
		if (outgoing >= 4) {
			AddLog(Debug, "No outgoing corridors from junction: " << it.Name);
		}
	}

	void GenerateCorridor(Corridor & it, const math::vec3 &UnitSize, float dpos, bool OutNormals) {
		auto hs = UnitSize / 2.0f;
		math::vec3 vector = it.Exit->pos - it.Entrance->pos;
		math::vec3 direction = vector / glm::length(vector); 

		math::vec3 dstart = (direction * (1.0f + 2.0f * dpos)) / 2.0f;
		math::vec3 dend = -dstart;

		math::vec3 vpos(0, -dpos, 0);

		float angle = d2math::D2Line_XZ(it.Entrance->pos, it.Entrance->pos + direction).AngleWithOX();
		if(abs(direction.z) < 0.01) angle += math::Constants::pi::value;
		math::vec3 a = d2math::PointOnCircle_XZ(math::vec3(), angle, hs[0]);

		std::array<math::vec3, 4> Vertex;

		Vertex[1] = it.Entrance->pos	+ math::vec3(-a[0], 0, -a[2]) + vpos + dstart;
		Vertex[0] = it.Entrance->pos	+ math::vec3( a[0], 0,  a[2]) + vpos + dstart;
		Vertex[2] = it.Exit->pos		+ math::vec3(-a[0], 0, -a[2]) + vpos + dend;
		Vertex[3] = it.Exit->pos		+ math::vec3( a[0], 0,  a[2]) + vpos + dend;

		QuadArray2 Texture(m_TextureBase);
		QuadArray2 TextureWall(m_TextureBase);

		{
			math::vec2 TexMult;
			float m = 1;// ((abs(vector[0]) > 0 && abs(vector[2]) > 0) ? 1.0f : -1.0f);

			if (abs(vector[0]) > 0) {
				auto tmp = Texture[0];
				for (int i = 1; i < 4; ++i)
					Texture[i - 1] = Texture[i];
				Texture[3] = tmp;
			}

			if (abs(vector[0]) > abs(vector[2])) 
				m *= -1.0f;
			float length = glm::length(vector);
			//AddLog(Hint, "vec:" << vector << "  l:" << length);
			TexMult[0] = it.SubDivs * -(abs(vector[2]) - 1.0f);
			TexMult[1] = it.SubDivs * -(abs(vector[0]) - 1.0f);
			for (int i = 0; i < 2; ++i)
				if (abs(TexMult[i]) < 0.001f) TexMult[i] = 1.0f;
			for (int i = 0; i < 4; ++i) {
				Texture[i][0] *= -TexMult[0];
				Texture[i][1] *= TexMult[1];
				TextureWall[i][0] *= length * it.SubDivs;
				//if (abs(vector[0]) > 0 || abs(vector[2]) > 0)
					//Texture[i][0] *= it.SubDivs *vector[0];
				//if ()
					//Texture[i][0] *= it.SubDivs;// *vector[2];
			}
			if (vector[2] < 0 || vector[0] > 0) {
				std::swap(Texture[0], Texture[3]);
				std::swap(Texture[1], Texture[2]);
			}
		}
		
		GetMesh(it.Style->Materials.Floor)->PushQuad(&Vertex[0], &(OutNormals?m_CeilingNormal:m_FloorNormal)[0], &Texture[0]);
		std::array<math::vec3, 4> VertexUp = Vertex;
		for (int i = 0; i < 4; ++i)
			VertexUp[i].y += UnitSize[1];
		GetMesh(it.Style->Materials.Ceiling)->PushQuad(&VertexUp[0], &(OutNormals?m_FloorNormal:m_CeilingNormal)[0], &Texture[0]);

		auto mesh = GetMesh(it.Style->Materials.Wall);

		std::array<math::vec3, 4> VertexWalla;
		std::array<math::vec3, 4> WallNormal;

		auto aa = glm::normalize(a * (OutNormals?1.0f:-1.0f));
		for (auto &jt : WallNormal)
			jt = aa;

		VertexWalla[0] = Vertex[0];
		VertexWalla[1] = VertexUp[0];
		VertexWalla[2] = VertexUp[3];
		VertexWalla[3] = Vertex[3];
		mesh->PushQuad(&VertexWalla[0], &WallNormal[0], &TextureWall[0]);

		aa = glm::normalize(a * (OutNormals?-1.0f:1.0f));
		for (auto &jt : WallNormal)
			jt = aa;

		VertexWalla[0] = Vertex[1];
		VertexWalla[1] = VertexUp[1];
		VertexWalla[2] = VertexUp[2];
		VertexWalla[3] = Vertex[2];
		mesh->PushQuad(&VertexWalla[0], &WallNormal[0], &TextureWall[0]);
	}

	void LoadMaterials(xml_node node) {
		XML::ForEachChild(node.child("Materials"), "Material", [this](xml_node node) -> int {
			Material m;
			m.Name = node.attribute("Name").as_string();
			xml_node Texture = node.child("Texture");
			m.Edges = Texture.attribute("Edges").as_string();
			m.TextureURI = Texture.child("File").text().as_string();
			m_Materials[m.Name] = m;
			return 0;
		});
	}

	void LoadStyles(xml_node node) {
		XML::ForEachChild(node.child("StyleConfig"), "Style", [this](xml_node node) -> int {
			StyleConfig s;
			s.Name = node.attribute("Name").as_string();
			xml_node materials = node.child("Materials");
			s.Materials.Wall = materials.attribute("Wall").as_string();
			s.Materials.Ceiling = materials.attribute("Ceiling").as_string();
			s.Materials.Floor = materials.attribute("Floor").as_string();
			m_StyleConfig[s.Name] = s;
			return 0;
		});	
	}

	void LoadJunctions(xml_node node) {
		XML::ForEachChild(node.child("Junctions"), "Node", [this](xml_node node) -> int {
			Junction j;
			XML::Vector::Read(node, 0, j.pos);
			j.Style = GetStyleConfig(node.child("Style").attribute("Name").as_string("Default"));
			j.pos.y = 0;
			j.Name = node.attribute("Name").as_string();
			m_JunctionMap[j.Name] = j;
			return 0;
		});	
	}

	void LoadCorridors(xml_node node) {
		XML::ForEachChild(node.child("Corridors"), "Item", [this](xml_node node) -> int {
			m_CorridorList.push_back(Corridor()); 
			Corridor &c = m_CorridorList.back();

			xml_node Entrance = node.child("Entrance");
			c.Entrance = GetJunction(Entrance.attribute("Junction").as_string());
			c.Entrance->Corridors.push_back(&c);

			xml_node Exit = node.child("Exit");
			c.Exit = GetJunction(Exit.attribute("Junction").as_string());
			c.Exit->Corridors.push_back(&c);

			c.SubDivs = node.child("SubDivs").attribute("Value").as_float(1);
			c.Style = GetStyleConfig(node.child("Style").attribute("Name").as_string("Default"));
			return 0;
		});
	}

	SimpleModelConstructor::cMesh* GetMesh(const string& Name) {
		auto it = m_MeshMap.find(Name);
		if (it != m_MeshMap.end())
			return it->second;

		auto matinfo = GetMaterial(Name);
		if (!matinfo) {
			AddLog(Error, "There is no material " << Name);
			throw false;
		}

		auto mesh = m_Constructor->NewMesh();
		auto mat = m_Constructor->NewMaterial();
		mesh->SelectMaterial(mat->GetID());
		mat->m_Edges = matinfo->Edges;
		mat->m_TextureURI = matinfo->TextureURI;
		m_MeshMap[Name] = mesh;
		return mesh; 	
	}

	Junction* GetJunction(const string &Name) {
		auto it = m_JunctionMap.find(Name);
		if (it != m_JunctionMap.end())
			return &it->second;
		else {
			AddLog(Error, "There is no junction " << Name);
			throw false;
		}
	}

	StyleConfig* GetStyleConfig(const string &Name) {
		auto it = m_StyleConfig.find(Name);
		if (it != m_StyleConfig.end())
			return &it->second;
		else {
			it = m_StyleConfig.find("Default");
			if (it != m_StyleConfig.end()) {
				return &it->second;
			} else
				AddLog(Error, "There is no style config " << Name);
			throw false;
		}
	}

	Material* GetMaterial(const string &Name) {
		auto it = m_Materials.find(Name);
		if (it != m_Materials.end())
			return &it->second;
		else {
			it = m_Materials.find("Default");
			if (it != m_Materials.end()) {
				return &it->second;
			} else
				AddLog(Error, "There is no material config " << Name);
			throw false;
		}
	}
};

} // namespace SimpleMap
} // namespace Modules
} // namespace MoonGlare
