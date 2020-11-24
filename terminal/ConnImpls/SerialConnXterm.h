//
// (c) 2020 chiv
//
// TODO: Xterm is so complicated, so complicated, so we'll implement it
//       step by step.
//
//       ECMA48 -> VT512
//
#ifndef _comxd_SerialConnXterm_h_
#define _comxd_SerialConnXterm_h_

#include "SerialConnAnsi.h"
#include "SerialConnVT102.h"
///
/// How dose Xterm works ?
///
///             VT
///        / ```|```\
///      /      |    \
///  Xterm, Ansi, VT102 share the VT, they process control seq in orders.
///
///  is xterm ? Yes -> process with this class(Xterm)
///             No  -> is ansi ? Yes -> process seq with Ansi
///                                No  -> is vt102 ? Yes -> process seq with VT102
///                                                  No  -> process seq with VT
///
class SerialConnXterm : public virtual SerialConnAnsi, virtual SerialConnVT102 {
public:
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
    //
    virtual void RenderSeqs()
    {
        SerialConnVT::RenderSeqs();
    }
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
