//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <ATL/ATLLibrary.h>

#include <dlfcn.h>

namespace atl {

    void* CATLLibrary::LoadSharedLibrary(const char *name, ELoadLibraryMode mode) {
       return dlopen(name, mode ) ;
    }

    void* CATLLibrary::GetFunction(void *hLib, const char *Fnname) {
         return dlsym(hLib, Fnname ) ;
    }

    basebool CATLLibrary::FreeSharedLibrary(void *hLib) {
        return ( 0 == dlclose( hLib ) ) ; // returns 0 on success
    }

} // atl namespace

