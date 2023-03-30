/*!
// (c) 2022 chiv
//
*/
#ifndef _comxd_CodecTool_h_
#define _comxd_CodecTool_h_

#include <CtrlLib/CtrlLib.h>

class Codec;
class CodecTool : public WithCodecTool<Upp::TopWindow> {
public:
	typedef CodecTool CLASSNAME;
	CodecTool();
	virtual ~CodecTool();
	//
	void Generate();
	//
	Codec* RequestCodec();
	//
private:
    Codec* mCodec;
};

#endif
