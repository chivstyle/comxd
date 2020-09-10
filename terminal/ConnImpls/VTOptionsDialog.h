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
        bool       TrackCaret;
        Upp::Color PaperColor;
        Upp::Color FontColor;
        Upp::Font  Font;
        int        LinesBufferSize;
    };
    Options GetOptions() const;
    void SetOptions(const Options& options);
    //
    static Upp::Font DefaultFont();
    //
protected:
    bool Accept();
    // Update font preview
    void PreviewFont();
    //
private:
    void InstallActions();
};

#endif
