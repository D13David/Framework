#include "Core.h"
#include "Texture.h"
#include "StringUtils.h"
#include "Game.h"
#include "RenderSystem.h"

#define ORIGX_RIGHT 0x08
#define ORIGY_BOTTOM 0x10

enum eTargaImageType
{
  IMAGE_NONE           = 0,
  IMAGE_UNCOMP_INDEXED = 1,
  IMAGE_UNCOMP_RGB     = 2,
  IMAGE_UNCOMP_MONO    = 3,
  IMAGE_RLE_INDEXED    = 4,
  IMAGE_RLE_RGB        = 5,
  IMAGE_RLE_MONO       = 6
};

#pragma pack(push, 1)
struct tgaHeader
{
  ubyte   imgIdLen;
  ubyte   paletteType;
  ubyte   imageType;
  uint16  paletteOffset;
  uint16  paletteLen;
  ubyte   paletteBits;
  uint16  originX;
  uint16  originY;
  uint16  width;
  uint16  height;
  ubyte   bitsPerPixel;
  ubyte   flags;
};
#pragma pack(pop)

Texture::Texture()
  : m_resource(0)
  , m_defaultSRV(0)
{
}

Texture::~Texture()
{
  SAFE_RELEASE(m_resource);
  SAFE_RELEASE(m_defaultSRV);
}

bool Texture::load(const String& fileName)
{
  bool result = false;

  DataBlob data;
  if (!readRawBlob(fileName, data))
    return false;

  if (StringUtils::endsWith(fileName, ".tga"))
    result = loadTarga(data);

  return result;
}

void Texture::unload()
{
}

bool Texture::loadTarga(const DataBlob& data)
{
  const ubyte* p = (const ubyte*)data.getPtr();

  tgaHeader* header = (tgaHeader*)p;

  // validate image data
  if (header->imageType != IMAGE_UNCOMP_RGB || header->bitsPerPixel < 24)
    return false;
  
  p += sizeof(tgaHeader) 
    + header->imgIdLen 
    + header->paletteLen;

  uint32 srcLineWidth = header->width * header->bitsPerPixel / 8;
  ubyte* imageData = (ubyte*)malloc(header->width * header->height * sizeof(uint32));
  ubyte* dest = imageData;

  if (header->bitsPerPixel == 24)
  {
    for (uint32 h = 0; h < header->height; ++h)
    {
      for (uint32 w = 0; w < header->width; ++w)
      {
        dest[2] = *p++;
        dest[1] = *p++;
        dest[0] = *p++;
        dest[3] = 0;
        dest += 4;
      }
    }
  }
  else if (header->bitsPerPixel == 32)
  {
    for (uint32 h = 0; h < header->height; ++h)
    {
      for (uint32 w = 0; w < header->width; ++w)
      {
        dest[2] = *p++;
        dest[1] = *p++;
        dest[0] = *p++;
        dest[3] = *p++;
        dest += 4;
      }
    }
  }

  // need to swap vertically
  if (header->originX == 0)
  {
    srcLineWidth = header->width * sizeof(uint32);

    ubyte* buffer = (ubyte*)malloc(srcLineWidth);
    for (uint16 i = 0; i < header->height / 2; ++i)
    {
      ubyte* src = imageData + i * srcLineWidth;
      ubyte* dst = imageData + ((header->height-1) - i) * srcLineWidth;
      memcpy(buffer, src, srcLineWidth);
      memcpy(src, dst, srcLineWidth);
      memcpy(dst, buffer, srcLineWidth);
    }

    free(buffer);
  }

  // calculate number of mip levels
  /*uint32 mipLevels = 1;
  uint32 width = header->width;
  uint32 height = header->height;
  while (width > 1 || height > 1)
  {
    width /= 2;
    height /= 2;

    if (width == 0)
      width = 1;
    if (height == 0)
      height = 1;

    ++mipLevels;
  }*/

  D3D11_TEXTURE2D_DESC desc = {0};
  desc.Width = header->width;
  desc.Height = header->height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA subresourceData = {0};
  subresourceData.pSysMem = imageData;
  subresourceData.SysMemPitch = desc.Width * sizeof(uint32);

  ID3D11Texture2D* resource;
  VALIDATE(RENDER_DEVICE->CreateTexture2D(&desc, &subresourceData, &resource));

  CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  srvDesc.Texture2D.MipLevels = 1;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  VALIDATE(RENDER_DEVICE->CreateShaderResourceView(resource, &srvDesc, &m_defaultSRV));

  m_resource = resource;

  return true;
}

Resource* TextureManager::createResource(const String& fileName)
{
  Texture* texture = new Texture();
  if (!texture->load(fileName))
  {
    delete texture;
    return 0;
  }

  return texture;
}