#include "Core.h"

DataBlob::DataBlob()
  : m_data(0)
  , m_size(0)
{
}

DataBlob::~DataBlob()
{
  free();
}

void DataBlob::allocate(int size)
{
  free();
  if (size > 0)
  {
    m_data = malloc(size);
    m_size = size;
  }
}

void DataBlob::free()
{
  ::free(m_data);
  m_size = 0;
}

long fileSize(FILE* fp)
{
  long size = 0;
  if (fp)
  {
    long pos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, pos, SEEK_SET);
  }

  return size;
}

String getCurrentDirectory()
{
  String result;
  char path[MAX_PATH] = {0};
  if (GetCurrentDirectory(MAX_PATH, path) != 0)
  {
    result = path;
  }

  return result;
}

void setCurrentDirectory(const String& path)
{
  SetCurrentDirectory(path.c_str());
}

bool readRawBlob(const String& fileName, DataBlob& data)
{
  bool result = false;

  FILE* fp;
  fopen_s(&fp, fileName.c_str(), "rb");
  if (fp)
  {
    long size = fileSize(fp);
    if (size > 0)
    {
      data.allocate(size);
      result = (fread(data.getPtr(), 1, size, fp) == size);
    }

    fclose(fp);
  }

  return result;
}

bool readAllFile(const String& fileName, String& result)
{
  DataBlob data;
  if (readRawBlob(fileName, data))
  {
    result = String((String::value_type*)data.getPtr(),
      (String::value_type*)((ubyte*)data.getPtr() + data.getSize()));

    return true;
  }

  return false;
}

uint16 readUInt16(const ubyte*& ptr)
{
  ubyte b1 = *ptr++;
  ubyte b2 = *ptr++;
  return (b2 << 8) | b1;
}

uint32 readUInt32(const ubyte*& ptr)
{
  ubyte b1 = *ptr++;
  ubyte b2 = *ptr++;
  ubyte b3 = *ptr++;
  ubyte b4 = *ptr++;
  return (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
}