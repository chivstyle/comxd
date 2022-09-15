/*!
// (c) 2022 chiv
//
*/
#include "resource.h"
#include "CodecTool.h"
#include "CodecFactory.h"

using namespace Upp;

// output fmts
static struct OutputFormat {
	enum FormatCode {
		FC_C,
		FC_ESCAPE_SEQS
	};
	String Name;
	int    Code;
} kOutputFormats[] = {
	{ t_("C/C++"), OutputFormat::FC_C },
	{ t_("Escape seqs"), OutputFormat::FC_ESCAPE_SEQS }
};
static inline std::string ToHexString_(const std::string& b, size_t linesz = 16)
{
    std::string out;
    for (size_t k = 0; k < b.length(); ++k) {
        char hex_[8];
        if (k + 1 == b.size()) {
            sprintf(hex_, "%02x", (unsigned char)b[k]);
        } else {
            sprintf(hex_, "%02x,", (unsigned char)b[k]);
        }
        out += hex_;
        if (((k+1) % linesz) == 0 && (k+1 != b.size())) {
            out += '\n';
        }
    }
    return out;
}
static inline std::string ToEscapeString_(const std::string& b, size_t linesz = 16)
{
    std::string out;
    for (size_t k = 0; k < b.length(); ++k) {
        char hex_[8];
        sprintf(hex_, "\\x%02x", (unsigned char)b[k]);
        out += hex_;
    }
    return out;
}
static std::string Generate_C(const std::string& input)
{
	return ToHexString_(input, 16);
}
static std::string Generate_EscapeSeqs(const std::string& input)
{
	return ToEscapeString_(input, 16);
}

CodecTool::CodecTool()
{
	mInput.SetFrame(FieldFrame());
    mOutput.SetFrame(FieldFrame());
	// init ui
	auto codecs = CodecFactory::Inst()->GetSupportedCodecNames();
	for (size_t k = 0; k < codecs.size(); ++k) {
		mOutputEncoding.Add(codecs[k]);
	}
	mOutputEncoding.SetIndex(0);
	for (size_t k = 0; k < sizeof(kOutputFormats) / sizeof(kOutputFormats[0]); ++k) {
		mOutputFmt.Add(kOutputFormats[k].Code, kOutputFormats[k].Name);
	}
	mOutputFmt.SetIndex(0);
	//
	mGenerate.WhenAction = [=]() { Genereate(); };
	//
	CtrlLayout(*this);
	this->Sizeable().Title(t_("Codec Tool"));
	//
	mInput.SetFocus();
}

void CodecTool::Genereate()
{
	std::string input = mInput.GetData().ToStd();
	int code = mOutputFmt.GetKey(mOutputFmt.GetIndex());
	Codec* codec = CodecFactory::Inst()->CreateInst((String)mOutputEncoding.GetData());
	if (codec) {
		std::string bit = codec->TranscodeFromUTF8((const unsigned char*)input.data(), input.length());
		switch (code) {
		case OutputFormat::FC_C:
			mOutput.SetData(Generate_C(bit));
			break;
		case OutputFormat::FC_ESCAPE_SEQS:
			mOutput.SetData(Generate_EscapeSeqs(bit));
			break;
		default:break;
		}
	}
}
