#include "Core.h"
#include "RenderTarget.h"
#include "RenderSystem.h"
#include "Game.h"

DXGI_FORMAT mapPixelFormat(ePixelFormat format)
{
  switch (format)
  {
  case PF_B8G8R8A8: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
  case PF_R16G16: return DXGI_FORMAT_R16G16_UNORM;
  case PF_A2B10G10R10: return DXGI_FORMAT_R10G10B10A2_UNORM;
  case PF_A16B16G16R16: return DXGI_FORMAT_R16G16B16A16_UNORM;
  case PF_DEPTHSTENCIL: return DXGI_FORMAT_R24G8_TYPELESS;
  case PF_SHADOW_DEPTH: return DXGI_FORMAT_D16_UNORM;
  case PF_R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
  case PF_R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
  case PF_R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
  case PF_R11G11A10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
  case PF_R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
  case PF_R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
  case PF_R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
  case PF_R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
  case PF_BC1: return DXGI_FORMAT_BC1_TYPELESS;
  case PF_BC2: return DXGI_FORMAT_BC2_TYPELESS;
  case PF_BC3: return DXGI_FORMAT_BC3_TYPELESS;
  case PF_BC4: return DXGI_FORMAT_BC4_UNORM;
  }

  return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT findAppropriateDepthStencilFormat(DXGI_FORMAT format)
{
  switch (format)
  {
  case DXGI_FORMAT_R24G8_TYPELESS:
    return DXGI_FORMAT_D24_UNORM_S8_UINT;
  case DXGI_FORMAT_R32_TYPELESS:
    return DXGI_FORMAT_D32_FLOAT;
  case DXGI_FORMAT_R16_TYPELESS:
    return DXGI_FORMAT_D16_UNORM;
  }

  return format;
}

DXGI_FORMAT findAppropriateRenderTargetFormat(DXGI_FORMAT format, bool srgb = false)
{
  if (srgb)
  {
    switch (format)
    {
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
      return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
    case DXGI_FORMAT_BC1_TYPELESS:
      return DXGI_FORMAT_BC1_UNORM_SRGB;
    case DXGI_FORMAT_BC2_TYPELESS:
      return DXGI_FORMAT_BC2_UNORM_SRGB;
    case DXGI_FORMAT_BC3_TYPELESS:
      return DXGI_FORMAT_BC3_UNORM_SRGB;
    }
  }
  else
  {
    switch (format)
    {
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
      return DXGI_FORMAT_B8G8R8X8_UNORM;
    case DXGI_FORMAT_BC1_TYPELESS:
      return DXGI_FORMAT_BC1_UNORM;
    case DXGI_FORMAT_BC2_TYPELESS:
      return DXGI_FORMAT_BC2_UNORM;
    case DXGI_FORMAT_BC3_TYPELESS:
      return DXGI_FORMAT_BC3_UNORM;
    }
  }

  switch (format)
  {
  case DXGI_FORMAT_R24G8_TYPELESS:
    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  case DXGI_FORMAT_R32_TYPELESS:
    return DXGI_FORMAT_R32_FLOAT;
  case DXGI_FORMAT_R16_TYPELESS:
    return DXGI_FORMAT_R16_UNORM;
  }

  return format;
}

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
  desc.Format = mapPixelFormat(info.pixelFormat);
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