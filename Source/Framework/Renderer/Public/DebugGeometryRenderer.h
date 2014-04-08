#ifndef __DebugGeometryRenderer_h_
#define __DebugGeometryRenderer_h_

struct ID3D11Buffer;

class DebugGeometryRenderer
{
public:
  DebugGeometryRenderer();
  ~DebugGeometryRenderer();

  void init();
  void shutdown();
  void prepareDebugRendering();
  void drawAllDebugGeometry(const Matrix4& view, const Matrix4& proj);

  void drawLine(const Vector3& pos1, const Vector3& pos2, uint32 color = 0xffffffff);
  void drawArrow(const Vector3& pos, const Vector3& dir, float len, float tipSize = 1.0f, uint32 color = 0xffffffff);
  void drawSphere(const Vector3& center, float radius, uint32 color = 0xffffffff);
  void drawAABB(const Vector3& center, const Vector3& min, const Vector3& max, uint32 color = 0xffffffff);

private:
  struct DebugVertex
  {
    float x, y, z;
    uint32 color;
  };

  ID3D11Buffer* m_vertexBuffer;
  ID3D11InputLayout* m_inputLayout;
  DebugVertex* m_mappedMemory;
  DebugVertex* m_nextVertex;
  SharedPtr<VertexShader> m_vertexShader;
  SharedPtr<PixelShader> m_pixelShader;
  SharedPtr<VertexDeclaration> m_vertexDeclaration;
};

#endif // __DebugGeometryRenderer_h_