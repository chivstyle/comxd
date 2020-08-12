//
// (c) 2020 chiv
//
#ifndef _comxd_SerialConnXterm_h_
#define _comxd_SerialConnXterm_h_

#include "SerialConnECMA48.h"

class SerialConnXterm : public SerialConnECMA48 {
public:
    using Superclass = SerialConnECMA48;
    SerialConnXterm(std::shared_ptr<serial::Serial> serial);
    virtual ~SerialConnXterm();
    // public methods
    struct ScreenData {
        std::vector<VTLine> LinesBuffer_; // buffer
        std::vector<VTLine> Lines_;       // virtual screen
        std::vector<std::function<void()> > AttrFuncs_;
        CursorData CursorData_; // include 2 parts, position and attrs
    };
    void SaveScr(ScreenData& sd);
    void LoadScr(const ScreenData& sd);
protected:
    virtual int IsControlSeq(const std::string& seq);
    virtual void ProcessControlSeq(const std::string& seq, int seq_type);
    virtual void ProcessXtermTrivial(const std::string& seq);
    // with K_DELTA
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
private:
    // ECMA48Trivial, includes C1
    std::map<std::string, std::function<void()> > mXtermTrivialHandlers;
    void InstallXtermFunctions();
    // support alternative screen
    ScreenData mAltScr;
    ScreenData mBkgScr;
    bool mIsAltScr;
};

#endif
