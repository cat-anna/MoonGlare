#ifndef GameCoordsH
#define GameCoordsH

#if 0
namespace MoonGlare {

class GameCoords : public cRootClass {
	SPACERTTI_DECLARE_CLASS(GameCoords, cRootClass);
public:	
	GameCoords();
	virtual ~GameCoords(); 

	//DefineFULLAcces(MapName, string);
	//DefineFULLAcces(Position, c3DPoint);
	//DefineFULLAcces(CameraRotation, c3DPoint);

	//bool SaveToXML(xml_node Node) const;
	//bool LoadFromXML(const xml_node Node);

	string AsString(bool Extended = false) const;
private:
	string m_MapName;
	//c3DPoint m_Position;
	//c3DPoint m_CameraRotation;
};

} //namespace MoonGlare

#endif

#endif
