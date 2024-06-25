#ifndef SCRIPTS_INCLUDE__BASE64_H
#define SCRIPTS_INCLUDE__BASE64_H

#define		BASE64_OK				0
#define		BASE64_ERROR			-1
#define		BASE64_BUFFER_OVERFLOW	-2

////////////////////////////////////////////////////////////////////////////////
//  Macros
////////////////////////////////////////////////////////////////////////////////
#define		BASE64_ENC_SIZE(x) (((x / 3) + 1) * 4 + 1)
#define		BASE64_DEC_SIZE(x) ((x /4) * 3)

//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//  Inline functions, should be used in place of macros.
////////////////////////////////////////////////////////////////////////////////
inline int Base64_Enc_Size(int src_length)
{
    return ((src_length + 2) / 3) * 4;
}

inline int Base64_Dec_Size(int src_length)
{
    int len = (src_length / 4) * 3;
    if ((src_length % 4) == 3) {
        len += 2;
    }
    if ((src_length % 4) == 2) {
        len += 1;
    }
    
    return len;
}

int Base64_Encode(void const *source, int srclength, void *dest, int destlength);
int Base64_Decode(void const *source, int srclength, void *dest, int destlength);

#endif

