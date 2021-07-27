//
// (c) 2020 chiv
//
#include "ProtoSs.h"
#include "Conn.h"
#include "ProtoFactory.h"
#include "ss.h"
#include "terminal_rc.h"

namespace proto {

REGISTER_PROTO_INSTANCE("Simple Stupid", ProtoSs);

ProtoSs::ProtoSs(SerialConn* conn)
    : Proto(conn)
{
    WhenUsrBar = [=](Bar& bar) {
        bar.Add(t_("Help"), terminal::help(), [=]() {
               PromptOK(Upp::DeQtf(t_("Input your command in JSON, here is an example:\n"
                                      "{\n    \"Part\":\"LED2\",\n    \"Action\":\"Blink\", \n    \"Parameters\":[\"10\"]\n}")));
           })
            .Help(t_("Popup help dialog"));
    };
}

ProtoSs::~ProtoSs()
{
}

std::string ProtoSs::GetDescription() const
{
    return t_("This proto was designed by chivstyle acording to KISS principal");
}
//
static inline std::vector<unsigned char> MakeCommand(const String& part,
    const String& action, const Value& params)
{
    std::vector<unsigned char> command;
    command.push_back(ss::ENQ);
    command.push_back(ss::STX);
    // No check
    for (int i = 0; i < part.GetLength(); ++i) {
        command.push_back(part[i]);
    }
    command.push_back(ss::US);
    for (int i = 0; i < action.GetLength(); ++i) {
        command.push_back(action[i]);
    }
    if (params.GetType() == VALUEARRAY_V) {
        int count = params.GetCount();
        if (count > 0)
            command.push_back(ss::US);
        for (int n = 0; n < count; ++n) {
            String pn = params[n].ToString();
            for (int i = 0; i < pn.GetLength(); ++i) {
                command.push_back(pn[i]);
            }
            if (n != count - 1) {
                command.push_back(ss::US);
            }
        }
    } else {
        String p0 = params.ToString();
        for (int i = 0; i < p0.GetLength(); ++i) {
            command.push_back(p0[i]);
        }
    }
    command.push_back(ss::ETX);
    // calculate check sum of contents between STX and ETX
    command.push_back(ss::ss_chksum((const char*)command.data() + 2, (int)command.size() - 3));
    command.push_back(ss::EOT);
    //
    return std::move(command);
}
//
static inline void operator+=(std::vector<unsigned char>& out, const std::vector<unsigned char>& in)
{
    for (size_t k = 0; k < in.size(); ++k)
        out.push_back(in[k]);
}
// use this proto to pack the data
std::vector<unsigned char> ProtoSs::Pack(const void* input, size_t input_size, std::string& errmsg)
{
    std::vector<unsigned char> out;
    std::string json_((const char*)input, input_size);
    Value json = ParseJSON(json_.c_str());
    if (json.IsError()) {
        errmsg = t_("Your input is not a valid JSON string");
    } else {
        if (json.GetType() == VALUEMAP_V) {
            if (json["Part"].IsError()) {
                // No part, parse as response.
                out.push_back(ss::ACK);
                out.push_back(ss::STX);
                out.insert(out.end(), json_.begin(), json_.end());
                out.push_back(ss::ETX);
                out.push_back(ss::ss_chksum(json_.c_str(), (int)json_.length()));
                out.push_back(ss::EOT);
                errmsg = t_("There's no key \"Part\", pack as response.");
            } else {
                String part = json["Part"].ToString();
                String action = json["Action"].ToString();
                const Value& params = json["Parameters"];
                out = MakeCommand(part, action, params);
            }
        } else if (json.GetType() == VALUEARRAY_V) {
            int count = json.GetCount();
            out.push_back(ss::SOH);
            for (int i = 0; i < count; ++i) {
                const Value& command = json[i];
                if (command.GetType() != VALUEMAP_V) {
                    errmsg = t_("Please pass a map-array");
                    break;
                }
                String part = command["Part"].ToString();
                String action = command["Action"].ToString();
                const Value& params = command["Parameters"];
                out += MakeCommand(part, action, params);
            }
            out.push_back(ss::ETB);
        } else {
            // response ?
            out.push_back(ss::ACK);
            out.push_back(ss::STX);
            out.insert(out.end(), json_.begin(), json_.end());
            out.push_back(ss::ETX);
            out.push_back(ss::ss_chksum(json_.c_str(), (int)json_.length()));
            out.push_back(ss::EOT);
        }
    }
    return out;
}

int ProtoSs::Transmit(const void* input, size_t input_size, std::string& errmsg)
{
    auto out = Pack(input, input_size, errmsg);
    if (out.empty())
        return T_FAILED;
    return mConn->GetIo()->Write(out);
}

}
