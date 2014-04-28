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

  static String toString(uint32 value)
  {
    char buffer[512];
    _itoa_s(value, buffer, 10);
    return String(buffer);
  }
};

class PathUtils
{
public:
  static String getExtension(const String& path);
  static String getFilename(const String& path, bool stripExtension);
  static String getPath(const String& path, bool stripDevice);
};

#endif // __StringUtils_h_