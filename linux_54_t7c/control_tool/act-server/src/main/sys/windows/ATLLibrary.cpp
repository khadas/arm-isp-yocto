//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <ATL/ATLLibrary.h>
#include <ATL/ATLError.h>
#include <ATL/ATLTypes.h>

#include <windows.h>

namespace atl {

    void* CATLLibrary::LoadSharedLibrary(const char *name, ELoadLibraryMode mode) {
       return (void*)LoadLibrary(name);
    }

    void* CATLLibrary::GetFunction(void *hLib, const char *Fnname) {
         return (void*)GetProcAddress((HINSTANCE)hLib,Fnname);
    }

    basebool CATLLibrary::FreeSharedLibrary(void *hLib) {
        return TRUE == FreeLibrary((HINSTANCE)hLib); // returns non-zero on success
    }

} // atl namespace

