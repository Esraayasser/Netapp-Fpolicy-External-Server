#pragma once
#include "Notification.h"

class Fpolicy_server_manager
{

public:
	Fpolicy_server_manager();
	~Fpolicy_server_manager();
	void Start_communication();

private:
	Notification Notification_manager;

};

