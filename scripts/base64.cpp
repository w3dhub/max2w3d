#include "General.h"
#include "base64.h"

//encode size is ((input_size - 1) / 3) * 4 + 4 plus 1 for \0 terminator
//decode size is (src_len / 4) * 3

#define GETBYTE32(x, n)    (*((uint8*)&(x)+n))
char const _pad = '=';
char const _encoder[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const uint8 _decoder[256] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
     52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
    255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
      7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
     19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
    255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
     37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255
};

int Base64_Encode(void const *source, int slen, void *dest, int dlen)
{
    //packet
    char *putp = (char *)dest;
    const uint8 *getp = (const uint8 *)source;
	int total = 0;
  
    if ( source != nullptr && slen > 0 && dest != nullptr && dlen > 0 ) {

        while ( slen > 0 ) {

            //to avoid buffer overrun
            if ( dlen < 4 ) {
                break;
            }
        
            int padding = 0;
            uint32 packedbytes = 0;

            GETBYTE32(packedbytes, 2) = *getp++;

            --slen;
        
            if ( slen ) {
                GETBYTE32(packedbytes, 1) = *getp++;
                --slen;
            } else {
                padding = 1;
            }
        
            if ( slen ) {
                GETBYTE32(packedbytes, 0) = *getp++;
                --slen;
            } else {
                padding++;
            }
        
            *putp++ = _encoder[(packedbytes << 8) >> 26];
            *putp++ = _encoder[(packedbytes << 14) >> 26];
            *putp++ = padding >= 2 ? _pad : _encoder[(packedbytes << 20) >> 26];
            *putp++ = padding >= 1 ? _pad : _encoder[packedbytes & 0x3F];
            dlen -= 4;
            total += 4;

        }

        //null terminate our cstring if buffer is longer than resulting data.
        if ( dlen > 0 ) {
            *putp = 0;
        }
    
    }

    return total;
}

int Base64_Decode(void const *source, int slen, void *dest, int dlen)
{
	const uint8 *getp = (const uint8 *)source;
    uint8 *putp = (uint8 *)dest;
    int total = 0;

    if ( source != nullptr && slen > 0 && dest != nullptr && dlen > 0 ) {
    
        while ( slen > 0 && dlen > 0) {

            //if ( dlen <= 0 )
            //	break;
        
            //this loop unpacks 4 chars into an int
            signed int blocksize = 0;
            uint32 packedbytes = 0;
            while ( slen > 0 && blocksize < 4) {

                char c = _decoder[*getp++];
                --slen;

                //nothing in the table we have is 254, so this would always be true?
                if ( c != -2 ) {

                    //this is an error case and should never be reached on valid input
                    if ( c == -1) {
                        slen = 0;
                        break;
                    }
                
                    //if ( blocksize <= 3 )
                        switch ( blocksize )
                        {
                            case 0:
                                packedbytes = (packedbytes & 0xFF03FFFF) | ((c & 0x3F) << 18);
                                break;
                            
                            case 1:
                                packedbytes = (packedbytes & 0xFFFC0FFF) | ((c & 0x3F) << 12);
                                break;
                            
                            case 2:
                                packedbytes = (packedbytes & 0xFFFFF03F) | ((c & 0x3F) << 6);
                                break;
                            
                            case 3:
                                packedbytes = (packedbytes & 0xFFFFFFC0) | (c & 0x3F);
                                break;
                            
                            default:
                                break;
                        }

                    ++blocksize;

                }
            }
        
       
            *putp++ = GETBYTE32(packedbytes, 2);
            ++total;
            --dlen;
        
            if ( dlen > 0 && blocksize > 2 ) {
                *putp++ = GETBYTE32(packedbytes, 1);
                --dlen;
                ++total;
            }
        
            if ( dlen > 0 && blocksize > 3 ) {
                *putp++ = GETBYTE32(packedbytes, 0);
                --dlen;
                ++total;
            }
        }
    }
    return total;
}
