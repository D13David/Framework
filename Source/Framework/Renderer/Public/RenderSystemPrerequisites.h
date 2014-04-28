#ifndef __RenderSystemPrerequisites_h_
#define __RenderSystemPrerequisites_h_

enum eCullMode
{
	CM_CULL_NONE,
	CM_CULL_FRONT,
	CM_CULL_BACK
};

enum eFillMode
{
	FM_FILL_SOLID,
	FM_FILL_WIREFRAME
};

enum eSampleAddress
{
  SA_ADDRESS_WRAP,
  SA_ADDRESS_MIRROR,
  SA_ADDRESS_CLAMP,
  SA_ADDRESS_BORDER,
  SA_ADDRESS_MIRROR_ONCE
};

enum eComparisonFunc
{
  CF_COMPARISON_NEVER,
  CF_COMPARISON_LESS,
  CF_COMPARISON_EQUAL,
  CF_COMPARISON_LESS_EQUAL,
  CF_COMPARISON_GREATER,
  CF_COMPARISON_NOT_EQUAL,
  CF_COMPARISON_GREATER_EQUAL,
  CF_COMPARISON_ALWAYS
};

enum eStencilOp
{
  SO_STENCIL_OP_KEEP,
  SO_STENCIL_OP_ZERO,
  SO_STENCIL_OP_REPLACE,
  SO_STENCIL_OP_INCR_SAT,
  SO_STENCIL_OP_DECR_SAT,
  SO_STENCIL_OP_INVERT,
  SO_STENCIL_OP_INCR,
  SO_STENCIL_OP_DECR
};

enum eSampleFilter
{
  SF_FILTER_POINT,
  SF_FILTER_BILINEAR,
  SF_FILTER_TRILINEAR,
  SF_FILTER_ANISOTROPIC_POINT,
  SF_FILTER_ANISOTROPIC_BILINEAR
};

enum ePixelFormat
{
  PF_UNKNOWN = 0,

  PF_B8G8R8A8,
  PF_R16G16,
  PF_A2B10G10R10,
  PF_A16B16G16R16,

  // depth
  PF_DEPTHSTENCIL,
  PF_SHADOW_DEPTH,

  // floating point
  PF_R32G32B32A32_FLOAT,  // 128 bit
  PF_R32G32B32_FLOAT,     //  96 bit
  PF_R16G16B16A16_FLOAT,  //  64 bit
  PF_R11G11A10_FLOAT,     //  32 bit
  PF_R16G16_FLOAT,        //  32 bit
  PF_R32G32_FLOAT,        //  64 bit
  PF_R16_FLOAT,           //  16 bit
  PF_R32_FLOAT,           //  32 bit

  // compressed
  PF_BC1,                 // dxt1
  PF_BC2,                 // dxt3
  PF_BC3,                 // dxt5
  PF_BC4,                 // ati1
};

enum eRenderTargets
{
	RT_RENDER_TARGET0 = 0x0001,
	RT_RENDER_TARGET1 = 0x0002,
	RT_RENDER_TARGET2 = 0x0004,
	RT_RENDER_TARGET3 = 0x0008,
	RT_RENDER_TARGET4 = 0x0010,
	RT_RENDER_TARGET5 = 0x0020,
	RT_RENDER_TARGET6 = 0x0040,
	RT_RENDER_TARGET7 = 0x0080,
	RT_DEPTH          = 0x0100,
	RT_STENCIL        = 0x0200,
	RT_ALL_TARGETS    = 0xffff
};

enum ePrimitiveType
{
  PT_LINELIST,
  PT_POINTLIST,
  PT_TRIANGLELIST,
  PT_TRIANGLESTRIP
};

enum eVertexElementFormat
{
  VEF_NONE,
  // 32 bit float formats
  VEF_FLOAT1,
  VEF_FLOAT2,
  VEF_FLOAT3,
  VEF_FLOAT4,
  // 16 bit float formats
  VEF_HALF1,
  VEF_HALF2,
  VEF_HALF4,
  // 16 bit integer formats
  VEF_SHORT2,
  VEF_SHORT4,
  VEF_SHORT4N,
  // special formats
  VEF_COLOR,

  VEF_MAX
};

#define MAX_RENDER_TARGETS 16

// project forward declarations
struct InitParams;
class TextureManager;

// d3d forward declaration
struct ID3D11RasterizerState;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11Buffer;

#endif // __RenderSystemPrerequisites_h_