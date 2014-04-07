#include "Core.h"
#include "StringUtils.h"

String PathUtils::getExtension(const String& path)
{
  String::size_type pos = path.find_last_of(".");
  if (pos != String::npos)
  {
    return path.substr(pos+1);
  }
  
  return String();
}

String PathUtils::getFilename(const String& path, bool stripExtension)
{
  String fileName = path;
  String::size_type pos = fileName.find_last_of("/\\");
  if (pos != String::npos)
  {
    fileName = path.substr(pos+1);
  }

  if (stripExtension)
  {
    fileName = fileName.substr(0, fileName.find_last_of("."));
  }

  return fileName;
}

String PathUtils::getPath(const String& path, bool stripDevice)
{
  String fileName;
  String::size_type pos = path.find_last_of("/\\");
  if (pos != String::npos)
  {
    fileName = path.substr(0, pos);
  }

  if (stripDevice)
  {
    pos = fileName.find_first_of("/\\");
    if (pos != String::npos)
      fileName = fileName.substr(pos+1);
  }

  return fileName;
}