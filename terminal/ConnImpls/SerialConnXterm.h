//
// (c) 2020 chiv
//
#ifndef _comxd_SerialConnXterm_h_
#define _comxd_SerialConnXterm_h_

#include "SerialConnECMA48.h"
#include "SerialConnVT102.h"
///
/// How dose Xterm works ?
///
///             VT
///        / ```|```\
///      /      |    \
///  Xterm, ECMA48, VT102 share the VT, they process control seq in orders.
///
///  is xterm ? Yes -> process with this class(Xterm)
///             No  -> is ecma48 ? Yes -> process seq with ECMA48
///                                No  -> is vt102 ? Yes -> process seq with VT102
///                                                  No  -> process seq with VT
///
class SerialConnXterm : public virtual SerialConnECMA48, virtual SerialConnVT102 {
public:
    using Superclass = SerialConnVT;
    SerialConnXterm(std::shared_ptr<SerialIo> serial);
    virtual ~SerialConnXterm();
    
protected:
	virtual Upp::WString TranscodeToUTF16(const VTChar& cc) const;
	virtual bool ProcessChar(Upp::dword cc);
    virtual int IsControlSeq(const std::string& seq);
    virtual bool ProcessControlSeq(const std::string& seq, int seq_type);
    virtual void ProcessXtermTrivial(const std::string& seq);
    virtual bool ProcessAsciiControlChar(char cc);
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    virtual bool ProcessKeyUp(Upp::dword key, Upp::dword flags);
    // override this to answer the shell program.
    virtual void ProcessDA(const std::string& seq);
private:
    // ECMA48Trivial, includes C1
    std::map<std::string, std::function<void()> > mXtermTrivialHandlers;
    std::map<int, std::function<void(const std::string& seq)> > mXtermFuncs;
    void InstallXtermFunctions();
    // support alternative screen
    ScreenData mAltScr;
    bool mIsAltScr;
};

#endif
