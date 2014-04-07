#ifndef __RenderTarget_h_
#define __RenderTarget_h_

#include "RenderSystemPrerequisites.h"

enum eRenderTargetSize
{
  RS_SIZE_MANUAL,       // manual size specified m
  RS_SIZE_SCREEN_1_1,   // full screen size
  RS_SIZE_SCREEN_1_2,   // half screen size
  RS_SIZE_SCREEN_1_4,   // 1/4 screen size
  RS_SIZE_SCREEN_1_8,   // 1/8 screen size
  RS_SIZE_SCREEN_1_16,  // 1/16 screen size
};

enum eRenderTargetHints
{
  RTH_DEPTH_STENCIL           = 0x001,
  RTH_DEPTH_STENCIL_READONLY  = 0x002,
  RTH_ACCESS_UAV              = 0x004,
};

struct RenderTargetCreationInfo
{
  RenderTargetCreationInfo()
  {
    renderTargetSize = RS_SIZE_SCREEN_1_1;
    width = 0;
    height = 0;
    pixelFormat = PF_UNKNOWN;
    flags = 0;
  }

  eRenderTargetSize renderTargetSize;
  uint32 width;
  uint32 height;
  ePixelFormat pixelFormat;
  uint32 flags;
};

class RenderTarget
{
public:
  RenderTarget();
  ~RenderTarget();
  void create(const RenderTargetCreationInfo& info);
  void destroy();

  ID3D11Texture2D* getTexturePtr() const { return m_texturePtr; }
  ID3D11DepthStencilView* getDefaultDSV() const { return m_defaultDSV; }
  ID3D11DepthStencilView* getDefaultDSVReadOnly() const { return m_defaultDSVReadOnly; }
  ID3D11RenderTargetView* getDefaultRTV() const {return m_defaultRTV; }
  ID3D11ShaderResourceView* getDefaultSRV() const { return m_defaultSRV; }

private:
  ID3D11Texture2D* m_texturePtr;
  ID3D11ShaderResourceView* m_defaultSRV;
  ID3D11DepthStencilView* m_defaultDSV;
  ID3D11DepthStencilView* m_defaultDSVReadOnly;
  ID3D11RenderTargetView* m_defaultRTV;
};

#endif // __RenderTarget_h_