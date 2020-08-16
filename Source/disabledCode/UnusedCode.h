/*
 * UnusedCode.h
 *
 * Unused core is stored here. Code which is not needed any more,
 * but i don't want it to be deleted.
 *
 *  Created on: 12-01-2014
 *      Author: Paweu
 */

#if 0

void cObject::TurnAt(const c3DPoint &Point){
	float hW = m_Size.x / 2.0f;
	float Angle = GabiLib::D2Math::pi + GabiLib::D2Math::cm2DLine(m_Position, Point).AngleWithOX();
	if(Point.x > m_Position.x) Angle += GabiLib::D2Math::pi;
	m_Rotation.y = GabiLib::D2Math::RadToDeg(Angle);
//	return;
	c3DPoint p0(0.0f);
	Angle = 0;
	GabiLib::D2Math::PointOnCircle(p0, Angle, hW, m_DrawPosition.LeftTop);
	m_DrawPosition.LeftBottom.x = m_DrawPosition.LeftTop.x;
	m_DrawPosition.LeftBottom.z = m_DrawPosition.LeftTop.z;
	GabiLib::D2Math::PointOnCircle(p0, Angle + GabiLib::D2Math::pi, hW, m_DrawPosition.RightTop);
	m_DrawPosition.RightBottom.x = m_DrawPosition.RightTop.x;
	m_DrawPosition.RightBottom.z = m_DrawPosition.RightTop.z;

}

#endif 
