//
// (c) 2020 chiv
//
#ifndef _terminal_VTOptionsDialog_h_
#define _terminal_VTOptionsDialog_h_

#include <CtrlLib/CtrlLib.h>

class VTOptionsDialog : public WithVTOptions<Upp::TopWindow> {
public:
    typedef VTOptionsDialog CLASSNAME;
    VTOptionsDialog();
    virtual ~VTOptionsDialog();
    // Set
    struct Options {
        Upp::Color PaperColor;
        Upp::Color TextsColor;
        Upp::Font Font;
        int LinesBufferSize;
    };
    Options GetOptions() const;
    void SetOptions(const Options& options);
    //
    static Upp::Font DefaultFont();
    //
protected:
    bool Key(Upp::dword key, int count);
    // Update font preview
    void PreviewFont();
    //
private:
    void InstallActions();
};

#endif
