/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/ecma48/SerialConnEcma48.h"

class SerialConnVT102 : public SerialConnEcma48 {
public:
	SerialConnVT102(std::shared_ptr<SerialIo> io);
	// override
	virtual void ProcessDA(const std::string& p);
	virtual void ProcessDSR(const std::string& p);
	//
	virtual void ProcessVT102_DSR(const std::string& p);
	//
	virtual void ProcessVT102_MODE_SET(const std::string& p);
	virtual void ProcessVT102_MODE_RESET(const std::string& p);
	virtual void ProcessVT102_HOME(const std::string& p);
    //
    virtual void ProcessDECSTBM(const std::string& p);
	virtual void ProcessDECSC(const std::string& p);
	virtual void ProcessDECRC(const std::string& p);
	virtual void ProcessDECALN(const std::string& p);
	virtual void ProcessDECTST(const std::string& p);
	virtual void ProcessDECLL(const std::string& p);
    //
private:
    struct VT102Modes {
        enum DECCKM_Value {
            DECCKM_Cursor = 0,
            DECCKM_Application
        };
        uint32_t DECCKM: 1;
        enum DECANM_Value {
            DECANM_VT52 = 0,
            DECANM_ANSI
        };
        uint32_t DECANM: 1;
        enum DECCOLM_Value {
            DECCOLM_80 = 0,
            DECCOLM_132
        };
        uint32_t DECCOLM: 1;
        enum DECSCLM_Value {
            DECSCLM_Jump = 0,
            DECSCLM_Smooth
        };
        uint32_t DECSCLM: 1;
        enum DECSCNM_Value {
            DECSCNM_Normal = 0,
            DECSCNM_Reverse
        };
        uint32_t DECSCNM: 1;
        enum DECOM_Value {
            DECOM_Absolute = 0,
            DECOM_Relative
        };
        uint32_t DECOM: 1;
        enum BOOL_Value {
            OFF,
            ON
        };
        uint32_t DECAWM: 1;
        uint32_t DECARM: 1;
        uint32_t DECPFF: 1;
        enum DECPEX_Value {
            DECPEX_ScrollingRegion = 0,
            DECPEX_FullScreen
        };
        uint32_t DECPEX: 1;
        VT102Modes()
            : DECCKM(DECCKM_Cursor)
            , DECANM(DECANM_ANSI)
            , DECCOLM(DECCOLM_80)
            , DECSCLM(DECSCLM_Smooth)
            , DECSCNM(DECSCNM_Normal)
            , DECOM(DECOM_Absolute)
            , DECAWM(OFF)
            , DECARM(OFF)
            , DECPFF(OFF)
            , DECPEX(DECPEX_FullScreen)
        {
        }
    };
    VT102Modes mModes;
    //
    struct CursorData {
        int Vx, Vy;
        int Px, Py;
        VTStyle Style;
    };
    void SaveCursor(CursorData& cd);
    void LoadCursor(const CursorData& cd);
    CursorData mCursorData;
    //
    void InstallFunctions();
};
