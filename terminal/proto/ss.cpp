//
// (C) 2020 chiv
//
//
#include "ss.h"
#include <stdio.h>
#include <string.h>
#include <algorithm>

namespace proto {
/*-----------------------------------------------------------------------------
// SS protocal v1.0
//-----------------------------------------------------------------------------
*/
/*!
// support ss protocol
//
// <STX>[part]<US>[action]<US>[parameters,delim is US]<ETX>[chksum][EOT]
// <STH>
//      <STX>[part]<US>[action]<US>[parameters,delim is US]<ETX>[chksum][EOT]
//      <STX>[part]<US>[action]<US>[parameters,delim is US]<ETX>[chksum][EOT]
// <EOB>
//
// What is ss ? ss is simple short or simple stupid.
*/

namespace ss {

static inline unsigned char b1_count(unsigned char bits)
{
    unsigned char bc = 0;
    while (bits) {
        bits &= bits - 1;
        bc++;
    }
    return bc;
}

unsigned char ss_chksum(const char* buf, int sz)
{
    int i = 0;
    unsigned char s = 0;
    if (sz > 0) {
        for (i = 0; i < sz; ++i) {
            s ^= buf[i];
        }
    } else {
        while (buf[i] != '\0') {
            s ^= buf[i++];
        }
    }
    if (s < 0x30) s += 0x30;
    if (b1_count(s) % 2 == 0) {
        s |= s + 1;
    }
    return s;
}

ss_command_t ss_parse_command(const char* ss, int sz)
{
    if (sz < 0) sz = (int)strlen(ss);
    std::string ss_; ss_.resize(sz);
    for (int k = 0; k < sz; ++k) {
        ss_[k] = ss[k];
    }
    return ss_parse_command(ss_);
}

ss_command_t ss_parse_command(const std::string& ss)
{
    ss_command_t cmd; cmd.error = 0;
    if (ss.length() < 4 || ss[0] != ENQ || ss[1] != STX || ss[ss.length()-1] != EOT) {
        cmd.error = ss_command_t::error_format;
    } else {
        size_t p_stx = 1;
        size_t p_etx = ss.find(ETX);
        size_t p_eot = ss.length() - 1;
        if (p_etx == ss.npos) {
            cmd.error = ss_command_t::error_format;
        } else {
            if (p_eot - p_etx == 2) {
                if (ss[p_etx+1] != ss_chksum(ss.c_str()+2, p_etx-2)) {
                    cmd.error = ss_command_t::error_chksum;
                }
            } else if (p_eot - p_etx != 1) {
                cmd.error = ss_command_t::error_format;
            }
        }
        if (cmd.error == 0) { // Ok, let's go
            size_t p = ss.find(US);
            size_t q = p_stx + 1;
            if (p == ss.npos) {
                cmd.part = ss.substr(q, p_etx-1);
            } else {
                cmd.part = ss.substr(q, p-q);
                q = p+1;
                p = ss.find(US, q);
                if (p == ss.npos) {
                    cmd.action = ss.substr(q, p_etx-q);
                } else {
                    cmd.action = ss.substr(q, p-q);
                    q = p+1;
                    p = ss.find(US, q);
                    while (p != ss.npos) {
                        cmd.params.push_back(ss.substr(q, p-q));
                        q = p+1;
                        p = ss.find(US, q);
                    }
                    if (q < p_etx) {
                        cmd.params.push_back(ss.substr(q, p_etx-q));
                    }
                }
            }
        }
    }
    return cmd;
}

std::vector<ss_command_t> ss_parse(const char* ss, int sz)
{
    if (sz < 0) sz = (int)strlen(ss);
    std::string ss_; ss_.resize(sz);
    for (int k = 0; k < sz; ++k) {
        ss_[k] = ss[k];
    }
    return ss_parse(ss_);
}

std::vector<ss_command_t> ss_parse(const std::string& ss_message)
{
    std::vector<ss_command_t> out;
    if (ss_message[0] == ENQ && ss_message[ss_message.length()-1] == EOT) {
        out.push_back(ss_parse_command(ss_message));
    } else if (ss_message[0] == SOH && ss_message[ss_message.length()-1] == ETB) {
        size_t q = 1, p = ss_message.find(EOT, q);
        while (p != ss_message.npos) {
            out.push_back(ss_parse_command(ss_message.substr(q, p-q)));
            q = p+1;
            p = ss_message.find(EOT, q);
        }
        if (q < ss_message.length()-1) {
            out.push_back(ss_parse_command(ss_message.substr(q, ss_message.length()-1-q)));
        }
    }
    return out;
}

std::string ss_make_request(const ss_command_t& command)
{
    return ss_make_request(command.part, command.action, command.params);
}

std::string ss_make_request(const std::string& part, const std::string& action, const std::vector<std::string>& params)
{
    std::string out;
    out.push_back(ENQ);
    out.push_back(STX);
    out += part;
    out.push_back(US);
    out += action;
    for (size_t k = 0; k < params.size(); ++k) {
        out.push_back(US);
        out += params[k];
    }
    out.push_back(ETX);
    out.push_back((unsigned char)ss_chksum(out.c_str() + 2, (int)out.length() - 3));
    out.push_back(EOT);
    return std::move(out);
}
//
// How to make request group ?
//
// std::string = ss_make_group(ss_make_request(...) + ss_make_request(...))
//
std::string ss_make_group(const std::string& ss)
{
    std::string out;
    out.push_back(SOH);
    out += ss;
    out.push_back(ETB);
    return std::move(out);
}

std::string ss_make_response(const std::string& json)
{
    std::string out;
    out.push_back(ACK);
    out.push_back(STX);
    out += json;
    out.push_back(ETX);
    out.push_back((char)ss_chksum(json.c_str(), -1));
    out.push_back(EOT);
    return std::move(out);
}

int ss_parse_response(const char* response, int sz, std::string& json)
{
    if (sz < 0) sz = (int)strlen(response);
    if (sz > 1) {
        int p_ack = -1;
        int p_stx = -1;
        int p_etx = -1;
        int p_eot = -1;
        for (int k = 0; k < sz; ++k) {
            switch (response[k]) {
            case ACK: p_ack = (int)k; break;
            case STX: p_stx = (int)k; break;
            case ETX: p_etx = (int)k; break;
            case EOT: p_eot = (int)k; break;
            }
        }
        if (p_ack < 0 || p_eot < 0)
        {
            return -1;
        }
        if (p_stx > 0 && p_etx > 0) { // has json
            if (p_eot - p_etx == 2) { // has chksum
                if (ss_chksum(response + p_stx+1, p_etx-p_stx-1) != (unsigned char)response[p_etx+1]) {
                    return 0; // chksum error
                }
            } else if (p_eot - p_etx != 1) {
                return -1; // format error
            }
            json = "";
            for (int i = p_stx+1; i < p_etx; ++i) {
                json.push_back(response[i]);
            }
        }
        else {
            if (p_eot - p_ack != 1) return -1; // Has no json, and not <ACK><EOT>, report error.
        }
    }
    return 1;
}


} // ss

} // proto
