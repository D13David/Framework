#ifndef __RenderStates_h_
#define __RenderStates_h_

#include "RenderSystemPrerequisites.h"

ID3D11RasterizerState* createRasterizerState(eFillMode fillMode, eCullMode cullMode, 
  bool frontCounterClockwise, bool depthClipEnable, bool scissorEnable, bool multisampleEnable);

ID3D11SamplerState* createSamplerState(eSampleFilter filter, eSampleAddress addressU, eSampleAddress addressV, eSampleAddress addressW,
  uint32 mipBias, uint32 maxAnisotropy, uint32 borderColor, eComparisonFunc cmpFunc);

ID3D11DepthStencilState* createDepthStencilState(bool depthEnabled, bool depthWriteEnabled, eComparisonFunc depthFunc, bool stencilEnabled, uint32 stencilReadMask, 
  uint32 stencilWriteMask, eStencilOp stencilFailOpFront, eStencilOp stencilDepthFailOpFront, eStencilOp stencilPassOpFront, eComparisonFunc stencilFuncFront,
  eStencilOp stencilFailOpBack, eStencilOp stencilDepthFailOpBack, eStencilOp stencilPassOpBack, eComparisonFunc stencilFuncBack);

template<typename Derived, typename RefType>
class StaticRenderState
{
public:
  static RefType* get() 
  {
    static StateResource resource;
    return resource.m_resource;
  }

private:
  struct StateResource
  {
    RefType* m_resource;

    StateResource()
      : m_resource(0)
    {  
      m_resource = Derived::create();
    }
    ~StateResource()
    {
      if (m_resource)
      {
        m_resource->Release();
        m_resource = 0;
      }
    }
  };
};

template<eFillMode FillMode = FM_FILL_SOLID,
  eCullMode CullMode = CM_CULL_FRONT,
  bool FrontCounterClockwise = true,
  bool DepthClipEnable = true,
  bool ScissorEnable = false,
  bool MultisampleEnable = false>
class RasterizerState 
  : public StaticRenderState<RasterizerState<FillMode, CullMode, FrontCounterClockwise, DepthClipEnable, ScissorEnable, MultisampleEnable>, ID3D11RasterizerState>
{
public:
  static ID3D11RasterizerState* create()
  {
    return createRasterizerState(FillMode, CullMode, FrontCounterClockwise, DepthClipEnable, ScissorEnable, MultisampleEnable);
  }
};

template<eSampleFilter Filter = SF_FILTER_BILINEAR,
  eSampleAddress AddressU = SA_ADDRESS_CLAMP,
  eSampleAddress AddressV = SA_ADDRESS_CLAMP,
  eSampleAddress AddressW = SA_ADDRESS_CLAMP,
  uint32 MipBias = 0,
  uint32 MaxAnisotropy = 1,
  uint32 BorderColor = 0xffffffff,
  eComparisonFunc CmpFunc = CF_COMPARISON_NEVER>
class SamplerState
  : public StaticRenderState<SamplerState<Filter, AddressU, AddressV, AddressW, MipBias, MaxAnisotropy, BorderColor, CmpFunc>, ID3D11SamplerState>
{
public:
  static ID3D11SamplerState* create()
  {
    return createSamplerState(Filter, AddressU, AddressV, AddressW, MipBias, MaxAnisotropy, BorderColor,  CmpFunc);
  }
};

template<bool DepthEnabled = true,
  bool DepthWriteEnabled = true,
  eComparisonFunc DepthFunc = CF_COMPARISON_LESS,
  bool StencilEnabled = false,
  uint32 StencilReadMask = 0xff,
  uint32 StencilWriteMask = 0xff,
  eStencilOp StencilFailOpFront = SO_STENCIL_OP_KEEP,
  eStencilOp StencilDepthFailOpFront = SO_STENCIL_OP_KEEP,
  eStencilOp StencilPassOpFront = SO_STENCIL_OP_KEEP,
  eComparisonFunc StencilFuncFront = CF_COMPARISON_ALWAYS,
  eStencilOp StencilFailOpBack = SO_STENCIL_OP_KEEP,
  eStencilOp StencilDepthFailOpBack = SO_STENCIL_OP_KEEP,
  eStencilOp StencilPassOpBack = SO_STENCIL_OP_KEEP,
  eComparisonFunc StencilFuncBack = CF_COMPARISON_ALWAYS>
class DepthStencilState
  : public StaticRenderState<DepthStencilState<DepthEnabled, DepthWriteEnabled, DepthFunc, StencilEnabled, StencilReadMask, StencilWriteMask, StencilFailOpFront, StencilDepthFailOpFront,
      StencilPassOpFront, StencilFuncFront, StencilFailOpBack, StencilDepthFailOpBack, StencilPassOpBack, StencilFuncBack>, ID3D11DepthStencilState>
{
public:
  static ID3D11DepthStencilState* create()
  {
    return createDepthStencilState(DepthEnabled, DepthWriteEnabled, DepthFunc, StencilEnabled, StencilReadMask, StencilWriteMask, StencilFailOpFront, StencilDepthFailOpFront,
      StencilPassOpFront, StencilFuncFront, StencilFailOpBack, StencilDepthFailOpBack, StencilPassOpBack, StencilFuncBack);
  }
};

#endif // __RenderStates_h_