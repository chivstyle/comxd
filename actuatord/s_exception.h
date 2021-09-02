#ifndef _actuatord_s_exception_h_
#define _actuatord_s_exception_h_

#include <exception>

class s_exception : public std::exception {
public:
	static const int kMaxTextSize = 512;
	s_exception(const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(mText, kMaxTextSize, fmt, va);
		va_end(va);
	}
	s_exception()
		: mText("")
	{
	}
	const char* what() const throw() { return mText; }
	
private:
	//
	char mText[kMaxTextSize];
};

#endif
