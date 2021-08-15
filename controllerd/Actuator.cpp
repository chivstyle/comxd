//
// (c) 2021 chiv
//
#include "Actuator.h"

using namespace Upp;

#define TAG "<Actuator>:"

Actuator::Actuator(const Value& conf)
{
    mWs.Connect(conf["URL"].ToString());
    //
    LOG(TAG << conf["URL"]);
}

Actuator::Actuator(const String& URL)
{
    mWs.Connect(URL);
}

Actuator::~Actuator()
{
    mWs.Close();
}

bool Actuator::SendText(const String& message)
{
    return mWs.SendText(message, true);
}

void Actuator::Run(volatile bool* should_exit)
{
    while (*should_exit == false) {
        SocketWaitEvent sel; // select indeed
        mWs.AddTo(sel);
        if (sel.Wait(100)) {
            if (sel[0]) {
                String message = mWs.Receive();
                if (!message.IsVoid())
                    WhenMessage(mWs, message);
            }
        }
        if (mWs.IsClosed()) {
            LOG(TAG << "WebSocket was closed, exit");
            break;
        }
    }
}
