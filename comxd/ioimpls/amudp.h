/*!
// (c) 2020 chiv
//
// \brief UDP for audiomatrix
// \file amudp.h
// \defgroup amudp UDP Server/Client wrapper
*/
#pragma once

#include <stdint.h>
#include <stddef.h>

namespace amnet {
    /// \class AmUdp
    /// \ingroup amudp
    class AmUdp {
    public:
        AmUdp(uint32_t local_ip, uint16_t port);
        AmUdp(const char* local_ip, uint16_t port);
        AmUdp();
        virtual ~AmUdp();
        /// \brief Read data from socket
        /// \param buffer
        /// \param buffer_sz In bytes
        /// \param timeout Unit: ms
        /// \param client_ip In network order
        /// \return
        ///        - <0 Error
        ///        - =0 Timeout
        ///        - >0 Bytes read
        ///
        int Read(void* buffer, int buffer_sz, int timeout, uint32_t* src_ip = nullptr, uint16_t* src_port = nullptr);
        ///
        /// \brief write data to socket
        /// \param buffer
        /// \param buffer_sz In bytes
        /// \param target_ip In network order
        /// \param target_port Little endian
        /// \return
        ///        - <0 Error
        ///        - >0 Bytes read
        ///
        int Write(const void* buffer, int buffer_sz, uint32_t target_ip, uint16_t target_port);
		///
		int WriteToUdpReceiver(const void* buffer, int buffer_sz, uint32_t target_ip, uint16_t target_port);
        /// \brief Set broadcast
        /// \param enable
        ///        - 0 disable
        ///        - 1 enable
        /// \return
        ///        - <0 Error
        ///        - =0 Ok
        ///
        int SetBroadcast(int enable);
        ///
        /// \brief From dot sep ip to network order IP
        /// \param ip dot sep ip
        /// \return 0 for invalid ip.
        ///
        static uint32_t IpAddr(const char* ip);
        ///
        /// \brief Network order IP to dot sep ip
        /// \param ip
        /// \param buffer The result stores here, should be large enough, such as 32
        /// \param buffer_size Buffer size, in bytes.
        ///
        static const char* MakeIpHumanReadable(uint32_t ip, char* buffer, size_t buffer_size);
        //
    private:
        uintmax_t mSock; // Enough for linux or win socket id.
    };

}
