#ifndef __Buffer_h_
#define __Buffer_h_

enum eBufferMapOptions
{
  BUFFER_MAP_READ,
  BUFFER_MAP_WRITE,
  BUFFER_MAP_WRITE_DISCARD,
  BUFFER_MAP_WRITE_NO_OVERWRITE
};

enum eBufferAccessFlags
{
  BUFFER_ACCESS_CPU_READ    = 1,
  BUFFER_ACCESS_CPU_WRITE   = 2,
  BUFFER_ACCESS_GPU_READ    = 4,
  BUFFER_ACCESS_GPU_WRITE   = 8
};

struct VertexBufferCreationInfo
{
  uint32 size;
  uint32 accessFlags;
  bool shadowed;
  void* data;
};

class VertexBuffer
{
public:
  VertexBuffer();
  explicit VertexBuffer(const VertexBufferCreationInfo& info);
  ~VertexBuffer();

  ID3D11Buffer* getResourcePtr() { return m_resource;  }

  void create(const VertexBufferCreationInfo& info);
  void destroy();
  void* map(eBufferMapOptions mapOption);
  void unmap();

private:
  ID3D11Buffer* m_resource;
  void* m_backingStore;

  struct BufferInfo
  {
    uint32 canRead : 1;
    uint32 canWrite : 1;
    uint32 mapped : 1;
    uint32 dynamic : 1;
  };
  BufferInfo m_bufferInfo;
  eBufferMapOptions m_mapOptions;
};

#endif // __Buffer_h_