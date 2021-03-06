//
// (c) 2020 chiv
//
#include "connimpls/SerialConnVT.h"

class Ecma48ControlSeqFactory;
class SerialConnEcma48 : public virtual SerialConnVT {
    using Superclass = SerialConnVT;
public:
    SerialConnEcma48(std::shared_ptr<SerialIo> io);
    ~SerialConnEcma48();

protected:
    virtual void ProcessSOH(const std::string_view&);
    virtual void ProcessSTX(const std::string_view&);
    virtual void ProcessETX(const std::string_view&);
    virtual void ProcessEOT(const std::string_view&);
    virtual void ProcessENQ(const std::string_view&);
    virtual void ProcessACK(const std::string_view&);
    virtual void ProcessBEL(const std::string_view&);
    virtual void ProcessBS(const std::string_view&);
    virtual void ProcessHT(const std::string_view&);
    virtual void ProcessLF(const std::string_view&);
    virtual void ProcessVT(const std::string_view&);
    virtual void ProcessFF(const std::string_view&);
    virtual void ProcessCR(const std::string_view&);
    virtual void ProcessSI(const std::string_view&);
    virtual void ProcessSO(const std::string_view&);
    virtual void ProcessDLE(const std::string_view&);
    virtual void ProcessDC1(const std::string_view&);
    virtual void ProcessDC2(const std::string_view&);
    virtual void ProcessDC3(const std::string_view&);
    virtual void ProcessDC4(const std::string_view&);
    virtual void ProcessNAK(const std::string_view&);
    virtual void ProcessSYN(const std::string_view&);
    virtual void ProcessETB(const std::string_view&);
    virtual void ProcessCAN(const std::string_view&);
    virtual void ProcessEM(const std::string_view&);
    virtual void ProcessSUB(const std::string_view&);
    virtual void ProcessFS(const std::string_view&);
    virtual void ProcessGS(const std::string_view&);
    virtual void ProcessRS(const std::string_view&);
    virtual void ProcessUS(const std::string_view&);
    virtual void ProcessDEL(const std::string_view&);
    // C1
    virtual void ProcessAPC(const std::string_view& p);
    virtual void ProcessBPH(const std::string_view& p);
    virtual void ProcessCBT(const std::string_view& p);
    virtual void ProcessCCH(const std::string_view& p);
    virtual void ProcessCHA(const std::string_view& p);
    virtual void ProcessCHT(const std::string_view& p);
    virtual void ProcessCMD(const std::string_view& p);
    virtual void ProcessCNL(const std::string_view& p);
    virtual void ProcessCPL(const std::string_view& p);
    virtual void ProcessCPR(const std::string_view& p);
    virtual void ProcessCTC(const std::string_view& p);
    virtual void ProcessCUB(const std::string_view& p);
    virtual void ProcessCUD(const std::string_view& p);
    virtual void ProcessCUF(const std::string_view& p);
    virtual void ProcessCUP(const std::string_view& p);
    virtual void ProcessCUU(const std::string_view& p);
    virtual void ProcessCVT(const std::string_view& p);
    virtual void ProcessDA(const std::string_view& p);
    virtual void ProcessDAQ(const std::string_view& p);
    virtual void ProcessDCH(const std::string_view& p);
    virtual void ProcessDCS(const std::string_view& p);
    virtual void ProcessDL(const std::string_view& p);
    virtual void ProcessDMI(const std::string_view& p);
    virtual void ProcessDSR(const std::string_view& p);
    virtual void ProcessDTA(const std::string_view& p);
    virtual void ProcessEA(const std::string_view& p);
    virtual void ProcessECH(const std::string_view& p);
    virtual void ProcessED(const std::string_view& p);
    virtual void ProcessEF(const std::string_view& p);
    virtual void ProcessEL(const std::string_view& p);
    virtual void ProcessEMI(const std::string_view& p);
    virtual void ProcessEPA(const std::string_view& p);
    virtual void ProcessESA(const std::string_view& p);
    virtual void ProcessFNK(const std::string_view& p);
    virtual void ProcessFNT(const std::string_view& p);
    virtual void ProcessGCC(const std::string_view& p);
    virtual void ProcessGSM(const std::string_view& p);
    virtual void ProcessGSS(const std::string_view& p);
    virtual void ProcessHPA(const std::string_view& p);
    virtual void ProcessHPB(const std::string_view& p);
    virtual void ProcessHPR(const std::string_view& p);
    virtual void ProcessHTJ(const std::string_view& p);
    virtual void ProcessHTS(const std::string_view& p);
    virtual void ProcessHVP(const std::string_view& p);
    virtual void ProcessICH(const std::string_view& p);
    virtual void ProcessIDCS(const std::string_view& p);
    virtual void ProcessIGS(const std::string_view& p);
    virtual void ProcessIL(const std::string_view& p);
    virtual void ProcessINT(const std::string_view& p);
    virtual void ProcessJFY(const std::string_view& p);
    virtual void ProcessLS1R(const std::string_view& p);
    virtual void ProcessLS2(const std::string_view& p);
    virtual void ProcessLS2R(const std::string_view& p);
    virtual void ProcessLS3(const std::string_view& p);
    virtual void ProcessLS3R(const std::string_view& p);
    virtual void ProcessMC(const std::string_view& p);
    virtual void ProcessMW(const std::string_view& p);
    virtual void ProcessNBH(const std::string_view& p);
    virtual void ProcessNEL(const std::string_view& p);
    virtual void ProcessNP(const std::string_view& p);
    virtual void ProcessOSC(const std::string_view& p);
    virtual void ProcessPEC(const std::string_view& p);
    virtual void ProcessPFS(const std::string_view& p);
    virtual void ProcessPLD(const std::string_view& p);
    virtual void ProcessPLU(const std::string_view& p);
    virtual void ProcessPM(const std::string_view& p);
    virtual void ProcessPP(const std::string_view& p);
    virtual void ProcessPPA(const std::string_view& p);
    virtual void ProcessPPB(const std::string_view& p);
    virtual void ProcessPPR(const std::string_view& p);
    virtual void ProcessPTX(const std::string_view& p);
    virtual void ProcessPU1(const std::string_view& p);
    virtual void ProcessPU2(const std::string_view& p);
    virtual void ProcessQUAD(const std::string_view& p);
    virtual void ProcessREP(const std::string_view& p);
    virtual void ProcessRI(const std::string_view& p);
    virtual void ProcessRIS(const std::string_view& p);
    virtual void ProcessRM(const std::string_view& p);
    virtual void ProcessSACS(const std::string_view& p);
    virtual void ProcessSAPV(const std::string_view& p);
    virtual void ProcessSCI(const std::string_view& p);
    virtual void ProcessSCO(const std::string_view& p);
    virtual void ProcessSCP(const std::string_view& p);
    virtual void ProcessSCS(const std::string_view& p);
    virtual void ProcessSD(const std::string_view& p);
    virtual void ProcessSDS(const std::string_view& p);
    virtual void ProcessSEE(const std::string_view& p);
    virtual void ProcessSEF(const std::string_view& p);
    virtual void ProcessSGR(const std::string_view& p);
    virtual void ProcessSHS(const std::string_view& p);
    virtual void ProcessSIMD(const std::string_view& p);
    virtual void ProcessSL(const std::string_view& p);
    virtual void ProcessSLH(const std::string_view& p);
    virtual void ProcessSLL(const std::string_view& p);
    virtual void ProcessSLS(const std::string_view& p);
    virtual void ProcessSM(const std::string_view& p);
    virtual void ProcessSOS(const std::string_view& p);
    virtual void ProcessSPA(const std::string_view& p);
    virtual void ProcessSPD(const std::string_view& p);
    virtual void ProcessSPH(const std::string_view& p);
    virtual void ProcessSPI(const std::string_view& p);
    virtual void ProcessSPL(const std::string_view& p);
    virtual void ProcessSPQR(const std::string_view& p);
    virtual void ProcessSR(const std::string_view& p);
    virtual void ProcessSRCS(const std::string_view& p);
    virtual void ProcessSRS(const std::string_view& p);
    virtual void ProcessSSA(const std::string_view& p);
    virtual void ProcessSSU(const std::string_view& p);
    virtual void ProcessSSW(const std::string_view& p);
    virtual void ProcessSS2(const std::string_view& p);
    virtual void ProcessSS3(const std::string_view& p);
    virtual void ProcessST(const std::string_view& p);
    virtual void ProcessSTAB(const std::string_view& p);
    virtual void ProcessSTS(const std::string_view& p);
    virtual void ProcessSU(const std::string_view& p);
    virtual void ProcessSVS(const std::string_view& p);
    virtual void ProcessTAC(const std::string_view& p);
    virtual void ProcessTALE(const std::string_view& p);
    virtual void ProcessTATE(const std::string_view& p);
    virtual void ProcessTBC(const std::string_view& p);
    virtual void ProcessTCC(const std::string_view& p);
    virtual void ProcessTSR(const std::string_view& p);
    virtual void ProcessTSS(const std::string_view& p);
    virtual void ProcessVPA(const std::string_view& p);
    virtual void ProcessVPB(const std::string_view& p);
    virtual void ProcessVPR(const std::string_view& p);
    virtual void ProcessVTS(const std::string_view& p);
    //
    virtual bool ProcessChar(Upp::dword cc);
    virtual bool ProcessOverflowLines(const struct Seq&);
    //
    void SetUseS8C(bool b);
    bool IsControlSeqPrefix(uint8_t c);
    int IsControlSeq(const std::string_view& seq, size_t& p_begin, size_t& p_sz, size_t& s_end);
    void RefineTheInput(std::string& raw);
    void Put(const std::string& s);
	//
    struct EcmaDAQ {
        Upp::Point From;
        Upp::Point To;
        int        Idx;
        EcmaDAQ()
            : Idx(0)
        {
        }
    };
    EcmaDAQ mDaq[12];
    // 0 the shifted part is limited to the active page in the presentation component
    // 1 the shifted part is limited to the active line in the presentation component
    // 2 the shifted part is limited to the active field in the presentation component
    // 3 the shifted part is limited to the active qualified area
    // 4 the shifted part consists of the relevant part of the entire presentation component.
    int mSee;
    int mPageHome;
    int mLineHome;
    // ANSI MODES
    struct Ecma48Modes {
        enum BDSMValue { BDSM_Explicit = 0, BDSM_Implicit };
        uint32_t BDSM: 1;
        enum CRMValue { CRM_Control = 0, CRM_Graphic };
        uint32_t CRM: 1;
        enum DCSMValue { DCSM_Presentation = 0, DCSM_Data };
        uint32_t DCSM: 1;
        enum ERMValue { ERM_Protect = 0, ERM_All };
        uint32_t ERM: 1;
        enum FEAMValue { FEAM_Execute = 0, FEAM_Store };
        uint32_t FEAM: 1;
        enum FETMValue { FETM_Insert = 0, FETM_Exclude };
        uint32_t FETM: 1;
        enum GATMValue { GATM_Guad = 0, GATM_All };
        uint32_t GATM: 1;
        enum GRCMValue { GRCM_Replacing = 0, GRCM_Cumulative };
        uint32_t GRCM: 1;
        enum HEMValue { HEM_Following = 0, HEM_Preceding };
        uint32_t HEM: 1;
        enum IRMValue { IRM_Replace = 0, IRM_Insert };
        uint32_t IRM: 1;
        enum KAMValue { KAM_Enabled = 0, KAM_Disabled };
        uint32_t KAM: 1;
        enum MATMValue { MATM_Single = 0, MATM_Multiple };
        uint32_t MATM: 1;
        uint32_t PUM: 1; // PUM was deprecated
        enum SATMValue { SATM_Select = 0, SATM_All };
        uint32_t SATM: 1;
        enum SRMValue { SRM_Monitor = 0, SRM_Simultaneous };
        uint32_t SRM: 1;
        enum SRTMValue { SRTM_Normal = 0, SRTM_Diagnostic };
        uint32_t SRTM: 1;
        enum TSMValue { TSM_Multiple = 0, TSM_Single };
        uint32_t TSM: 1;
        enum TTMValue { TTM_Cursor = 0, TTM_All };
        uint32_t TTM: 1;
        enum VEMValue { VEM_Following = 0, VEM_Preceding };
        uint32_t VEM: 1;
        uint32_t ZDM: 1; // ZDM was deprecated
        uint32_t EBM: 1; // Elimited mode
        enum LMNValue { LMN_NewLine = 0, LMN_LineFeed };
        uint32_t LMN: 1; // Elimited mode, but VT102 will use it.
        Ecma48Modes()
            : BDSM(BDSM_Implicit)
            , CRM(CRM_Control)
            , DCSM(DCSM_Presentation)
            , ERM(ERM_All)
            , FEAM(FEAM_Execute)
            , FETM(FETM_Exclude)
            , GATM(GATM_All)
            , GRCM(GRCM_Cumulative)
            , HEM(HEM_Following)
            , IRM(IRM_Replace)
            , KAM(KAM_Enabled)
            , MATM(MATM_Single)
            , SATM(SATM_Select)
            , SRM(SRM_Simultaneous)
            , SRTM(SRTM_Normal)
            , TSM(TSM_Single)
            , TTM(TTM_All)
            , VEM(VEM_Following)
            , LMN(LMN_NewLine)
        {
        }
    };
    struct Ecma48Modes mModes;
    // fill region with c
    void Fill(int X0, int Y0, int X1, int Y1, const VTChar& c);
private:
	bool mUseS8C; // 8-bit control seq
    void InstallFunctions();
};
