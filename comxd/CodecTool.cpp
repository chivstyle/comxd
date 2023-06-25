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
            sprintf(hex_, "0x%02x", (unsigned char)b[k]);
        } else {
            sprintf(hex_, "0x%02x,", (unsigned char)b[k]);
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
    : mCodec(nullptr)
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
	mGenerate.WhenAction = [=]() { Generate(); };
	//
	CtrlLayout(*this);
	this->Sizeable().Title(t_("Codec Tool"));
	//
	mInput.SetFocus();
}

CodecTool::~CodecTool()
{
    delete mCodec;
}

Codec* CodecTool::RequestCodec()
{
    auto expect_name = ~mOutputEncoding;
    if (mCodec && mCodec->GetName() != expect_name) {
        delete mCodec;
        mCodec = nullptr;
    }
    if (!mCodec) {
        mCodec = CodecFactory::Inst()->CreateInst(expect_name);
    }
    return mCodec;
}

void CodecTool::Generate()
{
	std::string input = mInput.GetData().ToStd();
	int code = mOutputFmt.GetKey(mOutputFmt.GetIndex());
	
	Codec* codec = RequestCodec();
	if (codec) {
	    size_t ep;
		std::string bit = codec->TranscodeFromUTF8((const unsigned char*)input.data(), input.length(), &ep);
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
