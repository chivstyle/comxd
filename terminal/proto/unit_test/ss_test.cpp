//
// (c) 2020 chiv
//
#include "proto/ss.h"
#include <stdio.h>
#include <string.h>

using namespace proto;

void dump(const std::vector<ss::ss_command_t>& cmds)
{
    for (size_t k = 0; k < 1; ++k) {
        switch (cmds[k].error) {
        case ss::ss_command_t::error_chksum:
            printf("checksum error\n");
            break;
        case ss::ss_command_t::error_format:
            printf("format error\n");
            break;
        default:
            printf("Part:%s\n", cmds[k].part.c_str());
            printf("Action:%s\n", cmds[k].action.c_str());
            for (size_t i = 0; i < cmds[k].params.size(); ++i) {
                printf("Ps%u:%s\n", i, cmds[k].params[i].c_str());
            }
            break;
        }
    }
}

void test_command()
{
    std::string ss1 = "\x05\x02part\x1f"
                      "action\x1fp1\x1fp2\x1fp3\x03\x04";
    std::string ss2 = "\x05\x02RequestID\x03\xcc\x04";
    std::string ss3 = "\x05\x02usb16x1\x1f\x1f"
                      "5\x03\x04";
    std::string ss4 = "\x06\x02{\"Status\":\"Ok\"}\x03\x04";

    printf("normal ss message with 3 parameters\n");
    dump(ss::ss_parse(ss1));
    printf("only part\n");
    dump(ss::ss_parse(ss2));
    printf("empty action\n");
    dump(ss::ss_parse(ss3));
    //
    printf("parse response\n");
    std::string json;
    int ret = ss::ss_parse_response(ss4.c_str(), -1, json);
    if (ret == 0)
        printf("chksum error\n");
    else if (ret < 0) {
        printf("format error\n");
    } else {
        printf("ok, json:%s\n", json.c_str());
    }
}

int main()
{
    test_command();

    return 0;
}