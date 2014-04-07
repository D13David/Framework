#include "Core.h"
#include "Shader.h"
#include "Game.h"
#include "RenderSystem.h"

Shader::Shader()
  : m_maxSlot(0)
{
  memset(m_cbuffers, 0, sizeof(m_cbuffers));
}

Shader::~Shader()
{
  destroy();
}

void Shader::init(const DataBlob& data)
{
  if (!_createShaderResource(data.getPtr(), data.getSize()))
    return;

  ID3D11ShaderReflection* reflector = 0;
  HRESULT hr = D3DReflect(data.getPtr(), data.getSize(), IID_ID3D11ShaderReflection, (void**)&reflector);
  if (SUCCEEDED(hr))
  {
    D3D11_SHADER_DESC shaderDesc;
    reflector->GetDesc(&shaderDesc);

    m_parameters.clear();
    m_inputSignature.clear();

    // initialize shader parameter lists
    for (uint32 i = 0; i < shaderDesc.ConstantBuffers; ++i)
    {
      D3D11_SHADER_BUFFER_DESC bufferDesc;
      ID3D11ShaderReflectionConstantBuffer* cbuffer = reflector->GetConstantBufferByIndex(i);
      cbuffer->GetDesc(&bufferDesc);

      if (bufferDesc.Type != D3D11_CT_CBUFFER)
        continue;

      _allocateCBuffer(i, bufferDesc.Size);

      for (uint32 paramIdx = 0; paramIdx < bufferDesc.Variables; ++paramIdx)
      {
        D3D11_SHADER_VARIABLE_DESC paramDesc;
        ID3D11ShaderReflectionVariable* param = cbuffer->GetVariableByIndex(paramIdx);
        param->GetDesc(&paramDesc);

        ShaderParameter shaderParameter;
        shaderParameter.name = paramDesc.Name;
        shaderParameter.byteOffset = paramDesc.StartOffset;
        shaderParameter.bufferIndex = i;
        m_parameters.push_back(shaderParameter);
      }
    }

    // create input layout (if needed)
    for (uint32 i = 0; i < shaderDesc.InputParameters; ++i)
    {
      D3D11_SIGNATURE_PARAMETER_DESC inputParamDesc;
      reflector->GetInputParameterDesc(i, &inputParamDesc);

      if (strncmp(inputParamDesc.SemanticName, "SV_", 3) == 0)
        continue;

      ShaderInputParameter inputParameter;
      inputParameter.semantic = inputParamDesc.SemanticName;
      inputParameter.semanticIndex = inputParamDesc.SemanticIndex;

      switch (inputParamDesc.ComponentType)
      {
      case D3D_REGISTER_COMPONENT_FLOAT32:
        inputParameter.type = INPUT_PARAM_FLOAT;
        break;
      case D3D_REGISTER_COMPONENT_UINT32:
        inputParameter.type = INPUT_PARAM_UINT;
        break;
      case D3D_REGISTER_COMPONENT_SINT32:
        inputParameter.type = INPUT_PARAM_INT;
        break;
      }

#define MASK_1111 0xff
#define MASK_0111 0x07
#define MASK_0011 0x03
#define MASK_0001 0x01

      if ((inputParamDesc.Mask & MASK_1111) == MASK_1111)
        inputParameter.componentsUsed = 4;
      else if ((inputParamDesc.Mask & MASK_0111) == MASK_0111)
        inputParameter.componentsUsed = 3;
      else if ((inputParamDesc.Mask & MASK_0011) == MASK_0011)
        inputParameter.componentsUsed = 2;
      else if ((inputParamDesc.Mask & MASK_0001) == MASK_0001)
        inputParameter.componentsUsed = 1;

#undef MASK_1111
#undef MASK_0111
#undef MASK_0011
#undef MASK_0001

      m_inputSignature.push_back(inputParameter);

    }

    reflector->Release();
  }
}

void Shader::destroy()
{
  for (uint32 i = 0; i < MAX_CONSTANT_BUFFERS; ++i)
    _destroyCBuffer(i);
}

void Shader::bindToPipeline()
{
  _bind(m_buffersToBind, m_maxSlot);
}

void Shader::setParamByName(const String& name, float value)
{
  float* ptr = (float*)_getParameterPointer(name);
  ASSERT(ptr, "parameter not found");
  *ptr = value;
}

void Shader::setParamByName(const String& name, float value1, float value2)
{
  float* ptr = (float*)_getParameterPointer(name);
  ASSERT(ptr, "parameter not found");
  *ptr++ = value1;
  *ptr++ = value2;
}

void Shader::setParamByName(const String& name, float value1, float value2, float value3)
{
  float* ptr = (float*)_getParameterPointer(name);
  ASSERT(ptr, "parameter not found");
  *ptr++ = value1;
  *ptr++ = value2;
  *ptr++ = value3;
}

void Shader::setParamByName(const String& name, float value1, float value2, float value3, float value4)
{
  float* ptr = (float*)_getParameterPointer(name);
  ASSERT(ptr, "parameter not found");
  *ptr++ = value1;
  *ptr++ = value2;
  *ptr++ = value3;
  *ptr++ = value4;
}

void Shader::beginUpdateParameters()
{
  memset(m_buffersToBind, 0, sizeof(m_buffersToBind));
  m_maxSlot = 0;
}

void Shader::endUpdateParameters()
{
  WeakPtr<RenderSystem> renderSys = g_Game->getRenderSystem();

  for (uint32 i = 0; i < MAX_CONSTANT_BUFFERS; ++i)
  {
    if (!m_cbuffers[i].dirty)
      continue;

    D3D11_MAPPED_SUBRESOURCE mappedSubRes;
    if (SUCCEEDED(renderSys->getDeviceContextPtr()->Map(m_cbuffers[i].buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubRes)))
    {
      memcpy(mappedSubRes.pData, m_cbuffers[i].backingStore, m_cbuffers[i].size);
      renderSys->getDeviceContextPtr()->Unmap(m_cbuffers[i].buffer, 0);
    }

    m_buffersToBind[i] = m_cbuffers[i].buffer;
    ++m_maxSlot;
  }
}

void Shader::setParamByName(const String& name, const float* value, uint32 size)
{
  float* ptr = (float*)_getParameterPointer(name);
  ASSERT(ptr, "parameter not found");
  memcpy(ptr, value, size);
}

void Shader::_allocateCBuffer(uint32 index, uint32 size)
{
  ASSERT(index < MAX_CONSTANT_BUFFERS, "index out of range");
  _destroyCBuffer(index);
  if (size > 0)
  {
    size = (size + 15) & ~15;

    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth = size;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.Usage = D3D11_USAGE_DYNAMIC;

    WeakPtr<RenderSystem> renderSys = g_Game->getRenderSystem();
    VALIDATE(renderSys->getDevicePtr()->CreateBuffer(&desc, NULL, &m_cbuffers[index].buffer));

    m_cbuffers[index].backingStore = (ubyte*)malloc(size);
    m_cbuffers[index].size = size;
    m_cbuffers[index].dirty = false;
  }
}

void Shader::_destroyCBuffer(uint32 index)
{
  ASSERT(index < MAX_CONSTANT_BUFFERS, "index out of range");
  cbuffer& ptr = m_cbuffers[index];
  if (ptr.backingStore)
  {
    free(ptr.backingStore);
    ptr.backingStore = 0;
    SAFE_RELEASE(ptr.buffer);
    ptr.dirty = false;
  }
}

ubyte* Shader::_getParameterPointer(const String& name)
{
  ShaderParameter* param = 0;

  for (ShaderParameterArray::iterator it = m_parameters.begin();
    it != m_parameters.end(); ++it)
  {
    if (it->name == name)
    {
      param = &*it;
      break;
    }
  }

  ubyte* result = 0;
  if (param)
  {
    cbuffer& buffer = m_cbuffers[param->bufferIndex];
    result = buffer.backingStore + param->byteOffset;
    buffer.dirty = true;
  }

  return result;
}

VertexShader::VertexShader()
{
}

VertexShader::~VertexShader()
{
  SAFE_RELEASE(m_resource);
}

bool VertexShader::_createShaderResource(const void* byteCode, uint32 byteCodeLen)
{
  WeakPtr<RenderSystem> renderSys = g_Game->getRenderSystem();
  VALIDATE(renderSys->getDevicePtr()->CreateVertexShader(byteCode, byteCodeLen, NULL, &m_resource));

  if (byteCode)
  {
    m_code.allocate(byteCodeLen);
    memcpy(m_code.getPtr(), byteCode, byteCodeLen);
  }

  return true;
}

void VertexShader::_bind(ID3D11Buffer* const* buffers, uint32 numBuffers)
{
  WeakPtr<RenderSystem> renderSys = g_Game->getRenderSystem();
  if (numBuffers > 0)
  {
    renderSys->getDeviceContextPtr()->VSSetConstantBuffers(0, numBuffers, buffers);
  }

  renderSys->getDeviceContextPtr()->VSSetShader(m_resource, NULL, 0);
}

PixelShader::PixelShader()
{
}

PixelShader::~PixelShader()
{
  SAFE_RELEASE(m_resource);
}

bool PixelShader::_createShaderResource(const void* byteCode, uint32 byteCodeLen)
{
  WeakPtr<RenderSystem> renderSys = g_Game->getRenderSystem();
  VALIDATE(renderSys->getDevicePtr()->CreatePixelShader(byteCode, byteCodeLen, NULL, &m_resource));
  return true;
}

void PixelShader::_bind(ID3D11Buffer* const* buffers, uint32 numBuffers)
{
  WeakPtr<RenderSystem> renderSys = g_Game->getRenderSystem();
  if (numBuffers > 0)
  {
    renderSys->getDeviceContextPtr()->PSSetConstantBuffers(0, numBuffers, buffers);
  }

  renderSys->getDeviceContextPtr()->PSSetShader(m_resource, NULL, 0);
}