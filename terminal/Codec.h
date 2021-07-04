//
// (c) 2020 chiv
//
#ifndef _terminal_Codec_h_
#define _terminal_Codec_h_

#include <cstdint>
#include <string>
#include <vector>
// UTF
// UTF8, UTF32
// Helper routines
static inline std::string UTF32ToUTF8_(const uint32_t& cp)
{
    std::string out;
    if (cp <= 0x7f)
        out.push_back((char)cp); // 7 bits
    else if (cp >= 0x80 && cp <= 0x7ff) { // 11 bits, 5, 6
        out.push_back((char)(0xc0 | (cp >> 6))); // high 5 bits
        out.push_back((char)(0x80 | (cp & 0x3f))); // low 6bits
    } else if (cp >= 0x800 && cp <= 0xd7ff || cp >= 0xe000 && cp <= 0xffff) { // 16 bits, 4,6,6
        out.push_back((char)(0xe0 | (cp >> 12)));
        out.push_back((char)(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back((char)(0x80 | (cp & 0x3f)));
    } else if (cp >= 0x10000 && cp <= 0x10ffff) { // 21 bits, 3,6,6,6
        out.push_back((char)(0xf0 | (cp >> 18)));
        out.push_back((char)(0x80 | ((cp >> 12) & 0x3f)));
        out.push_back((char)(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back((char)(0x80 | (cp & 0x3f)));
    } else {
        out.push_back('?'); // not supported code point
    }
    return out;
}
// UTF-8 to UTF-32, use ? for invalid UTF-8 seq.
static inline std::vector<uint32_t> UTF8ToUTF32_(const unsigned char* seq, size_t sz, size_t& ep)
{
    std::vector<uint32_t> out;
    size_t p = 0;
    // walk along
    while (p < sz) {
        int flag = seq[p] & 0xf0;
        if (flag == 0xf0) { // 4 bytes
            if (sz - p < 4)
                break;
            uint32_t bits = (seq[p] & 0x07) << 18; // 3+6+6+6=21bits
            // check and check
            if ((seq[p + 1] & 0xc0) != 0x80 || (seq[p + 2] & 0xc0) != 0x80 || (seq[p + 3] & 0xc0) != 0x80) {
                //out.push_back('?');
                out.push_back(seq[p]);
                p++;
            } else {
                bits |= (seq[p + 1] & 0x3f) << 12;
                bits |= (seq[p + 2] & 0x3f) << 6;
                bits |= (seq[p + 3] & 0x3f);
                out.push_back(bits);
                p += 4;
            }
        } else if ((flag & 0xe0) == 0xe0) { // 3 bytes, 4+6+6=16bits
            if (sz - p < 3)
                break;
            if ((seq[p + 1] & 0xc0) != 0x80 || (seq[p + 2] & 0xc0) != 0x80) {
                //out.push_back('?');
                out.push_back(seq[p]);
                p++;
            } else {
                uint32_t bits = (seq[p] & 0x0f) << 12;
                bits |= (seq[p + 1] & 0x3f) << 6;
                bits |= (seq[p + 2] & 0x3f);
                out.push_back(bits);
                p += 3;
            }
        } else if ((flag & 0xc0) == 0xc0) { // 2 bytes, 5+6 = 11bits
            if (sz - p < 2)
                break;
            if ((seq[p + 1] & 0xc0) != 0x80) {
                //out.push_back('?');
                out.push_back(seq[p]);
                p++;
            } else {
                uint32_t bits = (seq[p] & 0x1f) << 6;
                bits |= (seq[p + 1] & 0x3f);
                out.push_back(bits);
                p += 2;
            }
        } else if ((flag & 0x80) == 0x80) { // invalid
            //out.push_back('?');
            out.push_back(seq[p]);
            p++;
        } else {
            out.push_back(seq[p]);
            p++;
        }
    }
    // store the ep
    ep = p;
    //
    return out;
}
// Text codec
class Codec {
public:
    Codec() { }
    virtual ~Codec() { }
    //
    const std::string& GetName() const { return mName; }
    void SetName(const char* name) { mName = name; }
    // Raw to UTF-32
    virtual std::vector<uint32_t> TranscodeToUTF32(const unsigned char* data, size_t sz, size_t& ep) = 0;
    // Raw to UTF-8
    virtual std::string TranscodeToUTF8(const unsigned char* data, size_t sz) = 0;
    // UTF-32 To this
    virtual std::string TranscodeFromUTF32(const uint32_t* data, size_t sz) = 0;
    inline std::string TranscodeFromUTF32(const uint32_t& cc)
    {
        return TranscodeFromUTF32(&cc, 1);
    }
    //
    virtual std::string TranscodeFromUTF8(const unsigned char* data, size_t sz) = 0;
    //
private:
    std::string mName;
};

#endif
