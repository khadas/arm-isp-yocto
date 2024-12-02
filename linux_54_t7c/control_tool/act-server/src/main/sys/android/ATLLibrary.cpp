//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <ATL/ATLLibrary.h>

#include <stdexcept>

namespace atl {

    void* CATLLibrary::LoadSharedLibrary(const char *name, ELoadLibraryMode mode) {
       throw std::runtime_error("shared libraries are not supported in android");
    }

    void* CATLLibrary::GetFunction(void *hLib, const char *Fnname) {
         throw std::runtime_error("shared libraries are not supported in android");
    }

    basebool CATLLibrary::FreeSharedLibrary(void *hLib) {
        throw std::runtime_error("shared libraries are not supported in android");
    }

} // atl namespace

