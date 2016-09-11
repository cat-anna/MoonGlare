/*
  * Generated by cppsrc.sh
  * On 2016-09-11 17:09:28,63
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef ChangesManager_H
#define ChangesManager_H

namespace MoonGlare {
namespace QtShared {

class ChangesManager;
struct iChangeContainer;

class ChangesManager : public QObject {
	Q_OBJECT;
public:
	ChangesManager();
	virtual ~ChangesManager();

	static ChangesManager* Get() { return s_Instance; }

	void SetModiffiedState(iChangeContainer *sender, bool value);

	struct State {
		bool m_Modiffied;
	};
	using StateMap = std::unordered_map < iChangeContainer*, State>;
	const StateMap& GetStateMap() { return m_State; }
	void SaveAll();

signals:
	void Changed(iChangeContainer* sender, bool state);
protected:
	StateMap m_State;
private:
	static ChangesManager *s_Instance;
};

struct iChangeContainer {
	iChangeContainer() {  }
	virtual ~iChangeContainer() { 
		SetModiffiedState(false);
	};
	virtual bool SaveChanges() { return false; }
	virtual bool CanDropChanges() const { return false; }
	virtual bool DropChanges() { return false; }
	virtual std::string GetInfoLine() const { return ""; }
	virtual std::string GetName() const = 0;

	bool IsChanged() const { return m_Changed; }
protected:
	virtual void SetModiffiedState(bool value) {
		m_Changed = value;
		assert(ChangesManager::Get() != nullptr);
		ChangesManager::Get()->SetModiffiedState(this, value);
	}
private:
	bool m_Changed;
};

} //namespace QtShared
} //namespace MoonGlare

#endif
