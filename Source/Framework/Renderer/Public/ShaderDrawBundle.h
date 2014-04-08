#ifndef __ShaderDrawBundle_h_
#define __ShaderDrawBundle_h_

class ShaderDrawBundle
{
public:
  ShaderDrawBundle();
  ~ShaderDrawBundle();

  WeakPtr<VertexShader> getVertexShader() const { return m_vertexShader; }
  WeakPtr<PixelShader> getPixelShader() const { return m_pixelShader; }
  ID3D11InputLayout* getInputLayout() const { return m_inputLayout; }

  static SharedPtr<ShaderDrawBundle> createShaderDrawBundle(SharedPtr<VertexShader> vertexShader, SharedPtr<PixelShader> pixelShader, SharedPtr<const VertexDeclaration> vertexDeclaration);

private:
  SharedPtr<VertexShader> m_vertexShader;
  SharedPtr<PixelShader> m_pixelShader;
  ID3D11InputLayout* m_inputLayout;
};

#endif // __ShaderDrawBundle_h_