//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#include <ARMCPP11/ConditionVariable.h>

using namespace armcpp11;

ConditionVariable::ConditionVariable()
{
    pthread_cond_init(&mCondVar, NULL);
}

ConditionVariable::~ConditionVariable()
{
    NotifyAll();
    pthread_cond_destroy(&mCondVar);
}

void ConditionVariable::Wait(UniqueLock<Mutex> &lock)
{
    pthread_mutex_t *mutex = lock.Mutex().NativeHandle();

    pthread_cond_wait(&mCondVar, mutex);
}

void ConditionVariable::Notify()
{
    pthread_cond_signal(&mCondVar);
}

void ConditionVariable::NotifyAll()
{
    pthread_cond_broadcast(&mCondVar);
}
