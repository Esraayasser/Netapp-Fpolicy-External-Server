#pragma once
#include "Socket_manager.h"
#include "tinyxml2.h"

class Notification
{
public:
	Notification();
	~Notification();
	void Read_notification(string& notfType, int& contentLen, string& header, string& body);
	void Send_notification(string response);
	string Parse_notification_response(string notfType, string notfHeader, string notfBody);

private:
	Socket_manager Socket_manager;
	//char* header, * body;
	int headerLen, bodyLen;

	string Get_header(string notfReq, int& headerLen);
	string Extract_tag_value(string notification, string tag);
	string Parse_response(string notfType, string negoHeader, string negoBody);
	string Parse_NegoResp_body(string negoBody);
	string Parse_ScreenResp_body(string negoBody);
	string Parse_header(string negoHeader, int contentLen);
	string Read_XmlFile(string title);
	char* Response_header_type(const char* notfType);
	void Format_header_length(char* headerLenBuffer, int notfLen);
};

