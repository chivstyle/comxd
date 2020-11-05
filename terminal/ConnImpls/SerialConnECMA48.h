//
// (c) 2020 chiv
//
#ifndef _comxd_SerialConnECMA48_h_
#define _comxd_SerialConnECMA48_h_

#include "SerialConnVT.h"
// SerialConnECMA48 is a 8-bit vt, so we use \x1b<Fe> in C1
class SerialConnECMA48 : public virtual SerialConnVT {
public:
    using Superclass = SerialConnVT;
    SerialConnECMA48(std::shared_ptr<SerialIo> serial);
    virtual ~SerialConnECMA48();
protected:
    virtual int IsControlSeq(const std::string& seq);
    virtual bool ProcessAsciiControlChar(char cc);
    virtual bool ProcessControlSeq(const std::string& seq, int seq_type);
    //------------------------------------------------------------------------------------------
    // 00~0x1f
    virtual bool ProcessC0(char cc);
    // 0x1b c1, we have stripped the 0x1b already.
    virtual bool ProcessC1(char cc);
    //------------------------------------------------------------------------------------------
    virtual void ProcessSGR(const std::string& seq);
    virtual void ProcessSGR(int attr_code);
    virtual void ProcessEcma48Trivial(const std::string& seq);
    virtual void ProcessCBT(const std::string& seq);
    virtual void ProcessCHA(const std::string& seq);
    virtual void ProcessCHT(const std::string& seq);
    virtual void ProcessCNL(const std::string& seq);
    virtual void ProcessCPL(const std::string& seq);
    virtual void ProcessCPR(const std::string& seq);
    virtual void ProcessCTC(const std::string& seq);
    virtual void ProcessCTC(int ps);
    virtual void ProcessCUB(const std::string& seq);
    virtual void ProcessCUD(const std::string& seq);
    virtual void ProcessCUF(const std::string& seq);
    virtual void ProcessCUP(const std::string& seq);
    virtual void ProcessCUU(const std::string& seq);
    virtual void ProcessCVT(const std::string& seq);
    virtual void ProcessDA(const std::string& seq);
    virtual void ProcessDAQ(const std::string& seq);
    virtual void ProcessDCH(const std::string& seq);
    virtual void ProcessDL(const std::string& seq);
    virtual void ProcessDSR(const std::string& seq);
    virtual void ProcessDTA(const std::string& seq);
    virtual void ProcessEA(const std::string& seq);
    virtual void ProcessECH(const std::string& seq);
    virtual void ProcessED(const std::string& seq);
    virtual void ProcessEF(const std::string& seq);
    virtual void ProcessEL(const std::string& seq);
    virtual void ProcessFNK(const std::string& seq);
    virtual void ProcessFNT(const std::string& seq);
    virtual void ProcessGCC(const std::string& seq);
    virtual void ProcessGSM(const std::string& seq);
    virtual void ProcessGSS(const std::string& seq);
    virtual void ProcessHPA(const std::string& seq);
    virtual void ProcessHPB(const std::string& seq);
    virtual void ProcessHPR(const std::string& seq);
    virtual void ProcessHVP(const std::string& seq);
    virtual void ProcessICH(const std::string& seq);
    virtual void ProcessIDCS(const std::string& seq);
    virtual void ProcessIGS(const std::string& seq);
    virtual void ProcessIL(const std::string& seq);
    virtual void ProcessJFY(const std::string& seq);
    virtual void ProcessMC(const std::string& seq);
    virtual void ProcessNP(const std::string& seq);
    virtual void ProcessPEC(const std::string& seq);
    virtual void ProcessPFS(const std::string& seq);
    virtual void ProcessPP(const std::string& seq);
    virtual void ProcessPPA(const std::string& seq);
    virtual void ProcessPPB(const std::string& seq);
    virtual void ProcessPTX(const std::string& seq);
    virtual void ProcessQUAD(const std::string& seq);
    virtual void ProcessREP(const std::string& seq);
    virtual void ProcessRM(const std::string& seq);
    virtual void ProcessSACS(const std::string& seq);
    virtual void ProcessSAPV(const std::string& seq);
    virtual void ProcessSCO(const std::string& seq);
    virtual void ProcessSCP(const std::string& seq);
    virtual void ProcessSCS(const std::string& seq);
    virtual void ProcessSD(const std::string& seq);
    virtual void ProcessSDS(const std::string& seq);
    virtual void ProcessSEE(const std::string& seq);
    virtual void ProcessSEF(const std::string& seq);
    virtual void ProcessSHS(const std::string& seq);
    virtual void ProcessSIMD(const std::string& seq);
    virtual void ProcessSL(const std::string& seq);
    virtual void ProcessSLH(const std::string& seq);
    virtual void ProcessSLL(const std::string& seq);
    virtual void ProcessSLS(const std::string& seq);
    virtual void ProcessSM(const std::string& seq);
    virtual void ProcessSPD(const std::string& seq);
    virtual void ProcessSPH(const std::string& seq);
    virtual void ProcessSPI(const std::string& seq);
    virtual void ProcessSPL(const std::string& seq);
    virtual void ProcessSPQR(const std::string& seq);
    virtual void ProcessSR(const std::string& seq);
    virtual void ProcessSRCS(const std::string& seq);
    virtual void ProcessSRS(const std::string& seq);
    virtual void ProcessSSU(const std::string& seq);
    virtual void ProcessSSW(const std::string& seq);
    virtual void ProcessSTAB(const std::string& seq);
    virtual void ProcessSU(const std::string& seq);
    virtual void ProcessSVS(const std::string& seq);
    virtual void ProcessTAC(const std::string& seq);
    virtual void ProcessTALE(const std::string& seq);
    virtual void ProcessTATE(const std::string& seq);
    virtual void ProcessTBC(const std::string& seq);
    virtual void ProcessTCC(const std::string& seq);
    virtual void ProcessTSR(const std::string& seq);
    virtual void ProcessTSS(const std::string& seq);
    virtual void ProcessVPA(const std::string& seq);
    virtual void ProcessVPB(const std::string& seq);
    virtual void ProcessVPR(const std::string& seq);
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
            , GATM(GatmAll)
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
