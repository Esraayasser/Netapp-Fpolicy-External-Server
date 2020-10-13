#include "Notification.h"

using namespace tinyxml2;

Notification::Notification(){
}

Notification::~Notification()
{
}


void Notification::Read_notification(string& notfType, int& contentLen, string& header, string& body)
{
	// Get the length of the full xml file.
	char* notfBuffer = new char[HEADER_LEN];
	int notfLen, headerLen, bodyLen;
	string notfReq;

	Socket_manager.Read_msg(notfBuffer, HEADER_LEN);
	memcpy(&notfLen, notfBuffer + 1, 4);
	notfLen = ntohl(notfLen);

	cout << "The notification Xml file's length is:\n" << notfLen << endl;

	// Read the etire notification message.
	notfBuffer = new char[notfLen];
	Socket_manager.Read_msg(notfBuffer, notfLen);
	notfReq = string(notfBuffer);

	cout << "The full xml file is:\n" << notfReq << endl << endl;

	if (notfReq.size() > 2) {
		// Get the header of the XML file.
		header = Get_header(notfReq, headerLen);

		// Getting the Notification type from the header.
		notfType = Extract_tag_value(header, "<NotfType>");

		// Getting the content length from the header.
		stringstream ss(Extract_tag_value(header, "<ContentLen>"));
		ss >> contentLen;

		// getting the notification body.
		bodyLen = notfReq.size() - (headerLen + 2);
		body = notfReq.substr(headerLen + 2, bodyLen);
		//cout << "The body is:\n" << body << endl << endl;
	}
	else
		contentLen = 0;
	return;
}

void Notification::Send_notification(string response)
{
	char* responseBuffer = new char[response.size()];
	memcpy(responseBuffer, response.c_str(), response.length());
	Socket_manager.Send_msg(responseBuffer, response.length());

	return;
}

string Notification::Parse_notification_response(string notfType, string notfHeader, string notfBody)
{
	string notfReponse;

	if (notfType == "NEGO_REQ") {
		notfReponse = Parse_response(notfType, notfHeader, notfBody);
	}
	else if (notfType == "ALERT_MSG") {
		notfReponse = "Handshake, Failed with fatal error!";
	}
	else if (notfType == "SCREEN_REQ") {
		cout << "A file notification came with the following characteristics:\n";
		cout << "Operation type: " << Extract_tag_value(notfBody, "ReqType") << endl;
		cout << "Client IP: " << Extract_tag_value(notfBody, "ClientIp") << endl;
		cout << "File path: " << Extract_tag_value(notfBody, "DisplayPath") << endl;

		notfReponse = Parse_response(notfType, notfHeader, notfBody);
	}
	else if (notfType == "STATUS_QUERY_REQ") {
		notfReponse = "Did the query get stuck?";
	}
	else if (notfType == "SCREEN_CANCEL") {
		notfReponse = "Cancel the request handling!!";
	}
	
	return notfReponse;
}

string Notification::Get_header(string notfReq, int& headerLen)
{
	string header;
	int header_len;

	int en_ind = notfReq.find('\n');
	headerLen = en_ind;
	header = notfReq.substr(0, headerLen);

	//cout << "\nThe header is:\n" << header << endl << endl;

	return header;
}

string Notification::Extract_tag_value(string notification, string tag)
{
	int st_ind, end_ind, value_size;
	string value;

	// Getting the index on which the desiered tag begins.
	st_ind = notification.find(tag);
	// The start index of the desired tag value is obtained by adding the tag size to st_ind.
	st_ind += tag.size();

	// Getting the index on which the ending tag for the desiered tag begins.
	end_ind = find(notification.begin() + st_ind, notification.end(), '<') - notification.begin();

	// Getting the actual value using substring.
	value_size = end_ind - st_ind;
	value = notification.substr(st_ind, value_size);

	return value;
}


string Notification::Parse_response(string notfType, string negoHeader, string negoBody)
{
	// First, parse the response body to facilitate populating the headers' conentLen tag.
	string respBody;
	if (notfType == "NEGO_REQ")
		respBody = Parse_NegoResp_body(negoBody);
	else if (notfType == "SCREEN_REQ")
		respBody = Parse_ScreenResp_body(negoBody);

	cout << "The parsed response body is:\n" << respBody << endl << endl;
	
	// Second, parse the reponse header. the msg length includes the null character at the end of the message.
	string respHeader = Parse_header(negoHeader, respBody.size() + 1);
	cout << "The parsed response header is:\n" << respHeader << endl << endl;

	// The header and the body are seperated with a doble endl chaaracters, and the full message ends with a null character.
	string negoResp = respHeader + "\n\n" + respBody + "\x00";

	// Third format the length of the response in the header.
	unsigned long notfLen = (unsigned long)negoResp.size();
	// Call the Format_header_length to convert the integer length into a byte length.
	char headerLenBuffer[HEADER_LEN] = { 0 };
	Format_header_length(headerLenBuffer, notfLen);
	// Convert the output char array buffer to a string without affecting the byte vlaues.
	string	FullBuff(headerLenBuffer, headerLenBuffer + (sizeof(headerLenBuffer) / sizeof(headerLenBuffer[0])));
	// Append the response to the byte length in the buffer string.
	FullBuff.append(negoResp);

	return FullBuff;
}

char* Notification::Response_header_type(const char* notfType)
{
	if (strcmp(notfType, "NEGO_REQ") == 0)
	{
		return (char*)"NEGO_RESP";
	}
	if (strcmp(notfType, "SCREEN_REQ") == 0)
	{
		return (char*)"SCREEN_RESP";
	}
	return (char*)notfType;
}

void Notification::Format_header_length(char* headerLenBuffer, int notfLen)
{
	headerLenBuffer[0] = '\"';
	headerLenBuffer[1] = (unsigned char)((notfLen & 0xFF000000) >> 24);
	headerLenBuffer[2] = (unsigned char)((notfLen & 0x00FF0000) >> 16);
	headerLenBuffer[3] = (unsigned char)((notfLen & 0x0000FF00) >> 8);
	headerLenBuffer[4] = (unsigned char)((notfLen & 0X000000FF));
	headerLenBuffer[5] = '\"';


	return;
}

string Notification::Parse_header(string negoHeader, int contentLen)
{
	tinyxml2::XMLDocument xmlDoc;
	// We parse the notification's header as xml file to use as a template for the responses' header.
	XMLError error = xmlDoc.Parse(negoHeader.c_str());
	// The root is the first child/tag in the file.
	XMLNode* root = xmlDoc.FirstChild();
	if (root == nullptr)
	{
		cout << "couldn't parse root " << endl;
		return "root error!";
	}
	try {
		// Get the Header element.
		XMLElement* notfResp = root->NextSiblingElement("Header");
		// get the header's first child NotfType,
		notfResp = notfResp->FirstChildElement("NotfType");
		// change it's value to that this is the response to the notification.
		const char* notfType = notfResp->GetText();
		notfResp->SetText(Response_header_type(notfType));
		// set the ContentLen tag with the len of the previously parsed body.
		notfResp = notfResp->NextSiblingElement("ContentLen");
		notfResp->SetText(contentLen);

		// I'll save the xml file and re-read it to easily parse it back to a string.
		xmlDoc.SaveFile("test_RespHeader.xml");
		return Read_XmlFile("test_RespHeader.xml");
	}
	catch (...)
	{
		cout << "parsing Exception occur." << endl;
		return "parcing error!";
	}
}

string Notification::Parse_NegoResp_body(string negoBody)
{
	tinyxml2::XMLDocument xmlDoc;
	// We parse the  handshake request body as xml file to use as a template for the handshake responses' body.
	XMLError error = xmlDoc.Parse(negoBody.c_str());
	// The root is the dirst child/tag in the file.
	XMLNode* root = xmlDoc.FirstChild();
	if (root == nullptr)
	{
		cout << "couldn't parse root " << endl;
		return "root error!";
	}
	try {
		// Get the handshake element,
		XMLElement* notfResp = root->NextSiblingElement("Handshake");
		// change it's name to "HandshakeResp" to signify that this is the response to the handshake request.
		notfResp->SetName("HandshakeResp");
		// We'll skip the followin tags, VsUUID, PolicyName, SessionId, as they do not need any altering.
		notfResp = notfResp->FirstChildElement("VsUUID");
		notfResp = notfResp->NextSiblingElement("PolicyName");
		notfResp = notfResp->NextSiblingElement("SessionId");
		// As for the port version tage, we'll replace it's children with the chosen port value.
		notfResp = notfResp->NextSiblingElement("ProtVersion");
		notfResp->DeleteChildren();
		notfResp->SetText("1.0");

		// I'll save the xml file and re-read it to easily parse it back to a string.
		xmlDoc.SaveFile("test_RespBody.xml");
		return Read_XmlFile("test_RespBody.xml");
	}
	catch (...)
	{
		cout << "parsing Exception occur." << endl;
		return "parcing error!";
	}
}

string Notification::Parse_ScreenResp_body(string negoBody)
{
	tinyxml2::XMLDocument xmlDoc;
	// We parse the  handshake request body as xml file to use as a template for the handshake responses' body.
	XMLError error = xmlDoc.Parse(negoBody.c_str());
	xmlDoc.SaveFile("test_RespBody.xml");
	// The root is the dirst child/tag in the file.
	XMLNode* root = xmlDoc.FirstChild();
	if (root == nullptr)
	{
		cout << "couldn't parse root " << endl;
		return "root error!";
	}
	try {

		// Get the FscreenReq element,
		XMLElement* notfResp = root->NextSiblingElement("FscreenReq");
		// change it's name to "FscreenResp" to signify that this is the response to the file screen request.
		notfResp->SetName("FscreenResp");
		// We'll skip the followin tags, ReqID, ReqType, as they do not need any altering.
		// Get the NotfInfo element,
		notfResp = notfResp->LastChildElement("NotfInfo");
		// change it's name to "NotfResp" to signify that this is the response to the request.
		notfResp->SetName("NotfResp");
		// Then we'll replace it's children with 1 to indicate that the request is denied.
		notfResp->DeleteChildren();
		notfResp->SetText(2);

		// I'll save the xml file and re-read it to easily parse it back to a string.
		xmlDoc.SaveFile("test_RespBody.xml");
		return Read_XmlFile("test_RespBody.xml");
	}
	catch (...)
	{
		cout << "parsing Exception occur." << endl;
		return "parcing error!";
	}
}

string Notification::Read_XmlFile(string title)
{
	ifstream inFile;
	inFile.open(title);
	if (!inFile)
	{
		cerr << "Unable to open file datafile.txt";
		return "";
	}
	string text = "";
	string line;
	while (std::getline(inFile, line)) // for each line read from the file
	{
		text += line;
	}
	string filtered = "";
	int haveopen = 0;
	for (int i = 0; i < text.size(); i++)
	{
		if (haveopen == 0 && (text[i] == ' ' || text[i] == '\n'))
			continue;
		else if (text[i] == '<')haveopen = 1;
		else if (text[i] == '>')haveopen = 0;
		filtered.push_back(text[i]);
	}
	//cout << filtered << endl << filtered.size() << endl;
	return filtered;
}

