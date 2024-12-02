//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <ATL/ATLPlatform.h>

#ifdef _WIN32
#include <string.h>
#include <stdio.h>
const char * __strerror(int errnum, char *buf, size_t buflen) {
    strerror_s(buf, buflen, errnum);
    return buf;
}
int __vsnprintf(char* buffer, size_t buf_size, const char* format, va_list vlist) {
    int res = vsnprintf_s(buffer, buf_size, buf_size, format, vlist);
    return res;
}
#endif
