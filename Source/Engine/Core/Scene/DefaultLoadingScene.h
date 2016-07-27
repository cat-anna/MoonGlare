/*
 * cLoadingScene.h
 *
 *  Created on: 17-12-2013
 *      Author: Paweu
 */

#ifndef CLOADINGSCENE_H_
#define CLOADINGSCENE_H_

namespace MoonGlare {
namespace Core {
namespace Scene {

class DefaultLoadingScene : public LoadingBaseScene {
	SPACERTTI_DECLARE_CLASS(DefaultLoadingScene, LoadingBaseScene)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	DefaultLoadingScene();
	virtual ~DefaultLoadingScene();

	virtual int InvokeOnTimer(int TimerID) override;
	virtual int InvokeOnEscape() override;
	virtual int InvokeOnBeginScene() override;
	virtual int InvokeOnEndScene() override;
	virtual int InvokeOnInitialize() override;
	virtual int InvokeOnFinalize() override;

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	virtual bool DoInitialize() override;
 	virtual bool DoFinalize() override;

	void SetInfoText(const string &TableEntryName);
	bool SetDefaultLayout();
	static const char* InfoLabelName() { return "InfoLabel"; }
};

} //namespace Scene 
} //namespace Core 
} //namespace MoonGlare 

#endif // DefaultLoadingScene_H
