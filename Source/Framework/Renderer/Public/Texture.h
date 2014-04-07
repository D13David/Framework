#ifndef __Texture_h_
#define __Texture_h_

#include "Resource.h"
#include "ResourceManager.h"

class Texture : public Resource
{
public:
  Texture();
  ~Texture();
  virtual bool load(const String& fileName);
  virtual void unload();

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