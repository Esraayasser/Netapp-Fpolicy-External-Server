#include "Fpolicy_server_manager.h"

Fpolicy_server_manager::Fpolicy_server_manager()
{
}

Fpolicy_server_manager::~Fpolicy_server_manager()
{
}

void Fpolicy_server_manager::Start_communication()
{
    while (true) {
        string notfType, header, body, notfReponse;
        int contentLen = 0;

        // Reading the incoming notification
        Notification_manager.Read_notification(notfType, contentLen, header, body);

        // Parsing the responce based on the notification type.
        notfReponse = Notification_manager.Parse_notification_response(notfType, header, body);

        if (!notfReponse.empty()) {

            cout << "Full response:\n" << notfReponse << endl;
            // Send the response to the fpolicy engine.
            Notification_manager.Send_notification(notfReponse);
        }
    }
}
