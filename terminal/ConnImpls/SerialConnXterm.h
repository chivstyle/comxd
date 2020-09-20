//
// (c) 2020 chiv
//
#ifndef _comxd_SerialConnXterm_h_
#define _comxd_SerialConnXterm_h_

#include "SerialConnECMA48.h"

class SerialConnXterm : public SerialConnECMA48 {
public:
    using Superclass = SerialConnECMA48;
    SerialConnXterm(std::shared_ptr<SerialIo> serial);
    virtual ~SerialConnXterm();
    // public methods
    struct ScreenData {
        std::vector<VTLine> LinesBuffer; // buffer
        std::vector<VTLine> Lines;       // virtual screen
        std::vector<std::function<void()> > AttrFuncs;
        int Vx, Vy;
        Upp::Font Font;
        Upp::Color FgColor, BgColor;
        bool Blink;
        struct SelectionSpan SelSpan;
    };
    void SaveScr(ScreenData& sd);
    void LoadScr(const ScreenData& sd);
protected:
    virtual int IsControlSeq(const std::string& seq);
    virtual void ProcessControlSeq(const std::string& seq, int seq_type);
    virtual void ProcessXtermTrivial(const std::string& seq);
    //
    virtual void ProcessDA(const std::string& seq);
    // with K_DELTA
    virtual bool ProcessKeyDown(Upp::dword key, Upp::dword flags);
private:
    // ECMA48Trivial, includes C1
    std::map<std::string, std::function<void()> > mXtermTrivialHandlers;
    std::map<int, std::function<void(const std::string& seq)> > mXtermFuncs;
    void InstallXtermFunctions();
    // support alternative screen
    ScreenData mAltScr;
    ScreenData mBkgScr;
    bool mIsAltScr;
};

#endif
