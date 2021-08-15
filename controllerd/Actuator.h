#ifndef _controllerd_Actuator_h_
#define _controllerd_Actuator_h_

#include <Core/Core.h>

class Actuator {
public:
    Actuator(const Upp::Value& conf);
    Actuator(const Upp::String& URL);
    virtual ~Actuator();
    // send message to Actuator
    bool SendText(const Upp::String& message);
    //
    Upp::Event<Upp::WebSocket&, const Upp::String&> WhenMessage;
    //
    void Run(volatile bool* should_exit);
    //
private:
    Upp::WebSocket mWs;
};

#endif
