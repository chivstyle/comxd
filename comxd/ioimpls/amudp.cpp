//
// (c) 2020 chiv
//
#include "amudp.h"
#include <stdio.h>
#if defined(_WIN32)
#include <WS2tcpip.h>
#elif defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <unistd.h>
#else
#error "audiomatrix only support win and linux."
#endif
#include <stdexcept>
#include <string.h> // for memset/memcpy, .etc
#include <string>
#include <stdarg.h>
#include <errno.h>

namespace amnet {

    AmUdp::AmUdp(uint32_t local_ip, uint16_t port)
    {
        mSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
        addr.sin_addr.s_addr = local_ip;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
#if defined(_WIN32)
        if (mSock == INVALID_SOCKET) {
            if (WSAGetLastError() == WSANOTINITIALISED) {
                WSADATA wsa;
                int ret = WSAStartup(MAKEWORD(2, 2), &wsa);
                if (ret < 0 || (mSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
                    throw std::runtime_error(std::string("<AmUdp>:can't create socket, WSAGetLastError()=") + std::to_string(WSAGetLastError()));
                }
			}
			else
				throw std::runtime_error("Failed to create a socket");
        }
        if (bind((SOCKET)mSock, (const sockaddr*)&addr, sizeof(addr)))
#elif defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
        if (mSock < 0) {
            throw std::runtime_error("<AmUdp>:can't create socket, errno=" + std::to_string(errno));
        }
        if (bind((int)mSock, (const sockaddr*)&addr, sizeof(addr)))
#endif
        {
            throw std::runtime_error(std::string("<AmUdp>:can't bind to:") + std::to_string(local_ip & 0xff) + "." +
                std::to_string((local_ip >> 8) & 0xff) + "." + std::to_string((local_ip >> 16) & 0xff) + "." +
                std::to_string(local_ip >> 24) + ":" + std::to_string(port));
        }
    }
    uint32_t AmUdp::IpAddr(const char* ip)
    {
        struct in_addr addr;
        inet_pton(AF_INET, ip, &addr);
        return addr.s_addr;
    }
    const char* AmUdp::MakeIpHumanReadable(uint32_t ip, char* buffer, size_t buffer_size)
    {
        return inet_ntop(AF_INET, &ip, buffer, buffer_size);
    }

    AmUdp::AmUdp(const char* local_ip, uint16_t port)
        : AmUdp(IpAddr(local_ip), port) // C++ 11.
    {
    }
    AmUdp::AmUdp()
    {
        mSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#if defined(_WIN32)
        if (mSock == INVALID_SOCKET) {
            if (WSAGetLastError() == WSANOTINITIALISED) {
                WSADATA wsa;
                int ret = WSAStartup(MAKEWORD(2, 2), &wsa);
                if (ret < 0 || (mSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
                    throw std::runtime_error("<AmUdp>:can't create socket, WSAGetLastError()" + std::to_string(WSAGetLastError()));
                }
            }
        }
#elif defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
        if (mSock < 0) {
            throw std::runtime_error("<AmUdp>:can't create socket, errno=" + std::to_string(errno));
        }
#endif
    }
    AmUdp::~AmUdp()
    {
#if defined(_WIN32)
        closesocket((SOCKET)mSock);
#elif defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
        close((int)mSock);
#endif
    }
    //
    int AmUdp::Read(void* buffer, int buffer_sz, int timeout, uint32_t* src_ip, uint16_t* src_port)
    {
#if defined(_WIN32)
        SOCKET fd = (SOCKET)mSock;
#elif defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
        int fd = (int)mSock;
#endif
        struct timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
        fd_set fs;
        FD_ZERO(&fs);
        FD_SET(fd, &fs);
        int ret = select((int)fd + 1 /** windows will ignore this para */, &fs, nullptr, nullptr, &tv);
        if (ret == 0) return 0; else if (ret < 0) return -1;
        struct sockaddr_in addr; socklen_t len = sizeof(addr);
        ret = recvfrom(fd, (char*)buffer, (int)buffer_sz, 0, (sockaddr*)&addr, &len);
        if (src_ip) {
            *src_ip = addr.sin_addr.s_addr;
        }
        if (src_port) {
            *src_port = ntohs(addr.sin_port);
        }
        return ret;
    }
    //
    int AmUdp::Write(const void* buffer, int buffer_sz, uint32_t target_ip, uint16_t target_port)
    {
#if defined(_WIN32)
        SOCKET fd = (SOCKET)mSock;
#elif defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
        int fd = (int)mSock;
#endif
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(target_port);
        addr.sin_addr.s_addr = target_ip;
        return sendto(fd, (const char*)buffer, buffer_sz, 0, (const sockaddr*)&addr, sizeof(addr));
    }

    int AmUdp::SetBroadcast(int enable)
    {
#if defined(_WIN32)
        SOCKET fd = (SOCKET)mSock;
        ULONG optvalue = (ULONG)enable;
#elif defined(__linux__) || defined(__linux) || defined(__gnu_linux__)
        int fd = (int)mSock;
        const int& optvalue = enable;
#endif
        return setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (const char*)&optvalue, sizeof(optvalue));
    }
}
