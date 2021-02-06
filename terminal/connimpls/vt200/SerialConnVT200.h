/*!
// (c) 2021 chiv
//
*/
#pragma once

#include "connimpls/vt100/SerialConnVT100.h"

class SerialConnVT200 : public SerialConnVT100 {
public:
	SerialConnVT200(std::shared_ptr<SerialIo> io);
	//
protected:
	void ProcessSS2(const std::string&);
	void ProcessSS3(const std::string&);
	void ProcessDECSM(const std::string&);
	void ProcessDECRM(const std::string&);
	void ProcessDECDSR(const std::string&);
	void ProcessDECSC(const std::string&);
	void ProcessDECRC(const std::string&);
	// vt200
	void ProcessDECSCL(const std::string&);
	void ProcessG2_CS(const std::string&);
	void ProcessG3_CS(const std::string&);
	void ProcessS7C1T(const std::string&);
	void ProcessS8C1T(const std::string&);
	void ProcessDECKPAM(const std::string&);
	void ProcessDECKPNM(const std::string&);
	void ProcessDECSCA(const std::string&);
	void ProcessDECSEL(const std::string&);
	void ProcessDECSED(const std::string&);
	void ProcessDECSTR(const std::string&);
	void ProcessSecondaryDA(const std::string&);
	// override
	uint32_t RemapCharacter(uint32_t uc, int charset);
	// VT200 cursor data
	struct CursorDataVT200 : public CursorDataVT100 {
		bool       SelectiveErase;
		uint32_t   DECOM : 1;
	};
	CursorDataVT200 mCursorData;
	void SaveCursorData(CursorDataVT200& cd);
	void LoadCursorData(const CursorDataVT200& cd);
	//
	enum KeypadMode {
		KM_DECKPAM,
		KM_DECKPNM
	};
	int mKeypadMode;
	// selective erase attribute bit write state
	bool mSelectiveErase;
	//
	struct VT200Modes {
		enum DECNRCMValue {
			DECNRCM_Multinational = 0,
			DECNRCM_National
		};
		uint32_t DECNRCM : 1;
	};
	VT200Modes mModes;
private:
	void InstallFunctions();
};
