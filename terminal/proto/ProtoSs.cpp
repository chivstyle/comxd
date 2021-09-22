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
static inline std::string MakeCommand(const String& part,
    const String& action, const Value& params)
{
    std::vector<std::string> ps;
    if (params.GetType() == VALUEARRAY_V) {
        int count = params.GetCount();
        for (int n = 0; n < count; ++n) {
            String pn = params[n].ToString();
            ps.push_back(pn.ToStd());
        }
    } else {
        String p0 = params.ToString();
        ps.push_back(p0.ToStd());
    }
    return ss::ss_make_request(part.ToStd(), action.ToStd(), ps);
}
//
static inline void operator+=(std::vector<unsigned char>& out, const std::vector<unsigned char>& in)
{
    for (size_t k = 0; k < in.size(); ++k)
        out.push_back(in[k]);
}
// use this proto to pack the data
std::string _Pack(const void* input, size_t input_size, std::string& errmsg)
{
    std::string out;
    std::string json_((const char*)input, input_size);
    Value json = ParseJSON(json_.c_str());
    if (json.IsError()) {
        errmsg = t_("Your input is not a valid JSON string");
    } else {
        if (json.GetType() == VALUEMAP_V) {
            if (json["Part"].IsError()) {
                errmsg = t_("There's no key \"Part\", pack as response.");
            } else {
                String part = json["Part"].ToString();
                String action = json["Action"].ToString();
                const Value& params = json["Parameters"];
                out = MakeCommand(part, action, params);
            }
        } else if (json.GetType() == VALUEARRAY_V) {
            int count = json.GetCount();
            std::string reqs;
            for (int i = 0; i < count; ++i) {
                const Value& command = json[i];
                if (command.GetType() != VALUEMAP_V) {
                    errmsg = t_("Please pass a map-array");
                    break;
                }
                String part = command["Part"].ToString();
                String action = command["Action"].ToString();
                const Value& params = command["Parameters"];
                reqs += MakeCommand(part, action, params);
            }
            out = ss::ss_make_group(reqs);
        }
    }
    return out;
}

int ProtoSs::Transmit(const void* input, size_t input_size, std::string& errmsg)
{
    auto out = _Pack(input, input_size, errmsg);
    if (out.empty())
        return T_FAILED;
    return (int)mConn->GetIo()->Write(out);
}

}
