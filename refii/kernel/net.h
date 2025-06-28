#pragma once

#include <os/logger.h>

namespace refii {
namespace kernel {
    void NetDll_WSAStartup();
    void NetDll_WSACleanup();
    void NetDll_socket();
    void NetDll_closesocket();
    void NetDll_setsockopt();
    void NetDll_bind();
    void NetDll_connect();
    void NetDll_listen();
    void NetDll_accept();
    void NetDll_select();
    void NetDll_recv();
    void NetDll_send();
    void NetDll_inet_addr();
    void NetDll___WSAFDIsSet();
    void NetDll_XNetStartup();
    void NetDll_XNetGetTitleXnAddr();
    void NetDll_recvfrom();
    void NetDll_sendto();
    void NetDll_WSAGetLastError();
    void NetDll_WSAGetOverlappedResult();
    void NetDll_WSARecvFrom();  
    void NetDll_WSASendTo();
    void NetDll_ioctlsocket();
    void XNetLogonGetTitleID();
    void NetDll_XNetCleanup();
    void NetDll_XNetConnect();
    void NetDll_XNetGetConnectStatus();
    void NetDll_XNetInAddrToXnAddr();
    void NetDll_XNetQosListen();
    void NetDll_XNetQosRelease();
    void NetDll_XNetRandom();
    void NetDll_XNetRegisterKey();
    void NetDll_XNetUnregisterInAddr();
    void NetDll_XNetUnregisterKey();
    void NetDll_XNetXnAddrToInAddr();
    void NetDll_XNetXnAddrToMachineId();
    void XNetLogonGetTitleID();
    void NetDll_XNetQosListen();
    void NetDll_XNetQosRelease();
}
}
