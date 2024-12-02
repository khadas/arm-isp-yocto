//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef __ATL_LIBRARY__
#define __ATL_LIBRARY__

#include "ATLError.h"
#include "ATLTypes.h"

namespace atl {

    typedef enum _ELoadLibraryMode {
        RTLD_LAZY = 1,
        RTLD_NOW  = 2,
        RTLD_GLOBAL =  4
    } ELoadLibraryMode ;

    class CATLLibrary  {
    public:
        static void* LoadSharedLibrary(const char *name, ELoadLibraryMode mode = RTLD_NOW ) ;
        static void* GetFunction(void *hLib, const char *Fnname) ;
        static basebool FreeSharedLibrary(void *hLib) ;
    };

} // atl namespace

#endif // __ATL_CONFIG_STRING__
