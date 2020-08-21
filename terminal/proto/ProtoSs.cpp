//
// (c) 2020 chiv
//
#include "terminal_rc.h"
#include "ProtoSs.h"
#include "ss.h"
#include "ProtoFactory.h"

namespace proto {
    
REGISTER_PROTO_INSTANCE("Simple Stupid", ProtoSs);

ProtoSs::ProtoSs()
{
}

ProtoSs::~ProtoSs()
{
}

std::string ProtoSs::GetName() const
{
    return "(ss) Simple Stupid";
}

std::string ProtoSs::GetDescription() const
{
    return t_("This proto was designed by chivstyle acording to KISS principal");
}
// use this proto to pack the data
std::vector<unsigned char> ProtoSs::Pack(unsigned char* buf, size_t sz)
{
    std::vector<unsigned char> out(sz+4);
    out[0] = ss::ENQ;
    memcpy(out.data() + 1, buf, sz);
    out[sz+1] = ss::ETX;
    out[sz+2] = ss::ss_chksum((const char*)out.data(), out.size());
    out[sz+3] = ss::EOT;
    return std::move(out);
}
// return  0 Absolutely not
//         1 Pending
//        >1 Yes
int ProtoSs::IsProto(unsigned char* buf, size_t sz)
{
    if (sz > 0) {
        if (buf[0] == ss::ENQ) {
            if (buf[sz-1] == ss::EOT) return 2;
            else return 1;
        } else if (buf[0] == ss::SOH) {
            if (buf[sz-1] == ss::ETB) return 2;
            else return 1;
        } else if (buf[0] == ss::ACK) {
            if (buf[sz-1] == ss::EOT) return 2;
            else return 1;
        }
    }
    return 0;
}
//
static inline std::string GenerateReport(const ss::ss_command_t& rslt)
{
    std::string report = "Enq:\n";
    report += "\t" + rslt.part + "\n";
    report += "\t" + rslt.action + "\n";
    for (size_t k = 0; k < rslt.params.size(); ++k) {
        report += "\t" + rslt.params[k] + "\n";
    }
    switch (rslt.error) {
    case ss::ss_command_t::error_format: report += "Error: format\n"; break;
    case ss::ss_command_t::error_chksum: report += "Error: chksum\n"; break;
    }
    return report;
}
// parse the proto message, generate report.
std::string ProtoSs::Parse(unsigned char* buf, size_t sz)
{
    std::string report = "<Not recognized ss message>\n";
    if (IsProto(buf, sz) < 2) {
        return report;
    }
    if (buf[0] == ss::ACK) { // ACK
        std::string json;
        int ret = ss::ss_parse_response((const char*)buf, (int)sz, json);
        if (ret < 0) return "Ack:format error\n";
        else if (ret == 0) return "Ack:chksum error\n";
        else return std::string("Ack:") + json + "\n";
    } else if (buf[0] == ss::ENQ) {
        report = GenerateReport(ss::ss_parse_command((const char*)buf, (int)sz));
    } else if (buf[0] == ss::SOH) {
        auto eqns = ss::ss_parse((const char*)buf, (int)sz);
        for (size_t k = 0; k < eqns.size(); ++k) {
            report += GenerateReport(eqns[k]);
        }
    }
    return std::move(report);
}
    
}
