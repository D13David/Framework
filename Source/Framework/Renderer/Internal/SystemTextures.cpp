#include "Core.h"
#include "SystemTextures.h"
#include "Texture.h"

SharedPtr<Texture> SystemTextures::Default;
SharedPtr<Texture> SystemTextures::White;
SharedPtr<Texture> SystemTextures::Black;

void initCheckerboard(uint32 tileSize, uint32 textureWidth, uint32 textureHeight, uint32 color1, uint32 color2, uint32*& data)
{
  const uint32 tileDimension = tileSize * tileSize;

  uint32* ptr = 
    data = (uint32*)malloc(textureWidth * textureHeight * sizeof(uint32));

  uint32 colorLut[] =
  {
    color1, color2
  };

  for (uint32 y = 0; y < textureHeight; ++y)
  {
    for (uint32 x = 0; x < textureWidth; ++x)
    {
      uint32 tileIndexX = (x / tileSize) & 1;
      uint32 tileIndexY = (y / tileSize) & 1;
      *ptr++ = colorLut[tileIndexX ^ tileIndexY];
    }
  }
}

void SystemTextures::init()
{
  Default = new Texture();
  {
    uint32* checkerboard;
    initCheckerboard(16, 256, 256, 0x11000000, 0x11444444, checkerboard);

    TextureCreationInfo info;
    info.type = TT_TEXTURE_2D;
    info.width = 256;
    info.height = 256;
    info.flags = TF_USAGE_STATIC;
    info.data = checkerboard;
    info.format = PF_B8G8R8A8;
    Default->create(info);

    free(checkerboard);
  }

  White = new Texture();
  {
    uint32 value = 0x11111111;
    TextureCreationInfo info;
    info.type = TT_TEXTURE_2D;
    info.width = 1;
    info.height = 1;
    info.flags = TF_USAGE_STATIC;
    info.data = &value;
    info.format = PF_B8G8R8A8;
    White->create(info);
  }

  Black = new Texture();
  {
    uint32 value = 0;
    TextureCreationInfo info;
    info.type = TT_TEXTURE_2D;
    info.width = 1;
    info.height = 1;
    info.flags = TF_USAGE_STATIC;
    info.data = &value;
    info.format = PF_B8G8R8A8;
    Black->create(info);
  }
}