/*
	Generated by cppsrc.sh
	On 2015-01-22  6:51:10,16
	by Paweu
*/

#pragma once
#ifndef SoundEngine_H
#define SoundEngine_H

namespace MoonGlare {
namespace Modules {
namespace ThreadedSoundEngine {

using namespace ::MoonGlare::Sound; 

struct SoundEngineSettings;

class SoundEngine : public iSoundEngine {
	friend struct SoundEngineDebugApi;
	SPACERTTI_DECLARE_STATIC_CLASS(SoundEngine, iSoundEngine);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
 	SoundEngine(SoundEngineSettings *Settings);
 	~SoundEngine();

	virtual bool Initialize() override;
	virtual bool Finalize() override;

	void ConfigurationChanged(SettingsGroup what);

	bool IsRunning() const { return m_ThreadRunning; }

	/** Looks for both sounds and musics */
	virtual void ScanForSounds() override;

	virtual void RegisterSound(string Name, string FileName) override;
	virtual void RegisterMusic(string Name, string FileName) override;

	virtual void PlaySound(const string &Name) override;
	virtual void PlayMusic(const string &Name) override;

	virtual void PlaySoundScript(const string &Name, const string &Func, int param) override;
	virtual void PlayMusicScript(const string &Name, const string &Func, int param) override;

	virtual void StopMusic() override;

	virtual void SetPlayList(const string& PlayListName) override;
	virtual void RunPlayList() override;

	virtual void SoundNotifyEnd(iSound* s) override;
	virtual void HandleSoundEndAction(SoundEndAction action, iSound *sound) override;

	static bool GetSoundClass(const string &FileName, string &out);

	virtual void EnumerateAudio(EnumerationFunc func) override;

	template<class T>
	void PostAction(T && action) {
		m_ActionListMutex.lock();
		m_ActionList.emplace_back(std::forward<T>(action));
		m_ActionListMutex.unlock();
	}

	virtual void DumpContent(std::ostream &out) override;
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	struct SoundInfo {
		iSound *m_Sound = 0;
		string m_Class;
		string m_Name;
		string m_FileName;
		bool m_IsMusic;

		SoundInfo();
		~SoundInfo();
		bool Load(SoundEngineSettings *setting);
		void Release();
	};
	using SoundDatabase = std::unordered_map<string, SoundInfo>;
	using ActionList = std::list<std::function<void()>>;
	using PlayQueue = std::list<string>;

	struct ActionPlayItem { 
		string Name;
		SoundType Type;
		std::function<void(iSound*)> action;
	};

	volatile bool m_ThreadRunning;
	std::thread m_Thread;
	mutable std::condition_variable m_Lock;

	SoundDatabase m_Sounds, m_Music;
	PlayQueue m_SoundPlayQueue, m_MusicPlayQueue;
	std::recursive_mutex m_SoundsMutex, m_MusicMutex;
	std::list<iSound*> m_CurrentMusics; //!may only be modiffied by sound engine thread
	iSound *m_CurrentPlayListItem;
	
	ActionList m_ActionList;
	std::list <ActionPlayItem> m_ActionPlayList;
	std::recursive_mutex m_ActionListMutex;

	PlayListPtr m_PlayList;

	DefineFlagSetter(m_Flags, Flags::Ready, Ready);

	void Notify() const { m_Lock.notify_all(); }
	virtual iSound* GetSound(const string &Name) override;
	virtual iSound* GetMusic(const string &Name) override;

	struct SilentExitException {};

    SoundEngineSettings *settings;
private:
	void InternalInitialize();
	void InternalFinalize();
	void FinalizeSounds();
	void ProcessPlayList();
	void ThreadEntry();
	void ThreadMain();

	void ScanForSoundsImpl();
	void ScanForSoundsScanPath(DataPath mode, const string& basepath, const string& namepath);
};

} //namespace ThreadedSoundEngine 
} //namespace Modules 
} //namespace MoonGlare 

#endif
