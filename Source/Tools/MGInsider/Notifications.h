#ifndef Notifications_H
#define Notifications_H

class Notifications : public QObject {
	Q_OBJECT;
public:
	virtual ~Notifications();
	static Notifications* Get();


	static void SendSetStateValue(const std::string& ID, const std::string &Value, const std::string& Icon = "") { Get()->SetStateValue(ID, Value, Icon); }
	static void SendRemoveStateValue(const std::string& ID) { Get()->RemoveStateValue(ID); }

	static void SendRefreshView() { Get()->RefreshView(); }

	static void SendOnEngineDisconnected() { Get()->OnEngineDisconnected(); }
	static void SendOnEngineConnected() { Get()->OnEngineConnected(); }

signals:
	void SetStateValue(const std::string& ID, const std::string &Value, const std::string& Icon = "");
	void RemoveStateValue(const std::string& ID);

	void RefreshView();
	
	void OnEngineDisconnected();
	void OnEngineConnected();
protected:
	Notifications();
};

#endif // SUBWINDOW_H
