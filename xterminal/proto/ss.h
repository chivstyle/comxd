/*!
// (c) 2020 chiv
//
*/
#ifndef _proto_ss_h
#define _proto_ss_h

#include <string>
#include <vector>

namespace proto {
namespace ss {
    unsigned char ss_chksum(const char* buf, int sz);

    // we have a ss implementation in C, but it's designed for MCU, it's not safe for multi-thread,
    // high-performance PC. So we implement a safe version in C++.

    typedef struct ss_command_ {
        std::string part;
        std::string action;
        std::vector<std::string> params;
        enum ss_error_type {
            error_none, // No error
            error_chksum, // check sum error
            error_format, // format error
            error_count
        };
        int error;
    } ss_command_t;

    ss_command_t ss_parse_command(const std::string& ss);
    ss_command_t ss_parse_command(const char* ss, int sz);
    std::vector<ss_command_t> ss_parse(const std::string& ss_message);
    std::vector<ss_command_t> ss_parse(const char* ss_message, int sz);

    std::string ss_make_request(const ss_command_t& command);
    std::string ss_make_request(const std::string& part, const std::string& action, const std::vector<std::string>& params);
    //
    // How to make request group ?
    //
    // std::string = ss_make_group(ss_make_request(...) + ss_make_request(...))
    //
    std::string ss_make_group(const std::string& ss);

    // How to make response ?
    //
    // ss_make_response("{\"Status\":\"Ok\"}");
    //
    std::string ss_make_response(const std::string& json);
    // return -1 formnat error
    //         0 chksum error
    //        >0 ok
    int ss_parse_response(const char* response, int sz, std::string& json);

}
}

#endif
