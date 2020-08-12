//
// (c) 2020 chiv
//
#ifndef _comxd_SerialConnECMA48_h_
#define _comxd_SerialConnECMA48_h_

#include "SerialConnVT102.h"
// SerialConnECMA48 is a 8-bit vt, so we use \x1b<Fe> in C1
class SerialConnECMA48 : public SerialConnVT102 {
public:
    using Superclass = SerialConnVT102;
    SerialConnECMA48(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnECMA48();
protected:
    virtual int IsControlSeq(const std::string& seq);
    virtual void ProcessAsciiControlChar(char cc);
    virtual void ProcessControlSeq(const std::string& seq, int seq_type);
    //------------------------------------------------------------------------------------------
    // allow subclass to extend ECMA48
    virtual void ProcessSGR(const std::string& seq);
    virtual void ProcessEcma48Trivial(const std::string& seq);
    virtual void ProcessC0(char cc);
    virtual void ProcessC1(char cc);
    // extend VT102 to support ecma48 attributes.
    virtual void ProcessVT102CharAttribute(int attr_code);
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
    // PUM - character, PUM, ZDM are deprecated
    // ZDM - ZERO
    // SATM - ALL
    // SRM - simultaneous
    // SRTM - normal
    // TSM - single
    // TTM - ALL
    // VEM - following
    //------------------------------------------------------------------------------------------
    // ECMA48Trivial, includes C1
    std::map<std::string, std::function<void(const std::string& seq)> > mEcma48TrivialHandlers;
    std::map<int, std::function<void(const std::string& seq)> > mEcma48Funcs;
    void InstallEcma48Functions();
};

#endif
