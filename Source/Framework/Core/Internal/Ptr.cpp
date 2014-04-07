#include "Core.h"
#include "Ptr.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

long atomicIncrement(long volatile* value)
{
  return InterlockedIncrement(value);
}

long atomicDecrement(long volatile* value)
{
  return InterlockedDecrement(value);
}