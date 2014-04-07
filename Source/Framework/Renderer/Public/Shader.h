#ifndef __Shader_h_
#define __Shader_h_

#include "RenderSystemPrerequisites.h"

#define MAX_CONSTANT_BUFFERS 5

enum eInputParameterType
{
  INPUT_PARAM_FLOAT,
  INPUT_PARAM_UINT,
  INPUT_PARAM_INT
};

struct ShaderParameter
{
  String name;
  uint32 byteOffset;
  uint32 bufferIndex;
};

typedef std::vector<ShaderParameter> ShaderParameterArray;

struct ShaderInputParameter
{
  String semantic;
  uint32 semanticIndex;
  eInputParameterType type;
  uint32 componentsUsed;
};

typedef std::vector<ShaderInputParameter> ShaderInputParameterArray;

class Shader
{
public:
  explicit Shader();
  virtual ~Shader();

  void init(const DataBlob& data);
  void destroy();

  void bindToPipeline();

  // paramter interface
  void setParamByName(const String& name, float value);
  void setParamByName(const String& name, float value1, float value2);
  void setParamByName(const String& name, float value1, float value2, float value3);
  void setParamByName(const String& name, float value1, float value2, float value3, float value4);
  void setParamByName(const String& name, const float* value, uint32 size);
  void beginUpdateParameters();
  void endUpdateParameters();

protected:
  virtual bool _createShaderResource(const void* byteCode, uint32 byteCodeLen) = 0;
  virtual void _bind(ID3D11Buffer* const* buffers, uint32 numBuffers) = 0;

private:
  void _allocateCBuffer(uint32 index, uint32 size);
  void _destroyCBuffer(uint32 index);
  ubyte* _getParameterPointer(const String& name);

  ShaderParameterArray m_parameters;
  ShaderInputParameterArray m_inputSignature;

  struct cbuffer
  {
    ubyte* backingStore;
    ID3D11Buffer* buffer;
    bool dirty;
    uint32 size;
  };

  cbuffer m_cbuffers[MAX_CONSTANT_BUFFERS];
  uint32 m_maxSlot;
  ID3D11Buffer* m_buffersToBind[MAX_CONSTANT_BUFFERS];
};

class VertexShader : public Shader
{
public:
  VertexShader();
  virtual ~VertexShader();

  ID3D11VertexShader* getResourcePtr() const { return m_resource; }
  const ubyte* getCode() const { return (ubyte*)m_code.getPtr(); }
  uint32 getCodeSize()  const { return m_code.getSize(); }

protected:
  virtual bool _createShaderResource(const void* byteCode, uint32 byteCodeLen);
  virtual void _bind(ID3D11Buffer* const* buffers, uint32 numBuffers);

private:
  DataBlob m_code;
  ID3D11VertexShader* m_resource;
};

class PixelShader : public Shader
{
public:
  PixelShader();
  virtual ~PixelShader();

  ID3D11PixelShader* getResourcePtr() const { return m_resource; }

protected:
  virtual bool _createShaderResource(const void* byteCode, uint32 byteCodeLen);
  virtual void _bind(ID3D11Buffer* const* buffers, uint32 numBuffers);

private:
  ID3D11PixelShader* m_resource;
};

#endif // __Shader_h_