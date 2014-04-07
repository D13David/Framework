#include "Core.h"
#include "Buffer.h"
#include "Game.h"
#include "RenderSystem.h"

VertexBuffer::VertexBuffer()
  : m_resource(0)
  , m_backingStore(0)
{
  m_bufferInfo.canRead = false;
  m_bufferInfo.canWrite = false;
  m_bufferInfo.mapped = false;
  m_bufferInfo.dynamic = false;
}

VertexBuffer::VertexBuffer(const VertexBufferCreationInfo& info)
  : m_resource(0)
  , m_backingStore(0)
{
  m_bufferInfo.canRead = false;
  m_bufferInfo.canWrite = false;
  m_bufferInfo.mapped = false;
  m_bufferInfo.dynamic = false;
  create(info);
}

VertexBuffer::~VertexBuffer()
{
  destroy();
}

void VertexBuffer::create(const VertexBufferCreationInfo& info)
{
  if (info.size == 0)
    return;

  D3D11_USAGE usage = D3D11_USAGE_STAGING;
  uint32 accessFlags = 0;

  if (info.accessFlags & BUFFER_ACCESS_GPU_WRITE)
  {
    usage = D3D11_USAGE_DEFAULT;
  }
  else
  {
    if (info.accessFlags & BUFFER_ACCESS_CPU_WRITE)
    {
      usage = D3D11_USAGE_DYNAMIC;
      accessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
      if (info.data)
      {
        usage = D3D11_USAGE_IMMUTABLE;
      }
      else
      {
        usage = D3D11_USAGE_DEFAULT;
      }
    }
  }

  if (usage == D3D11_USAGE_STAGING)
  {
    if (info.accessFlags & BUFFER_ACCESS_CPU_READ)
    {
      accessFlags |= D3D11_CPU_ACCESS_READ;
    }
    if (info.accessFlags & BUFFER_ACCESS_CPU_WRITE)
    {
      accessFlags |= D3D11_CPU_ACCESS_WRITE;
    }
  }

  D3D11_BUFFER_DESC desc = {0};
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.ByteWidth = info.size;
  desc.CPUAccessFlags = accessFlags;
  desc.Usage = usage;

  D3D11_SUBRESOURCE_DATA data = {0};
  data.pSysMem = info.data;

  VALIDATE(RENDER_DEVICE->CreateBuffer(&desc, info.data ? &data : 0, &m_resource));

  if (info.shadowed)
  {
    m_backingStore = malloc(info.size);
    if (info.data)
      memcpy(m_backingStore, info.data, info.size);

    m_bufferInfo.canRead = true;
    m_bufferInfo.canWrite = usage != D3D11_USAGE_IMMUTABLE;
  }
  else
  {
    m_bufferInfo.canRead = accessFlags & D3D11_CPU_ACCESS_READ;
    m_bufferInfo.canWrite = accessFlags & D3D11_CPU_ACCESS_WRITE;
  }

  m_bufferInfo.dynamic = usage == D3D11_USAGE_DYNAMIC;
}

void VertexBuffer::destroy()
{
  SAFE_RELEASE(m_resource);
  if (m_backingStore)
  {
    free(m_backingStore);
    m_backingStore = 0;
  }
}

void* VertexBuffer::map(eBufferMapOptions mapOption)
{
  if (m_bufferInfo.mapped)
    return 0;

  D3D11_MAP map;
  switch (mapOption)
  {
  case BUFFER_MAP_READ: map = D3D11_MAP_READ; break;
  case BUFFER_MAP_WRITE: map = D3D11_MAP_WRITE; break;
  case BUFFER_MAP_WRITE_DISCARD: D3D11_MAP_WRITE_DISCARD; break;
  case BUFFER_MAP_WRITE_NO_OVERWRITE: D3D11_MAP_WRITE_NO_OVERWRITE; break;
  }

  // check for valid map options
  if ((map == D3D11_MAP_READ && !m_bufferInfo.canRead) ||
    (map != D3D11_MAP_READ && !m_bufferInfo.canWrite))
  {
    return 0;
  }

  void* result = 0;

  if (!m_bufferInfo.dynamic && m_backingStore)
  {
    result = m_backingStore;
  }
  else
  {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(RENDER_CONTEXT->Map(m_resource, 0, map, 0, &mappedResource)))
    {
      result = mappedResource.pData;
    }
  }
  
  m_bufferInfo.mapped = (result != 0);

  return result;
}

void VertexBuffer::unmap()
{
  if (!m_bufferInfo.mapped)
    return;

  
  if (!m_bufferInfo.dynamic && m_backingStore)
  {
    if (m_mapOptions != D3D11_MAP_READ)
      RENDER_CONTEXT->UpdateSubresource(m_resource, 0, NULL, m_backingStore, 0, 0);
  }
  else
  {
    RENDER_CONTEXT->Unmap(m_resource, 0);
  }

  m_bufferInfo.mapped = false;
}
