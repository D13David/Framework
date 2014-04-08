#include "Core.h"
#include "ShaderDrawBundle.h"
#include "Shader.h"
#include "VertexDeclaration.h"
#include "RenderSystem.h"
#include "Game.h"
#include "Hash.h"

typedef std::map<long, SharedPtr<ShaderDrawBundle>> ShaderDrawBundleMap;
typedef std::vector<String> SemanticNameCache;

ShaderDrawBundleMap& getShaderDrawBundleMap()
{
  static ShaderDrawBundleMap shaderDrawBundleMap;
  return shaderDrawBundleMap;
}

SemanticNameCache& getSemanticNameCache()
{
  static SemanticNameCache semanticNameCache;
  static bool initialized = false;
  if (!initialized)
  {
    semanticNameCache.reserve(16);
    initialized = true;
  }
  return semanticNameCache;
}

ShaderDrawBundle::ShaderDrawBundle()
  : m_inputLayout(0)
{

}

ShaderDrawBundle::~ShaderDrawBundle()
{
  SAFE_RELEASE(m_inputLayout);
}

SharedPtr<ShaderDrawBundle> ShaderDrawBundle::createShaderDrawBundle(SharedPtr<VertexShader> vertexShader, SharedPtr<PixelShader> pixelShader, SharedPtr<const VertexDeclaration> vertexDeclaration)
{
  // FIXME: cache this array
  std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

  for (uint32 elementIdx = 0; ; elementIdx++)
  {
    const VertexElement* element = vertexDeclaration->getElement(elementIdx);
    if (!element)
      break;

    D3D11_INPUT_ELEMENT_DESC desc = {0};
    for (uint32 i = 0; i < getSemanticNameCache().size(); ++i)
    {
      if (element->semantic == getSemanticNameCache()[i])
      {
        desc.SemanticName = getSemanticNameCache()[i].c_str();
        break;
      }
    }
    if (!desc.SemanticName)
    {
      getSemanticNameCache().push_back(element->semantic);
      desc.SemanticName = getSemanticNameCache().back().c_str();
    }

    desc.SemanticIndex = element->semanticIndex;
    desc.AlignedByteOffset = element->byteOffset;
    desc.InputSlot = element->stream;
    if (element->usePerInstance)
    {
      desc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
      desc.InstanceDataStepRate = 1;
    }
    else
    {
      desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
      desc.InstanceDataStepRate = 0;
    }
    
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    switch (element->format)
    {
    case VEF_FLOAT1: format = DXGI_FORMAT_R32_FLOAT; break;
    case VEF_FLOAT2: format = DXGI_FORMAT_R32G32_FLOAT; break;
    case VEF_FLOAT3: format = DXGI_FORMAT_R32G32B32_FLOAT; break;
    case VEF_FLOAT4: format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
    case VEF_HALF1: format = DXGI_FORMAT_R16_FLOAT; break;
    case VEF_HALF2: format = DXGI_FORMAT_R16G16_FLOAT; break;
    case VEF_HALF4: format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
    case VEF_SHORT2: format = DXGI_FORMAT_R16G16_UINT; break;
    case VEF_SHORT4: format = DXGI_FORMAT_R16G16B16A16_UINT; break;
    case VEF_SHORT4N: format = DXGI_FORMAT_R16G16B16A16_UNORM; break;
    case VEF_COLOR: format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
    }

    desc.Format = format;

    inputElements.push_back(desc);
  }

  // FIXME: validate / patch vertex data

  long hash = crc32Hash((const ubyte*)&inputElements[0], inputElements.size() * sizeof(D3D11_INPUT_ELEMENT_DESC));
  hash ^= crc32Hash(vertexShader->getCode(), vertexShader->getCodeSize());

  SharedPtr<ShaderDrawBundle> shaderDrawBundle;
  ShaderDrawBundleMap::iterator it = getShaderDrawBundleMap().find(hash);
  if (it == getShaderDrawBundleMap().end())
  {
    ID3D11InputLayout* inputLayout;
    VALIDATE(RENDER_DEVICE->CreateInputLayout(&inputElements[0], inputElements.size(), vertexShader->getCode(), vertexShader->getCodeSize(), &inputLayout));
    
    shaderDrawBundle = new ShaderDrawBundle();
    shaderDrawBundle->m_inputLayout = inputLayout;
    shaderDrawBundle->m_pixelShader = pixelShader;
    shaderDrawBundle->m_vertexShader = vertexShader;
    getShaderDrawBundleMap().insert(std::make_pair(hash, shaderDrawBundle));
  }
  else
  {
    shaderDrawBundle = it->second;
  }

  return shaderDrawBundle;
}