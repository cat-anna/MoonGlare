#pragma once
#ifndef cGUIH
#define cGUIH

#if 0

enum class eScreenPosition{
	spNone,
	spCenter,
	spLeftTop,
	spLeftBottom,
	spRightTop,
	spRightBotom, 
};

class cGUI : public DataClasses::cDataClass {
#ifdef _BUILDING_EDITOR_
friend class cGUIEditor;
#endif
//	class cGUIScriptEvents: public cScriptEvents {
//		DEFINE_SCRIPT_EVENTS(cGUIScriptEvents,
//					SCRIPT_ADD(OnInitialize)
//					SCRIPT_ADD(OnClick));
//	public:
//		string
//			OnInitialize,
//			OnClick;
//	};
protected:
/*	class cGUIAnim {

	};
	typedef std::list<cGUIAnim> cGUIAnimList;

	cGUIAnimList m_AnimList;

	cFPSEngine *m_Engine;
	cBasicFont *FFont;
//	cGUIScriptEvents m_ScriptHandlers;

//precalculated values
	c3DPoint FPSDrawPos;
	
//only loaded vales
	c3DPoint FPSTextPositioDelta;//Odstêp od wybranoego rogu
	eScreenPosition FPSTextPosition;


	int InvokeOnInitialize();
	int InvokeOnClick(int x, int y);*/
public:
	/*cGUI(cFPSEngine *Engine);
	~cGUI();

	void Draw(cEngineConsole *ECon = 0);	

	bool SaveToXML(xml_node Node) const;
	bool LoadFromXML(xml_node Node);

	void AddAnimation(const string& TexName, const c3DPoint& ScreenPos, float scale);

	bool Initialize();
	void Finalize();
	void Clear();*/
};
#endif
#endif
