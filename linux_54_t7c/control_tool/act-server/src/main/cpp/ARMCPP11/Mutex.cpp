//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#include <ARMCPP11/Mutex.h>

using namespace armcpp11;

Mutex::Mutex()
{
    int rc = pthread_mutexattr_init(&mAttr);

    if (rc == 0) {
        rc = pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_ERRORCHECK);
    }
    if (rc == 0) {
        rc = pthread_mutex_init(&mMutex, &mAttr);
    }

    if (rc != 0) {
        char buffer[256];
        sprintf(buffer, "Mutex initialisation failed with error %d", rc);
        throw std::runtime_error(buffer);
    }
}

Mutex::~Mutex()
{
    pthread_mutexattr_destroy(&mAttr);
    pthread_mutex_destroy(&mMutex);
}

void Mutex::Lock()
{
    int rc = pthread_mutex_lock(&mMutex);
    if (rc != 0) {
        char buffer[256];
        sprintf(buffer, "Mutex initialisation failed with error %d", rc);
        throw std::runtime_error(buffer);
    }
}

bool Mutex::TryLock()
{
    int rc = pthread_mutex_trylock(&mMutex);

    return (rc == 0);
}

void Mutex::Unlock()
{
    int rc = pthread_mutex_unlock(&mMutex);

    if (rc != 0)
    {
        char buffer[256];
        sprintf(buffer, "Mutex initialisation failed with error %d", rc);
        throw std::runtime_error(buffer);
    }
}

Mutex::NativeHandleType Mutex::NativeHandle()
{
    return &mMutex;
}

//############################################################################//
//                             RECURSIVE MUTEX                                //
//############################################################################//
RecursiveMutex::RecursiveMutex()
{
    int rc = pthread_mutexattr_init(&mAttr);

    if (rc == 0) {
        rc = pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE);
    }

    if (rc == 0) {
        rc = pthread_mutexattr_setpshared(&mAttr, PTHREAD_PROCESS_PRIVATE);
    }
    if (rc == 0) {
        rc = pthread_mutex_init(&mMutex, &mAttr);
    }

    if (rc != 0) {
        char buffer[256];
        sprintf(buffer, "Mutex initialisation failed with error %d", rc);
        throw std::runtime_error(buffer);
    }
}

RecursiveMutex::~RecursiveMutex()
{
    pthread_mutexattr_destroy(&mAttr);
    pthread_mutex_destroy(&mMutex);
}

void RecursiveMutex::Lock()
{
    int rc = pthread_mutex_lock(&mMutex);
    if (rc != 0) {
        char buffer[256];
        sprintf(buffer, "Mutex initialisation failed with error %d", rc);
        throw std::runtime_error(buffer);
    }
}

bool RecursiveMutex::TryLock()
{
    int rc = pthread_mutex_trylock(&mMutex);

    return (rc == 0);
}

void RecursiveMutex::Unlock()
{
    int rc = pthread_mutex_unlock(&mMutex);

    if (rc != 0)
    {
        char buffer[256];
        sprintf(buffer, "Mutex initialisation failed with error %d", rc);
        throw std::runtime_error(buffer);
    }
}

RecursiveMutex::NativeHandleType RecursiveMutex::NativeHandle()
{
    return &mMutex;
}
