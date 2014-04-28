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

struct IndexBufferCreationInfo
{
  uint32 size;
  bool use32BitIndices;
  bool dynamic;
  bool shadowed;
  void* data;
};

struct BufferInfo
{
  uint32 canRead : 1;
  uint32 canWrite : 1;
  uint32 mapped : 1;
  uint32 dynamic : 1;
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
  BufferInfo m_bufferInfo;
  eBufferMapOptions m_mapOptions;
};

class IndexBuffer
{
public:
  IndexBuffer();
  explicit IndexBuffer(const IndexBufferCreationInfo& info);
  ~IndexBuffer();

  ID3D11Buffer* getResourcePtr() { return m_resource;  }

  void create(const IndexBufferCreationInfo& info);
  void destroy();
  void* map(eBufferMapOptions mapOption);
  void unmap();

private:
  ID3D11Buffer* m_resource;
  void* m_backingStore;
  BufferInfo m_bufferInfo;
  eBufferMapOptions m_mapOptions;
};

#endif // __Buffer_h_