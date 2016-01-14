/*
 * nfScene.h
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#ifndef NFSCENE_H_
#define NFSCENE_H_

namespace Core {
	namespace Scene {
		class ModelInstance;
		class ModelInstanceManager;

		class GameScene;
		class ciScene;

		class ScenesManager;

		struct ModelInstanceDeleter {
			void operator()(ModelInstance*);
		};
		using ModelInstancePtr = std::unique_ptr<ModelInstance, ModelInstanceDeleter>;

		using SceneClassRegister = GabiLib::DynamicClassRegister < ciScene > ;
	} // namespace Scene
	using Scene::GameScene;
	using Scene::ciScene;

} // namespace Core

#endif // SCENE_H_ 
