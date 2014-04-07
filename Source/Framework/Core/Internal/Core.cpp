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