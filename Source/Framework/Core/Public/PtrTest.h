#ifndef __PtrTest_h_
#define __PtrTest_h_

#include "Ptr.h"

namespace PtrTest
{

#define RUN_TEST(test) { \
    if ((test) == false) \
      printf("WARNING: " #test " failed...\n"); \
    else \
      printf(#test " succeeded...\n"); }

  template<ePtrType Type>
  static void TestSharedPointer()
  {
    printf("\nStarting TestSharedPtr Tests...\n");

    printf("Test 1\n");
    {
      SharedPtr<int, Type> ptr;
      SharedPtr<int, Type> ptr1;

      ptr1 = nullptr;

      RUN_TEST(ptr.get() == 0);
      RUN_TEST(ptr.getReferenceCount() == 1);
      RUN_TEST(ptr == 0);

      RUN_TEST(ptr1.get() == 0);
      RUN_TEST(ptr1.getReferenceCount() == 1);
      RUN_TEST(ptr1 == 0);
    }

    printf("\nTest 2\n");
    {
      SharedPtr<int, Type> ptr(new int(42));

      RUN_TEST(ptr.get() != 0);
      RUN_TEST(ptr.getReferenceCount() == 1);
      RUN_TEST(ptr != 0);
      RUN_TEST(*ptr == 42);
    }

    printf("\nTest 3\n");
    {
      SharedPtr<int, Type> ptr = new int(42);

      {
        SharedPtr<int, Type> ptr1(ptr);

        RUN_TEST(ptr.get() != 0);
        RUN_TEST(ptr.getReferenceCount() == 2);
        RUN_TEST(ptr1.get() != 0);
        RUN_TEST(ptr1.getReferenceCount() == 2);
        RUN_TEST(*ptr == 42);
        *ptr1 = 44;
        RUN_TEST(*ptr1 == 44);
        RUN_TEST(*ptr == 44);
      }

      RUN_TEST(ptr.getReferenceCount() == 1);
      RUN_TEST(ptr != 0);

      {
        SharedPtr<int, Type> ptr1 = ptr;

        RUN_TEST(ptr.get() != 0);
        RUN_TEST(ptr.getReferenceCount() == 2);
        RUN_TEST(ptr1.get() != 0);
        RUN_TEST(ptr1.getReferenceCount() == 2);
        RUN_TEST(*ptr == 44);
        *ptr1 = 50;
        RUN_TEST(*ptr1 == 50);
        RUN_TEST(*ptr == 50);
      }

      RUN_TEST(ptr.getReferenceCount() == 1);
      RUN_TEST(ptr != 0);
    }

    printf("\nTest 4\n");
    {
      struct foo
      {
        int value;
      };

      SharedPtr<foo, Type> ptr = new foo;
      ptr->value = 10;
      RUN_TEST(ptr->value == 10);
      RUN_TEST((*ptr).value == 10);

      SharedPtr<foo, Type> ptr1 = ptr;
      RUN_TEST(ptr->value == 10);
      RUN_TEST((*ptr).value == 10);
    }

    printf("\nTest 5\n");
    {
      struct foo
      {
        virtual const String getName() const { return "foo"; }
      };
      struct bar : foo
      {
        const String getName() const { return "bar"; }
      };

      {
        SharedPtr<foo, Type> ptr = new bar;
        RUN_TEST(ptr->getName() == "bar");
      }

      {
        SharedPtr<bar, Type> ptr = new bar;
        SharedPtr<foo, Type> ptr1 = ptr;
        RUN_TEST(ptr->getName() == "bar");
        RUN_TEST(ptr1->getName() == "bar");
      }
    }

    printf("\nTest 6\n");
    {
      SharedPtr<int, Type> ptr = new int(10);
      *ptr += 100;
      RUN_TEST(*ptr == 110);
      RUN_TEST(ptr.getReferenceCount() == 1);
      RUN_TEST(ptr.getWeakReferenceCount() == 1);
      {
        WeakPtr<int, Type> ptr1 = ptr;
        RUN_TEST(*ptr1 == 110);
        RUN_TEST(ptr1.getReferenceCount() == 1);
        RUN_TEST(ptr1.getWeakReferenceCount() == 2);
        *ptr1 = 5;
        RUN_TEST(*ptr == 5);
        RUN_TEST(*ptr1 == 5);
      }
      RUN_TEST(ptr.getReferenceCount() == 1);
      RUN_TEST(ptr.getWeakReferenceCount() == 1);

      WeakPtr<int, Type> ptr2;
      {
        SharedPtr<int, Type> ptr1 = new int(200);
        ptr2 = ptr1;
        RUN_TEST(ptr1.getReferenceCount() == 1);
        RUN_TEST(ptr1.getWeakReferenceCount() == 2);
      }
      RUN_TEST(ptr2.getReferenceCount() == 0);
      RUN_TEST(ptr2.getWeakReferenceCount() == 1);
      RUN_TEST(ptr2 == 0);
    }
  }

#undef RUN_TEST

}

#endif // __PtrTest_h_