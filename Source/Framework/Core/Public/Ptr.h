#ifndef __Ptr_h_
#define __Ptr_h_

enum ePtrType
{
  PT_THREAD_SAFE,
  PT_FAST
};

long atomicIncrement(long volatile* value);
long atomicDecrement(long volatile* value);

template<typename T, ePtrType PtrType> class SharedPtr;

template<ePtrType PtrType>
class ReferenceCount
{
public:
  ReferenceCount()
    : m_refs(1)
    , m_weakRefs(1)
  {
  }

  void addReference()
  {
    if (PtrType == PT_THREAD_SAFE)
    {
      atomicIncrement(&m_refs);
    }
    else
    {
      ++m_refs;
    }
  }
  bool releaseReferences()
  {
    if (PtrType == PT_THREAD_SAFE)
    {
      if (atomicDecrement(&m_refs) == 0)
      {
        releaseWeakReference();
        return true;
      }
    }
    else
    {
      if (--m_refs == 0)
      {
        releaseWeakReference();
        return true;
      }
    }
    return false;
  }
  void addWeakReference()
  {
    if (PtrType == PT_THREAD_SAFE)
    {
      atomicIncrement(&m_weakRefs);
    }
    else
    {
      ++m_weakRefs;
    }
  }
  void releaseWeakReference()
  {
    if (PtrType == PT_THREAD_SAFE)
    {
      if (atomicDecrement(&m_weakRefs) == 0)
      {
        delete this;
      }
    }
    else
    {
      if (--m_weakRefs == 0)
      {
        delete this;
      }
    }
  }

  long getReferenceCount() const { return m_refs; }
  long getWeakReferenceCount() const { return m_weakRefs; }

private:
  //friend class SharedPtr<T, PtrType>;
  volatile long m_refs;
  volatile long m_weakRefs;
};

template<typename T, ePtrType PtrType = PT_FAST>
class SharedPtr
{
public:
  SharedPtr() 
    : m_object(0), m_refs(new ReferenceCount<PtrType>())
  {
  }
  SharedPtr(std::nullptr_t) 
    : m_object(0), m_refs(new ReferenceCount<PtrType>()) 
  {
  }
  SharedPtr(T* ptr)
    : m_object(ptr), m_refs(new ReferenceCount<PtrType>())
  {
  }
  template<typename O>
  SharedPtr(O* ptr)
    : m_object(ptr), m_refs(new ReferenceCount<PtrType>())
  {
  }
  SharedPtr(const SharedPtr<T, PtrType>& other)
    : m_object(other.m_object), m_refs(other.m_refs)
  {
    m_refs->addReference();
    //m_refs->addWeakReference();
  }

  template<typename O>
  SharedPtr(const SharedPtr<O, PtrType>& other)
    : m_object(other.m_object), m_refs(other.m_refs)
  {
    m_refs->addReference();
    //m_refs->addWeakReference();
  }

  ~SharedPtr()
  {
    m_refs->releaseReferences();
    //m_refs->releaseWeakReference();
  }

  SharedPtr<T, PtrType>& operator=(std::nullptr_t)
  {
    SharedPtr<T>(nullptr).swap(*this);
    return *this;
  }

  SharedPtr<T, PtrType>& operator=(const SharedPtr<T, PtrType>& other)
  {
    SharedPtr<T>(other).swap(*this);
    return *this;
  }

  template<typename O>
  SharedPtr<T, PtrType>& operator=(const SharedPtr<O, PtrType>& other)
  {
    SharedPtr<T, PtrType>(other).swap(*this);
    return *this;
  }

  operator bool() const
  {
    return m_object != 0;
  }

  bool operator!() const
  {
    return m_object == 0;
  }

  T& operator*() const
  {
    // ASSERT(m_refs->m_object, "deferencing nullptr");
    return *m_object;
  }

  T* operator->() const
  {
    // ASSERT(m_refs->m_object, "deferencing nullptr");
    return m_object;
  }

  T* get() const
  { 
    return m_object; 
  }

  long getReferenceCount() const 
  {
    return m_refs->getReferenceCount();
  }

  long getWeakReferenceCount() const 
  {
    return m_refs->getWeakReferenceCount();
  }

  void swap(SharedPtr<T, PtrType>& other)
  {
    std::swap(m_object, other.m_object);
    std::swap(m_refs, other.m_refs);
  }

private:
  SharedPtr(T* ptr, ReferenceCount<PtrType>* refs)
    : m_ptr(ptr), m_refs(refs)
  {
    m_refs->addReference();
  }

  template<typename O, ePtrType PtrType>
  friend class SharedPtr;

  template<typename O, ePtrType PtrType>
  friend class WeakPtr;

  template<typename U, typename V, ePtrType PtrType>
  friend SharedPtr<U, PtrType> staticCastSharedPtr(const SharedPtr<V>& sharedPtr);

  template<typename U, typename V, ePtrType PtrType>
  friend SharedPtr<U, PtrType> constCastSharedPtr(const SharedPtr<V>& sharedPtr);

  template<typename U, typename V, ePtrType PtrType>
  friend SharedPtr<U, PtrType> dynamicCastSharedPtr(const SharedPtr<V>& sharedPtr);

	ReferenceCount<PtrType>* m_refs;
  T* m_object;
};

template<typename T, ePtrType PtrType = PT_FAST>
class WeakPtr
{
public:
  WeakPtr() 
    : m_object(0), m_refs(new ReferenceCount<PtrType>())
  {
  }
  WeakPtr(std::nullptr_t)
    : m_object(0), m_refs(new ReferenceCount<PtrType>())
  {
  }
  WeakPtr(const WeakPtr<T, PtrType>& other)
    : m_object(other.m_object), m_refs(other.m_refs)
  {
    m_refs->addWeakReference();
  }

  template<typename O>
  WeakPtr(const WeakPtr<O, PtrType>& other)
    : m_object(other.m_object), m_refs(other.m_refs)
  {
    m_refs->addWeakReference();
  }

  template<typename O>
  WeakPtr(const SharedPtr<O, PtrType>& other)
    : m_object(other.m_object), m_refs(other.m_refs)
  {
    m_refs->addWeakReference();
  };

  WeakPtr<T, PtrType>& operator=(std::nullptr_t)
  {
    WeakPtr<T>(nullptr).swap(*this);
    return *this;
  }

  WeakPtr<T, PtrType>& operator=(const WeakPtr<T>& other)
  {
    WeakPtr<T>(other).swap(*this);
    return *this;
  }

  template<typename O>
  WeakPtr<T, PtrType>& operator=(const WeakPtr<O, PtrType>& other)
  {
    WeakPtr<T, PtrType>(other).swap(*this);
    return *this;
  }

  template<typename O>
  WeakPtr<T, PtrType>& operator=(const SharedPtr<O, PtrType>& other)
  {
    WeakPtr<T, PtrType>(other).swap(*this);
    return *this;
  }

  ~WeakPtr()
  {
    m_refs->releaseWeakReference();
  }

  operator bool() const
  {
    return get() != 0;
  }

  bool operator!() const
  {
    return get() == 0;
  }

  T& operator*() const
  {
    // ASSERT(m_refs->m_object, "deferencing nullptr");
    return *get();
  }

  T* operator->() const
  {
    // ASSERT(m_refs->m_object, "deferencing nullptr");
    return get();
  }

  T* get() const
  {
    if (m_refs->getReferenceCount() == 0)
      return 0;

    return m_object; 
  }

  long getReferenceCount() const 
  {
    return m_refs->getReferenceCount();
  }

  long getWeakReferenceCount() const 
  {
    return m_refs->getWeakReferenceCount();
  }

  void swap(WeakPtr<T, PtrType>& other)
  {
    std::swap(m_object, other.m_object);
    std::swap(m_refs, other.m_refs);
  }

private:
  WeakPtr(T* ptr, ReferenceCount<PtrType>* refs)
    : m_ptr(ptr), m_refs(refs)
  {
    m_refs->addWeakReference();
  }

  template<typename O, ePtrType PtrType>
  friend class SharedPtr;

  template<typename U, typename V, ePtrType PtrType>
  friend WeakPtr<U, PtrType> staticCastWeakPtr(const WeakPtr<V>& sharedPtr);

  template<typename U, typename V, ePtrType PtrType>
  friend WeakPtr<U, PtrType> dynamicCastWeakPtr(const WeakPtr<V>& sharedPtr);

  template<typename U, typename V, ePtrType PtrType>
  friend WeakPtr<U, PtrType> dynamicCastWeakPtr(const WeakPtr<V>& sharedPtr);

  ReferenceCount<PtrType>* m_refs;
  T* m_object;
};

template<typename U, typename V, ePtrType PtrType>
inline SharedPtr<U, PtrType> staticCastSharedPtr(const SharedPtr<V>& sharedPtr)
{
  if (sharedPtr == 0)
    return SharedPtr<U, PtrType>();

  return SharedPtr<U, PtrType>(
    static_cast<U*>(sharedPtr->m_object),
    sharedPtr->m_refs);
}

template<typename U, typename V, ePtrType PtrType>
inline SharedPtr<U, PtrType> constCastSharedPtr(const SharedPtr<V>& sharedPtr)
{
  if (sharedPtr == 0)
    return SharedPtr<U, PtrType>();

  return SharedPtr<U, PtrType>(
    const_cast<U*>(sharedPtr->m_object),
    sharedPtr->m_refs);
}

template<typename U, typename V, ePtrType PtrType>
inline SharedPtr<U, PtrType> dynamicCastSharedPtr(const SharedPtr<V>& sharedPtr)
{
  if (sharedPtr == 0)
    return SharedPtr<U, PtrType>();

  return SharedPtr<U, PtrType>(
    dynamic_cast<U*>(sharedPtr->m_object),
    sharedPtr->m_refs);
}

template<typename U, typename V, ePtrType PtrType>
inline WeakPtr<U, PtrType> staticCastWeakPtr(const WeakPtr<V>& sharedPtr)
{
  if (sharedPtr == 0)
    return WeakPtr<U, PtrType>();

  return WeakPtr<U, PtrType>(
    static_cast<U*>(sharedPtr->m_object),
    sharedPtr->m_refs);
}

template<typename U, typename V, ePtrType PtrType>
inline WeakPtr<U, PtrType> constCastWeakdPtr(const WeakPtr<V>& sharedPtr)
{
  if (sharedPtr == 0)
    return WeakPtr<U, PtrType>();

  return WeakPtr<U, PtrType>(
    const_cast<U*>(sharedPtr->m_object),
    sharedPtr->m_refs);
}

template<typename U, typename V, ePtrType PtrType>
inline WeakPtr<U, PtrType> dynamicCastWeakPtr(const WeakPtr<V>& sharedPtr)
{
  if (sharedPtr == 0)
    return WeakPtr<U, PtrType>();

  return WeakPtr<U, PtrType>(
    dynamic_cast<U*>(sharedPtr->m_object),
    sharedPtr->m_refs);
}

template<typename U, typename V, ePtrType PtrType>
inline bool operator==(const SharedPtr<U, PtrType>& a, const SharedPtr<V, PtrType>& b)
{
  return (a.get() == b.get());
}

template<typename U, typename V, ePtrType PtrType>
inline bool operator!=(const SharedPtr<U, PtrType>& a, const SharedPtr<V, PtrType>& b)
{
  return (a.get() != b.get());
}

//template<typename U, ePtrType PtrType>
//inline bool operator==(const SharedPtr<U, PtrType>& a, std::nullptr_t)
//{
//  return a.get() == 0;
//}

template<typename U, ePtrType PtrType>
inline bool operator==(std::nullptr_t, const SharedPtr<U, PtrType>& a)
{
  return a.get() == 0;
}

//template<typename U, ePtrType PtrType>
//inline bool operator!=(const SharedPtr<U, PtrType>& a, std::nullptr_t)
//{
//  return a.get() != 0;
//}

template<typename U, ePtrType PtrType>
inline bool operator!=(std::nullptr_t, const SharedPtr<U, PtrType>& a)
{
  return a.get() != 0;
}

template<typename U, typename V, ePtrType PtrType>
inline bool operator==(const WeakPtr<U, PtrType>& a, const WeakPtr<V, PtrType>& b)
{
  return (a.get() == b.get());
}

template<typename U, typename V, ePtrType PtrType>
inline bool operator!=(const WeakPtr<U, PtrType>& a, const WeakPtr<V, PtrType>& b)
{
  return (a.get() != b.get());
}

//template<typename U, ePtrType PtrType>
//inline bool operator==(const WeakPtr<U, PtrType>& a, std::nullptr_t)
//{
//  return a.get() == 0;
//}

template<typename U, ePtrType PtrType>
inline bool operator==(std::nullptr_t, const WeakPtr<U, PtrType>& a)
{
  return a.get() == 0;
}

//template<typename U, ePtrType PtrType>
//inline bool operator!=(const WeakPtr<U, PtrType>& a, std::nullptr_t)
//{
//  return a.get() != 0;
//}

template<typename U, ePtrType PtrType>
inline bool operator!=(std::nullptr_t, const WeakPtr<U, PtrType>& a)
{
  return a.get() != 0;
}

template<typename U, typename V, ePtrType PtrType>
inline bool operator==(const SharedPtr<U, PtrType>& a, const WeakPtr<V, PtrType>& b)
{
  return (a.get() == b.get());
}

template<typename U, typename V, ePtrType PtrType>
inline bool operator!=(const SharedPtr<U, PtrType>& a, const WeakPtr<V, PtrType>& b)
{
  return (a.get() != b.get());
}

template<typename U, typename V, ePtrType PtrType>
inline bool operator==(const WeakPtr<U, PtrType>& a, const SharedPtr<V, PtrType>& b)
{
  return (a.get() == b.get());
}

template<typename U, typename V, ePtrType PtrType>
inline bool operator!=(const WeakPtr<U, PtrType>& a, const SharedPtr<V, PtrType>& b)
{
  return (a.get() != b.get());
}

#endif // __Ptr_h_