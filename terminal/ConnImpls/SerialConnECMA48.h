//
// (c) 2020 chiv
//
#ifndef _comxd_SerialConnECMA48_h_
#define _comxd_SerialConnECMA48_h_

#include "SerialConnVT.h"
// SerialConnECMA48 is a 8-bit vt, so we use \x1b<Fe> in C1
class SerialConnECMA48 : public SerialConnVT {
public:
    using Superclass = SerialConnVT;
    SerialConnECMA48(std::shared_ptr<SerialIo> serial);
    virtual ~SerialConnECMA48();
protected:
    virtual int IsControlSeq(const std::string& seq);
    virtual bool ProcessAsciiControlChar(char cc);
    virtual void ProcessControlSeq(const std::string& seq, int seq_type);
    //------------------------------------------------------------------------------------------
    bool ProcessC0(char cc);
    bool ProcessC1(char cc);
    //
    void ProcessSGR(const std::string& seq);
    void ProcessSGR(int attr_code);
    void ProcessEcma48Trivial(const std::string& seq);
    void ProcessCHA(const std::string& seq);
    void ProcessCHT(const std::string& seq);
    void ProcessCNL(const std::string& seq);
    void ProcessCPL(const std::string& seq);
    void ProcessCPR(const std::string& seq);
    void ProcessCTC(const std::string& seq);
    void ProcessCTC(int ps);
    void ProcessCUB(const std::string& seq);
    void ProcessCUD(const std::string& seq);
    void ProcessCUF(const std::string& seq);
    void ProcessCUP(const std::string& seq);
    void ProcessCUU(const std::string& seq);
    void ProcessCVT(const std::string& seq);
    void ProcessDA(const std::string& seq);
    void ProcessDAQ(const std::string& seq);
    void ProcessDCH(const std::string& seq);
    void ProcessDL(const std::string& seq);
    void ProcessDSR(const std::string& seq);
    void ProcessDTA(const std::string& seq);
    void ProcessEA(const std::string& seq);
    void ProcessECH(const std::string& seq);
    void ProcessED(const std::string& seq);
    void ProcessEF(const std::string& seq);
    void ProcessEL(const std::string& seq);
    void ProcessFNK(const std::string& seq);
    void ProcessFNT(const std::string& seq);
    void ProcessGCC(const std::string& seq);
    void ProcessGSM(const std::string& seq);
    void ProcessGSS(const std::string& seq);
    void ProcessHPA(const std::string& seq);
    void ProcessHPB(const std::string& seq);
    void ProcessHPR(const std::string& seq);
    void ProcessHVP(const std::string& seq);
    void ProcessICH(const std::string& seq);
    void ProcessIDCS(const std::string& seq);
    void ProcessIGS(const std::string& seq);
    void ProcessIL(const std::string& seq);
    void ProcessJFY(const std::string& seq);
    void ProcessMC(const std::string& seq);
    void ProcessNP(const std::string& seq);
    void ProcessPEC(const std::string& seq);
    void ProcessPFS(const std::string& seq);
    void ProcessPP(const std::string& seq);
    void ProcessPPA(const std::string& seq);
    void ProcessPPB(const std::string& seq);
    void ProcessPTX(const std::string& seq);
    void ProcessQUAD(const std::string& seq);
    void ProcessREP(const std::string& seq);
    void ProcessRM(const std::string& seq);
    void ProcessSACS(const std::string& seq);
    void ProcessSAPV(const std::string& seq);
    void ProcessSCO(const std::string& seq);
    void ProcessSCP(const std::string& seq);
    void ProcessSCS(const std::string& seq);
    void ProcessSD(const std::string& seq);
    void ProcessSDS(const std::string& seq);
    void ProcessSEE(const std::string& seq);
    void ProcessSEF(const std::string& seq);
    void ProcessSHS(const std::string& seq);
    void ProcessSIMD(const std::string& seq);
    void ProcessSL(const std::string& seq);
    void ProcessSLH(const std::string& seq);
    void ProcessSLL(const std::string& seq);
    void ProcessSLS(const std::string& seq);
    void ProcessSM(const std::string& seq);
    void ProcessSPD(const std::string& seq);
    void ProcessSPH(const std::string& seq);
    void ProcessSPI(const std::string& seq);
    void ProcessSPL(const std::string& seq);
    void ProcessSPQR(const std::string& seq);
    void ProcessSR(const std::string& seq);
    void ProcessSRCS(const std::string& seq);
    void ProcessSRS(const std::string& seq);
    void ProcessSSU(const std::string& seq);
    void ProcessSSW(const std::string& seq);
    void ProcessSTAB(const std::string& seq);
    void ProcessSU(const std::string& seq);
    void ProcessSVS(const std::string& seq);
    void ProcessTAC(const std::string& seq);
    void ProcessTALE(const std::string& seq);
    void ProcessTATE(const std::string& seq);
    void ProcessTBC(const std::string& seq);
    void ProcessTCC(const std::string& seq);
    void ProcessTSR(const std::string& seq);
    void ProcessTSS(const std::string& seq);
    void ProcessVPA(const std::string& seq);
    void ProcessVPB(const std::string& seq);
    void ProcessVPR(const std::string& seq);
    //
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
private:
    // options.
    // ECMA48 is somewhat more complicated than imagine. We use predefined modes
    // BDSM - explicit
    // CRM - control
    // DCSM - presentation
    // ERM - protected
    // FEAM - execute
    // FETM - insert
    // GATM - GUARD
    // GRCM - cumulative
    // HEM - following
    // RM - replace
    // KAM - enabled
    // MATM - single
    // PUM - character, PUM, ZDM are deprecated, we do not support them.
    // ZDM - ZERO
    // SATM - ALL
    // SRM - simultaneous
    // SRTM - normal
    // TSM - single
    // TTM - ALL
    // VEM - following
    struct Ecma48Modes {
        enum BDSMValue {BdsmExplicit, BdsmImplicit};
        unsigned int BDSM: 1;
        enum CRMValue {CrmControl, CrmGraphic};
        unsigned int CRM: 1;
        enum DCSMValue {DcsmPresentation, DcsmData};
        unsigned int DCSM: 1;
        enum ERMValue {ErmProtect, ErmAll};
        unsigned int ERM: 1;
        enum FEAMValue {FeamExecute, FeamStore};
        unsigned int FEAM: 1;
        enum FETMValue {FetmInsert, FetmExclude};
        unsigned int FETM: 1;
        enum GATMValue {GatmGuard, GatmAll};
        unsigned int GATM: 1;
        enum GRCMValue {GrcmReplacing, GrcmCumulative};
        unsigned int GRCM: 1;
        enum HEMValue {HemFollowing, HemPreceding};
        unsigned int HEM: 1;
        enum IRMValue {IrmReplace, IrmInsert};
        unsigned int IRM: 1;
        enum KAMValue {KamEnabled, KamDisabled};
        unsigned int KAM: 1;
        enum MATMValue {MatmSingle, MatmMultiple};
        unsigned int MATM: 1;
        enum SATMValue {SatmSelect, SatmAll};
        unsigned int SATM: 1;
        enum SRMValue {SrmMonitor, SrmSimultaneous};
        unsigned int SRM: 1;
        enum SRTMValue {SrtmNormal, SrtmDiagnostic};
        unsigned int SRTM: 1;
        enum TSMValue {TsmMultiple, TsmSingle};
        unsigned int TSM: 1;
        enum TIMValue {TimCursor, TimAll};
        unsigned int TIM: 1;
        enum VEMValue {VemFollowing, VemPreceding};
        unsigned int VEM: 1;
        //
        Ecma48Modes()
            : BDSM(BdsmExplicit)
            , CRM(CrmControl)
            , DCSM(DcsmPresentation)
            , ERM(ErmAll)
            , FEAM(FeamExecute)
            , FETM(FetmInsert)
            , GRCM(GrcmCumulative)
            , HEM(HemFollowing)
            , IRM(IrmReplace)
            , KAM(KamEnabled)
            , MATM(MatmMultiple)
            , SATM(SatmAll)
            , SRM(SrmSimultaneous)
            , SRTM(SrtmNormal)
            , TSM(TsmSingle)
            , TIM(TimAll)
            , VEM(VemFollowing)
        {
        }
    };
    Ecma48Modes mModes;
    //------------------------------------------------------------------------------------------
    // ECMA48Trivial, includes C1
    std::map<std::string, std::function<void()> > mEcma48TrivialHandlers;
    std::map<int, std::function<void(const std::string& seq)> > mEcma48Funcs;
    void InstallEcma48Functions();
};

#endif
