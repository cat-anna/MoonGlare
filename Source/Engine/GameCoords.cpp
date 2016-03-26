#include <pch.h>
#include <MoonGlare.h>
#include "GameCoords.h"

namespace MoonGlare {

GABI_IMPLEMENT_CLASS(GameCoords);

GameCoords::GameCoords(): cRootClass(), m_MapName() {
}

GameCoords::~GameCoords(){
}

} //namespace MoonGlare

#if 0

/*
cGameCoords::cGameCoords(const string &MapName, const c3DPoint &Position, const c3DPoint &CameraRotation):
		cRootClass(), m_MapName(MapName), m_Position(Position), m_CameraRotation(CameraRotation){
//	AddLog("Create cGameCoords 2 - " << this);	
}*/

cGameCoords::cGameCoords(const char* MapName, float pX, float pZ, float Angle):
		cRootClass(), m_MapName(MapName)/*, m_Position(pX, 0, pZ), m_CameraRotation(0, Angle, 0)*/{
//	AddLog("Create cGameCoords 3 - " << this);
}



const char *c3DPoint_cGameCoords_Captions[] = {"aX", "aY", "aZ", };

bool cGameCoords::SaveToXML(xml_node Node) const{
	LOG_NOT_IMPLEMENTED();
/*	if(!Node)return false;
	Node.append_attribute("MapName") = m_MapName.c_str();
	m_Position.SaveToXML(Node);
	m_CameraRotation.SaveToXML(Node, c3DPoint_cGameCoords_Captions);*/
	return true;
}

bool cGameCoords::LoadFromXML(const xml_node Node){
	LOG_NOT_IMPLEMENTED();
/*	m_MapName = Node.attribute("MapName").as_string();
	m_Position.LoadFromXML(Node);
	m_CameraRotation.LoadFromXML(Node, c3DPoint_cGameCoords_Captions);*/
	return Node;
}

string cGameCoords::AsString(bool Extended) const {
	std::ostringstream ss;
	LOG_NOT_IMPLEMENTED();
/*	if(Extended){
		char buf[2048] = {};
		sprintf(buf, "\"%s\",%.3f,%f,%f,%f,%f,%f", GetMapName().c_str(), GetPosition().x, GetPosition().y, GetPosition().z, 
									GetCameraRotation().x, GetCameraRotation().y, GetCameraRotation().z);
		return string(buf);
	} else
		ss << "\"" << GetMapName() << "\"," << GetPosition().x << "," << GetPosition().z << "," << GetCameraRotation().y;
*/	return ss.str();
}
#endif
