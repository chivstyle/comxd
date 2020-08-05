//
// (c) 2020 chiv
//
#ifndef _comxd_ConnVT102_h_
#define _comxd_ConnVT102_h_

#include "SerialConnVT.h"

class SerialConnVT102 : public SerialConnVT {
public:
    using Superclass = SerialConnVT;
    SerialConnVT102(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnVT102();
protected:
    /// workflow
    ///
    /// rx_buffer, check byte one by one, then
    ///    seq_type = IsControlSeq(seq)
    ///    seq_type == 0, can't recognize the control seq, treat it as normal text
    ///    seq_type == 1, pending, need more bytes to confirm
    ///    seq_type == 2, trivial control seq, it's in the table 'kVT102ControlSeqs'
    ///    seq_type > 2, other kinds of control seq
    /// then, RenderText(seq, seq_type)
    ///    ProcessAsciiControlChar(cc), cc is 8, 9, 0xd, 0xa, .etc
    ///    ProcessControlSeq(seq), seq matches control seq pattern
    ///
    virtual int IsControlSeq(const std::string& seq);
    // This is VT102 cursor key codes
    void ProcessVT102CursorKeyCodes(const std::string& seq);
    void ProcessVT102EditingFunctions(const std::string& seq);
    /// seq matches pattern \033[[number];[number];[number]<m>, this routine parse
    /// the attr_code, i.e the number between ';' was passed to next routine -- ProcessAttr
    void ProcessVT102Attrs(const std::string& seq);
    /// see ProcessVT102Attrs
    /// VT102 supports 1,2,4,5,7, so the subclass could override this routine to support
    /// more attributes.
    virtual void ProcessAttr(const std::string& attr_code);
    //-------------------------------------------------------------------------------------
    // you can override these two routines to modify all functions.
    virtual void ProcessControlSeq(const std::string& seq, int seq_type);
    virtual void ProcessAsciiControlChar(unsigned char cc);
    //-------------------------------------------------------------------------------------
    // with K_DELTA
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
    virtual bool ProcessKeyUp(Upp::dword key, Upp::dword flags);
    // pure ascii, 32 ~ 126
    virtual bool ProcessKeyDown_Ascii(Upp::dword key, Upp::dword flags);
private:
    void InstallControlSeqHandlers();
};

#endif
