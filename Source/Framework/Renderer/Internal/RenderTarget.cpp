#include "Core.h"
#include "RenderTarget.h"
#include "RenderSystem.h"
#include "Game.h"
#include "RendererUtils.h"

RenderTarget::RenderTarget()
  : m_texturePtr(0)
  , m_defaultDSV(0)
  , m_defaultDSVReadOnly(0)
  , m_defaultRTV(0)
  , m_defaultSRV(0)
{

}

RenderTarget::~RenderTarget()
{
  destroy();
}

void RenderTarget::create(const RenderTargetCreationInfo& info)
{
  D3D11_TEXTURE2D_DESC desc = {0};

  if (info.renderTargetSize == RS_SIZE_MANUAL)
  {
    desc.Width = info.width;
    desc.Height = info.height;
  }
  else
  {
    uint32 screenWidth = g_Game->getViewportWidth();
    uint32 screenHeight = g_Game->getViewportHeight();

    switch (info.renderTargetSize)
    {
    case RS_SIZE_SCREEN_1_1:
      desc.Width = screenWidth;
      desc.Height = screenHeight;
      break;
    case RS_SIZE_SCREEN_1_2:
      desc.Width = screenWidth / 2;
      desc.Height = screenHeight / 2;
      break;
    case RS_SIZE_SCREEN_1_4:
      desc.Width = screenWidth / 4;
      desc.Height = screenHeight / 4;
      break;
    case RS_SIZE_SCREEN_1_8:
      desc.Width = screenWidth / 8;
      desc.Height = screenHeight / 8;
      break;
    case RS_SIZE_SCREEN_1_16:
      desc.Width = screenWidth / 16;
      desc.Height = screenHeight / 16;
      break;
    }
  }

  destroy();

  desc.MipLevels = 0;
  desc.ArraySize = 1;
  desc.Format = mapPixelFormatForRendertarget(info.pixelFormat);
  desc.SampleDesc.Quality = 0;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  if (info.flags & RTH_DEPTH_STENCIL)
    desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
  else
    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

  if (info.flags & RTH_ACCESS_UAV)
    desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

  VALIDATE(RENDER_DEVICE->CreateTexture2D(&desc, 0, &m_texturePtr));

  if (info.flags & RTH_DEPTH_STENCIL)
  {
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = findAppropriateDepthStencilFormat(desc.Format);
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = 0;
    dsvDesc.Texture2D.MipSlice = 0;
    VALIDATE(RENDER_DEVICE->CreateDepthStencilView(m_texturePtr, &dsvDesc, &m_defaultDSV));

    // create additional readonly depth stencil view to gain access both for shader input reading and depthstencil tests
    if (info.flags & RTH_DEPTH_STENCIL_READONLY)
    {
      D3D11_DEPTH_STENCIL_VIEW_DESC dsvReadOnlyDesc;
      dsvReadOnlyDesc.Format = dsvDesc.Format;
      dsvReadOnlyDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
      dsvReadOnlyDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
      dsvReadOnlyDesc.Texture2D.MipSlice = 0;
      VALIDATE(RENDER_DEVICE->CreateDepthStencilView(m_texturePtr, &dsvReadOnlyDesc, &m_defaultDSVReadOnly));
    }
  }
  else
  {
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = findAppropriateRenderTargetFormat(desc.Format);
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    VALIDATE(RENDER_DEVICE->CreateRenderTargetView(m_texturePtr, &rtvDesc, &m_defaultRTV));
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  srvDesc.Format = findAppropriateRenderTargetFormat(desc.Format);
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  srvDesc.Texture2D.MostDetailedMip = 0;
  VALIDATE(RENDER_DEVICE->CreateShaderResourceView(m_texturePtr, &srvDesc, &m_defaultSRV));
}

void RenderTarget::destroy()
{
  SAFE_RELEASE(m_texturePtr);
  SAFE_RELEASE(m_defaultDSV);
  SAFE_RELEASE(m_defaultDSVReadOnly);
  SAFE_RELEASE(m_defaultRTV);
  SAFE_RELEASE(m_defaultSRV);
}