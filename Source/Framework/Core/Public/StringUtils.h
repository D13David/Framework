#ifndef __StringUtils_h_
#define __StringUtils_h_

class StringUtils
{
public:
  static bool endsWith(const String& str1, const char* str2)
  {
    size_t len = strlen(str2);

    if (len > str1.length())
      return false;

    const char* p1 = str1.c_str() + str1.length()  - len;
    const char* p2 = str2;
    while (*p2)
    {
      if (*p1++ != *p2++)
        return false;
    }

    return true;
  }

  static String stripFileName(const String& str)
  {
    const char* begin = str.c_str();
    const char* p = begin + str.length() - 1;
    while (p >= begin && *p != '\\')
      --p;
    
    return String(begin, p - begin + 1);
  }

  static String stripDevice(const String& str)
  {
    const char* p = str.c_str();

    while (*p && *p != ':')
      ++p;

    return String(p, str.length() -  (p - str.c_str()));
  }
};

#endif // __StringUtils_h_