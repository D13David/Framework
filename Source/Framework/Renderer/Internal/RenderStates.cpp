#include "Core.h"
#include "RenderStates.h"
#include "Game.h"
#include "RenderSystem.h"

D3D11_FILL_MODE mapFillMode(eFillMode fillMode)
{
  switch (fillMode)
  {
  case FM_FILL_SOLID:
    return D3D11_FILL_SOLID;
  case FM_FILL_WIREFRAME:
    return D3D11_FILL_WIREFRAME;
  }

  return D3D11_FILL_SOLID;
}

D3D11_CULL_MODE mapCullMode(eCullMode cullMode)
{
  switch (cullMode)
  {
  case CM_CULL_NONE:
    return D3D11_CULL_NONE;
  case CM_CULL_FRONT:
    return D3D11_CULL_FRONT;
  case CM_CULL_BACK:
    return D3D11_CULL_BACK;
  }

  return D3D11_CULL_BACK;
}

D3D11_FILTER mapSampleFilter(eSampleFilter filter, bool useSampleCompare, uint32 maxAnisotropyHint)
{
  switch (filter)
  {
  case SF_FILTER_POINT:
    return useSampleCompare ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_POINT;
  case SF_FILTER_BILINEAR:
    return useSampleCompare ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
  case SF_FILTER_TRILINEAR:
    return useSampleCompare ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  case SF_FILTER_ANISOTROPIC_POINT:
  case SF_FILTER_ANISOTROPIC_BILINEAR:
    {
      if (maxAnisotropyHint == 1)
      {
        return useSampleCompare ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
      }

      return useSampleCompare ? D3D11_FILTER_COMPARISON_ANISOTROPIC : D3D11_FILTER_ANISOTROPIC;
    }
  }

  return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
}

D3D11_TEXTURE_ADDRESS_MODE mapSampleAddress(eSampleAddress address)
{
  switch (address)
  {
  case SA_ADDRESS_WRAP:
    return D3D11_TEXTURE_ADDRESS_WRAP;
  case SA_ADDRESS_MIRROR:
    return D3D11_TEXTURE_ADDRESS_MIRROR;
  case SA_ADDRESS_CLAMP:
    return D3D11_TEXTURE_ADDRESS_CLAMP;
  case SA_ADDRESS_BORDER:
    return D3D11_TEXTURE_ADDRESS_BORDER;
  case SA_ADDRESS_MIRROR_ONCE:
    return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
  }

  return D3D11_TEXTURE_ADDRESS_WRAP;
}

D3D11_COMPARISON_FUNC mapComparisonFunc(eComparisonFunc cmpFunc)
{
  switch (cmpFunc)
  {
  case CF_COMPARISON_NEVER:
    return D3D11_COMPARISON_NEVER;
  case CF_COMPARISON_LESS:
    return D3D11_COMPARISON_LESS;
  case CF_COMPARISON_EQUAL:
    return D3D11_COMPARISON_EQUAL;
  case CF_COMPARISON_LESS_EQUAL:
    return D3D11_COMPARISON_LESS_EQUAL;
  case CF_COMPARISON_GREATER:
    return D3D11_COMPARISON_GREATER;
  case CF_COMPARISON_NOT_EQUAL:
    return D3D11_COMPARISON_NOT_EQUAL;
  case CF_COMPARISON_GREATER_EQUAL:
    return D3D11_COMPARISON_GREATER_EQUAL;
  case CF_COMPARISON_ALWAYS:
    return D3D11_COMPARISON_ALWAYS;
  }

  return D3D11_COMPARISON_NEVER;
}

D3D11_STENCIL_OP mapStencilOp(eStencilOp stencilOp)
{
  switch (stencilOp)
  {
  case SO_STENCIL_OP_KEEP:
    return D3D11_STENCIL_OP_KEEP;
  case SO_STENCIL_OP_ZERO:
    return D3D11_STENCIL_OP_ZERO;
  case SO_STENCIL_OP_REPLACE:
    return D3D11_STENCIL_OP_REPLACE;
  case SO_STENCIL_OP_INCR_SAT:
    return D3D11_STENCIL_OP_INCR_SAT;
  case SO_STENCIL_OP_DECR_SAT:
    return D3D11_STENCIL_OP_DECR_SAT;
  case SO_STENCIL_OP_INVERT:
    return D3D11_STENCIL_OP_INVERT;
  case SO_STENCIL_OP_INCR:
    return D3D11_STENCIL_OP_INCR;
  case SO_STENCIL_OP_DECR:
    return D3D11_STENCIL_OP_DECR;
  }

  return D3D11_STENCIL_OP_KEEP;
}

ID3D11RasterizerState* createRasterizerState(eFillMode fillMode, eCullMode cullMode, 
  bool frontCounterClockwise, bool depthClipEnable, bool scissorEnable, bool multisampleEnable)
{
  CD3D11_RASTERIZER_DESC desc;
  desc.FillMode = mapFillMode(fillMode);
  desc.CullMode = mapCullMode(cullMode);
  desc.FrontCounterClockwise = frontCounterClockwise ? TRUE : FALSE;
  desc.DepthClipEnable = depthClipEnable;
  desc.ScissorEnable = scissorEnable ? TRUE : FALSE;
  desc.MultisampleEnable = multisampleEnable ? TRUE : FALSE;

  ID3D11RasterizerState* result = 0;
  VALIDATE(RENDER_DEVICE->CreateRasterizerState(&desc, &result));
  return result;
}

ID3D11SamplerState* createSamplerState(eSampleFilter filter, eSampleAddress addressU, eSampleAddress addressV, eSampleAddress addressW,
  uint32 mipBias, uint32 maxAnisotropy, uint32 borderColor, eComparisonFunc cmpFunc)
{
  CD3D11_SAMPLER_DESC desc;
  desc.Filter = mapSampleFilter(filter, cmpFunc != CF_COMPARISON_NEVER, maxAnisotropy);
  desc.AddressU = mapSampleAddress(addressU);
  desc.AddressV = mapSampleAddress(addressV);
  desc.AddressW = mapSampleAddress(addressW);
  desc.MipLODBias = (float)mipBias;
  desc.MaxAnisotropy = maxAnisotropy;
  desc.BorderColor[0] = ((borderColor >> 24) & 0xff) / 255.0f;
  desc.BorderColor[1] = ((borderColor >> 16) & 0xff) / 255.0f;
  desc.BorderColor[2] = ((borderColor >> 8) & 0xff) / 255.0f;
  desc.BorderColor[3] = ((borderColor >> 0) & 0xff) / 255.0f;
  desc.ComparisonFunc = mapComparisonFunc(cmpFunc);

  ID3D11SamplerState* result = 0;
  VALIDATE(RENDER_DEVICE->CreateSamplerState(&desc, &result));
  return result;
}

ID3D11DepthStencilState* createDepthStencilState(bool depthEnabled, bool depthWriteEnabled, eComparisonFunc depthFunc, bool stencilEnabled, uint32 stencilReadMask, 
  uint32 stencilWriteMask, eStencilOp stencilFailOpFront, eStencilOp stencilDepthFailOpFront, eStencilOp stencilPassOpFront, eComparisonFunc stencilFuncFront,
  eStencilOp stencilFailOpBack, eStencilOp stencilDepthFailOpBack, eStencilOp stencilPassOpBack, eComparisonFunc stencilFuncBack)
{
  CD3D11_DEPTH_STENCIL_DESC desc;
  desc.DepthEnable = depthEnabled ? TRUE : FALSE;
  desc.DepthWriteMask = depthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
  desc.DepthFunc = mapComparisonFunc(depthFunc);
  desc.StencilEnable = stencilEnabled ? TRUE : FALSE;
  desc.StencilReadMask = stencilReadMask;
  desc.StencilWriteMask = stencilWriteMask;
  desc.FrontFace.StencilFailOp = mapStencilOp(stencilFailOpFront);
  desc.FrontFace.StencilDepthFailOp = mapStencilOp(stencilDepthFailOpFront);
  desc.FrontFace.StencilPassOp = mapStencilOp(stencilPassOpFront);
  desc.FrontFace.StencilFunc = mapComparisonFunc(stencilFuncFront);
  desc.BackFace.StencilFailOp = mapStencilOp(stencilFailOpBack);
  desc.BackFace.StencilDepthFailOp = mapStencilOp(stencilDepthFailOpBack);
  desc.BackFace.StencilPassOp = mapStencilOp(stencilPassOpBack);
  desc.BackFace.StencilFunc = mapComparisonFunc(stencilFuncBack);

  ID3D11DepthStencilState* result = 0;
  VALIDATE(RENDER_DEVICE->CreateDepthStencilState(&desc, &result));
  return result;
}