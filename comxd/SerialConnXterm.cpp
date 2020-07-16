//
// (c) 2020 chiv
//
#include "SerialConnXterm.h"
#include "XtermControlSeq.h"
#include "ConnFactory.h"

REGISTER_CONN_INSTANCE("Xterm", SerialConnXterm);

using namespace Upp;

SerialConnXterm::SerialConnXterm(std::shared_ptr<serial::Serial> serial)
    : SerialConnVT102(serial)
{
    InstallXtermControlSeqHandlers();
}
//
SerialConnXterm::~SerialConnXterm()
{
}

int SerialConnXterm::IsControlSeq(const std::string& seq)
{
    int ret = IsXtermControlSeq(seq);
    if (ret == 0) { // It's not a xterm control seq absolutely
        return SerialConnVT102::IsControlSeq(seq);
    }
    return ret;
}

void SerialConnXterm::InstallXtermControlSeqHandlers()
{
    //
    mCtrlHandlers["[1;0m"] = mCtrlHandlers["[m"];
    //
    mCtrlHandlers["[1;30m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = Color(0, 0, 0);
        };
    };
    mCtrlHandlers["[1;31m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = Color(255, 0, 0);
        };
    };
    mCtrlHandlers["[1;32m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = Color(0, 255, 0);
        };
    };
    mCtrlHandlers["[1;33m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = Color(255, 255, 0);
        };
    };
    mCtrlHandlers["[1;34m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = Color(0, 0, 255);
        };
    };
    mCtrlHandlers["[1;35m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = Color(255, 0, 255);
        };
    };
    mCtrlHandlers["[1;36m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = Color(0, 255, 255);
        };
    };
    mCtrlHandlers["[1;37m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = Color(255, 255, 255);
        };
    };
    /// reserved for future standardization
    mCtrlHandlers["[1;38m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = Color(255, 255, 255);
        };
    };
    /// default display color
    mCtrlHandlers["[1;39m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mFgColor = mDefaultFgColor;
        };
    };
    // background
    mCtrlHandlers["[1;40m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = Color(0, 0, 0);
        };
    };
    mCtrlHandlers["[1;41m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = Color(255, 0, 0);
        };
    };
    mCtrlHandlers["[1;42m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = Color(0, 255, 0);
        };
    };
    mCtrlHandlers["[1;43m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = Color(255, 255, 0);
        };
    };
    mCtrlHandlers["[1;44m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = Color(0, 0, 255);
        };
    };
    mCtrlHandlers["[1;45m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = Color(255, 0, 255);
        };
    };
    mCtrlHandlers["[1;46m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = Color(0, 255, 255);
        };
    };
    mCtrlHandlers["[1;47m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = Color(255, 255, 255);
        };
    };
    /// reserved for future standardization
    mCtrlHandlers["[1;48m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = Color(255, 255, 255);
        };
    };
    /// default display color
    mCtrlHandlers["[1;49m"] = [=]() {
        mCurrAttrFunc = [=]() {
            mBgColor = mDefaultFgColor;
        };
    };
    ///
    mCtrlHandlers["[0;0m"] = mCtrlHandlers["[m"];
    mCtrlHandlers["[0;30m"] = mCtrlHandlers["[1;30m"];
    mCtrlHandlers["[0;31m"] = mCtrlHandlers["[1;31m"];
    mCtrlHandlers["[0;32m"] = mCtrlHandlers["[1;32m"];
    mCtrlHandlers["[0;33m"] = mCtrlHandlers["[1;33m"];
    mCtrlHandlers["[0;34m"] = mCtrlHandlers["[1;34m"];
    mCtrlHandlers["[0;35m"] = mCtrlHandlers["[1;35m"];
    mCtrlHandlers["[0;36m"] = mCtrlHandlers["[1;36m"];
    mCtrlHandlers["[0;37m"] = mCtrlHandlers["[1;37m"];
    mCtrlHandlers["[0;38m"] = mCtrlHandlers["[1;38m"];
    mCtrlHandlers["[0;39m"] = mCtrlHandlers["[1;39m"];
    mCtrlHandlers["[0;40m"] = mCtrlHandlers["[1;40m"];
    mCtrlHandlers["[0;41m"] = mCtrlHandlers["[1;41m"];
    mCtrlHandlers["[0;42m"] = mCtrlHandlers["[1;42m"];
    mCtrlHandlers["[0;43m"] = mCtrlHandlers["[1;43m"];
    mCtrlHandlers["[0;44m"] = mCtrlHandlers["[1;44m"];
    mCtrlHandlers["[0;45m"] = mCtrlHandlers["[1;45m"];
    mCtrlHandlers["[0;46m"] = mCtrlHandlers["[1;46m"];
    mCtrlHandlers["[0;47m"] = mCtrlHandlers["[1;47m"];
    mCtrlHandlers["[0;48m"] = mCtrlHandlers["[1;48m"];
    mCtrlHandlers["[0;49m"] = mCtrlHandlers["[1;49m"];
}
