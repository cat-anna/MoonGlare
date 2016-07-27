/*
 * nfScene.h
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#ifndef NFSCENE_H_
#define NFSCENE_H_

namespace MoonGlare {
namespace Core {
	namespace Scene {
		class ModelInstance;

		class GameScene;
		class ciScene;

		class ScenesManager;

		using SceneClassRegister = Space::DynamicClassRegister < ciScene > ;
	} // namespace Scene
	using Scene::GameScene;
	using Scene::ciScene;

} // namespace Core
} //namespace MoonGlare 

#endif // SCENE_H_ 
