#pragma once
#include "Global_Header.h"
#include "gettimeofday.h"

class Socket_manager
{
public:
    Socket_manager();
    ~Socket_manager();
    void Read_msg(char* msgBuffer, int msg_len);
    void Send_msg(char* responseBuffer, int response_len);

private:
    SOCKET listening_sock, sending_sock;
    gettimeofday_Class timer;

    bool Manage_connection();

};

