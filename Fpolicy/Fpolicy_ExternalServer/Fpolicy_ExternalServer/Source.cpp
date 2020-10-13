/*
    - When you start to create the external engine in netapp, make sure that your virtual machine running the netapp,
        the one running the domain windows iso and the one runnig the client windows iso, are all running on the same virtual network.
    - In the pro VMware workstation -> Edit -> Virtual Network Editor -> "if VMnet2 doesn't exist" -> change settings 
            -> add network -> select VMnet2 with configurations, Host-only.
    - Make sure the cluster, svm, subnets, LIFS, domain's vm, and client's vm are all on the vnet2 IP subnet that you get from your local device.
        Edit virtual machine settings -> Network adapter -> Custom -> chose VMnet2 -> OK -> OK.
    - Then for the firewall permissions, open Windos Defender Firewall with Advanced Security -> Inbound rules -> 
                                                                new rule and add the server exe path giving it all permissions
*/
#include "Fpolicy_server_manager.h"

int main()
{
    Fpolicy_server_manager Fpolicy_manager;
    Fpolicy_manager.Start_communication();

    return 0;
}