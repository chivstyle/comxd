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
    mUsrActions.emplace_back(terminal::help(), t_("Help"), t_("Popup help dialog"), [=]() {
        PromptOK(Upp::DeQtf(t_("Input your command in JSON, here is an example:\n"
            "{\n    \"Part\":\"LED2\",\n    \"Action\":\"Blink\", \n    \"Parameters\":[\"10\"]\n}")));
    });
}

ProtoSs::~ProtoSs()
{
}

std::string ProtoSs::GetName() const
{
    return "Simple Stupid";
}

std::string ProtoSs::GetDescription() const
{
    return t_("This proto was designed by chivstyle acording to KISS principal");
}
// use this proto to pack the data
std::vector<unsigned char> ProtoSs::Pack(const unsigned char* buf, size_t sz, std::string& errmsg)
{
    std::vector<unsigned char> out;
    std::string json_;json_.resize(sz);
    for (size_t k = 0; k < sz; ++k) {
        json_[k] = (char)buf[k];
    }
    Value json = ParseJSON(json_.c_str());
    if (json.IsError()) {
        errmsg = t_("Your input is not a valid JSON string");
    } else {
        if (json.GetType() != VALUEMAP_V) {
            errmsg = t_("ROOT should be Object");
        } else {
            out.push_back(ss::ENQ);
            out.push_back(ss::STX);
            String part = json["Part"].ToString();
            String action = json["Action"].ToString();
            const Value& params = json["Parameters"];
            // No check
            for (int i = 0; i < part.GetLength(); ++i) {
                out.push_back(part[i]);
            }
            out.push_back(ss::US);
            for (int i = 0; i < action.GetLength(); ++i) {
                out.push_back(action[i]);
            }
            out.push_back(ss::US);
            if (params.GetType() == VALUEARRAY_V) {
                int count = params.GetCount();
                for (int n = 0; n < count; ++n) {
                    String pn = params[n].ToString();
                    for (int i = 0; i < pn.GetLength(); ++i) {
                        out.push_back(pn[i]);
                    }
                }
            } else {
                String p0 = params.ToString();
                for (int i = 0; i < p0.GetLength(); ++i) {
                    out.push_back(p0[i]);
                }
            }
            out.push_back(ss::ETX);
            out.push_back(ss::ss_chksum((const char*)out.data() + 2, out.size()-3));
            out.push_back(ss::EOT);
        }
    }
    return out;
}
// return  0 Absolutely not
//         1 Pending
//        >1 Yes
int ProtoSs::IsProto(const unsigned char* buf, size_t sz)
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
// JSON
static inline std::string GenerateReport(const ss::ss_command_t& rslt)
{
    std::string report = "\{\n";
    report += "    \"Part\":\"" + rslt.part + "\",\n";
    report += "    \"Action\":\"" + rslt.action + "\",\n";
    report += "    \"Parameters\":[\n";
    for (size_t k = 0; k < rslt.params.size(); ++k) {
        report += "        \"" + rslt.params[k] + "\",\n";
    }
    report += "    ],\n    \"Error\":";
    switch (rslt.error) {
    case ss::ss_command_t::error_none:   report += "\"No error\"\n"; break;
    case ss::ss_command_t::error_format: report += "\"format\"\n"; break;
    case ss::ss_command_t::error_chksum: report += "\"chksum\"\n"; break;
    default: report += "\"Unkown\""; break;
    }
    report += "}"; // End of JSON Object
    
    return report;
}
// parse the proto message, generate report.
std::string ProtoSs::Parse(const unsigned char* buf, size_t sz)
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
        else return "<ACK>" + json;
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
