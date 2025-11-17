#ifndef OPCUADATDEVICESTATUS_H
#define OPCUADATDEVICESTATUS_H


#include "OpcuaDataBase.h"

class OpcuaDataDeviceStatus : public OpcuaDataBaseString
{
public:
	OpcuaDataDeviceStatus(UA_Server* server, int rwflag, char* nodeName) :OpcuaDataBaseString(server,rwflag,nodeName) {
		m_status = 0;
	}
	~OpcuaDataDeviceStatus() {}

	void SetStatus(const char* status) {
		if (m_status) {
			delete[] m_status;
		}
		size_t len = strlen(status) + 1;
		m_status = new char[len];
		strcpy_s(m_status, len, status);
		if (m_lastDeviceIsOnline) {
			SetValue(m_status);
		}
	}
	void SetDeviceOnline(bool isOnline) {
		if (isOnline == m_lastDeviceIsOnline) {
			return;
		}
		m_lastDeviceIsOnline = isOnline;
		if (!m_lastDeviceIsOnline) {
			SetValue("Offline");
		}
		else {
			SetValue(m_status);
		}
	}
private:
	bool m_lastDeviceIsOnline = true;
	char* m_status;
};

#endif