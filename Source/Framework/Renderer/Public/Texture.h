#ifndef __Texture_h_
#define __Texture_h_

#include "Resource.h"
#include "ResourceManager.h"
#include "RenderSystemPrerequisites.h"

enum eTextureType
{
  TT_TEXTURE_1D,
  TT_TEXTURE_2D,
  TT_TEXTURE_3D,
  TT_TEXTURE_CUBE
};

enum eTextureFlags
{
  // mutual exclusive flags
  TF_USAGE_DYNAMIC = 0x001,
  TF_USAGE_STAGING = 0x002,
  TF_USAGE_STATIC  = 0x003,

  TF_GPU_WRITE_ACCESS = 0x004,
};

struct TextureCreationInfo
{
  TextureCreationInfo()
  {
    type = TT_TEXTURE_2D;
    width = 1;
    height = 1;
    depth = 0;
    arraySize = 1;
    format = PF_B8G8R8A8;
    flags = 0;
    data = 0;
  }
  eTextureType type;
  uint32 width;
  uint32 height;
  uint32 depth;
  uint32 arraySize;
  ePixelFormat format;
  uint32 flags;
  void* data;
};

class Texture : public Resource
{
public:
  Texture();
  ~Texture();
  virtual bool load(const String& fileName);
  virtual void unload();
  void create(const TextureCreationInfo& info);

  ID3D11Texture2D* getTexture2DPtr() const { return (ID3D11Texture2D*)m_resource; }
  ID3D11ShaderResourceView* getDefaultSRV() const { return m_defaultSRV; }

private:
  bool loadTarga(const DataBlob& data);

  ID3D11Resource* m_resource;
  ID3D11ShaderResourceView* m_defaultSRV;
};

class TextureManager : public ResourceManager
{
protected:
  virtual Resource* createResource(const String& fileName);
};

#endif // __Texture_h_