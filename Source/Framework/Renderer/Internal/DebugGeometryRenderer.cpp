#include "Core.h"
#include "DebugGeometryRenderer.h"
#include "Game.h"
#include "RenderSystem.h"
#include "Shader.h"

DebugGeometryRenderer::DebugGeometryRenderer()
  : m_vertexBuffer(0)
  , m_inputLayout(0)
  , m_vertexShader(0)
  , m_pixelShader(0)
{
}

DebugGeometryRenderer::~DebugGeometryRenderer()
{
  shutdown();
}

void DebugGeometryRenderer::init()
{
  shutdown();

  D3D11_BUFFER_DESC desc;
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.ByteWidth = 1024 * 1024 * 4;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;
  desc.Usage = D3D11_USAGE_DYNAMIC;

  WeakPtr<RenderSystem> renderSys = g_Game->getRenderSystem();
  VALIDATE(renderSys->getDevicePtr()->CreateBuffer(&desc, NULL, &m_vertexBuffer));

  DataBlob buffer;
  if (ShaderCompiler::compile("Data\\Shaders\\debug.hlsl", "vs_main", "vs_5_0", buffer))
  {
    m_vertexShader = new VertexShader();
    m_vertexShader->init(buffer);

    D3D11_INPUT_ELEMENT_DESC desc[] = 
    {
      { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    VALIDATE(renderSys->getDevicePtr()->CreateInputLayout(desc, 2, buffer.getPtr(), buffer.getSize(), &m_inputLayout));
  }

  if (ShaderCompiler::compile("Data\\Shaders\\debug.hlsl", "ps_main", "ps_5_0", buffer))
  {
    m_pixelShader = new PixelShader();
    m_pixelShader->init(buffer);
  }
}

void DebugGeometryRenderer::shutdown()
{
  delete m_vertexShader;
  delete m_pixelShader;
  SAFE_RELEASE(m_vertexBuffer);
  SAFE_RELEASE(m_inputLayout);
}

void DebugGeometryRenderer::prepareDebugRendering()
{
  WeakPtr<RenderSystem> renderSys = g_Game->getRenderSystem();
  
  m_nextVertex = 0;

  D3D11_MAPPED_SUBRESOURCE mappedSubRes;
  if (SUCCEEDED(renderSys->getDeviceContextPtr()->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubRes)))
  {
    m_mappedMemory = 
      m_nextVertex = (DebugVertex*)mappedSubRes.pData;
  }
}

void DebugGeometryRenderer::drawAllDebugGeometry(const Matrix4& view, const Matrix4& proj)
{
  WeakPtr<RenderSystem> renderSys = g_Game->getRenderSystem();

  if (m_mappedMemory)
  {
    renderSys->getDeviceContextPtr()->Unmap(m_vertexBuffer, 0);
  }

  m_vertexShader->beginUpdateParameters();
  m_vertexShader->setParamByName("projMat", proj.getPtr(), sizeof(float)*16);
  m_vertexShader->setParamByName("modelMat", view.getPtr(), sizeof(float)*16);
  m_vertexShader->endUpdateParameters();

  UINT strides = sizeof(DebugVertex);
  UINT offsets = 0;
  renderSys->getDeviceContextPtr()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &strides, &offsets);
  renderSys->getDeviceContextPtr()->IASetIndexBuffer(0, DXGI_FORMAT_UNKNOWN, 0);
  renderSys->getDeviceContextPtr()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  renderSys->getDeviceContextPtr()->IASetInputLayout(m_inputLayout);
  m_vertexShader->bindToPipeline();
  m_pixelShader->bindToPipeline();
  renderSys->getDeviceContextPtr()->Draw(m_nextVertex - m_mappedMemory, 0);
}

void DebugGeometryRenderer::drawLine(const Vector3& pos1, const Vector3& pos2, uint32 color)
{
  m_nextVertex->x = pos1.x;
  m_nextVertex->y = pos1.y;
  m_nextVertex->z = pos1.z;
  m_nextVertex->color = color;
  ++m_nextVertex;

  m_nextVertex->x = pos2.x;
  m_nextVertex->y = pos2.y;
  m_nextVertex->z = pos2.z;
  m_nextVertex->color = color;
  ++m_nextVertex;
}

void DebugGeometryRenderer::drawArrow(const Vector3& pos, const Vector3& dir, float len, float tipSize, uint32 color)
{
  Vector3 pos1 = pos + dir * len;

  // shaft
  drawLine(pos, pos1, color);

  // tip
  Vector3 offsets[4];
  Vector3 pos2 = pos + dir * (len - tipSize);
  Vector3 absDir = abs(dir);
  float a = max(absDir.x, max(absDir.y, absDir.z));
  int j, i = 0;
  for (; i < 3; ++i)
  {
    if (dir[i] == a)
      break;
  }

  i = i % 3;
  j = (i + 1) % 3;

  Vector3 ortho = dir;
  ortho[i]  = -dir[j];
  ortho[j] = dir[i];

  Vector3 w2 = cross(dir, ortho);
  Vector3 w3 = cross(dir, w2);

  offsets[0] = w2 - dir * (dot(dir, w2) / length(dir));
  offsets[2] = w3 - dir * (dot(dir, w3) / length(dir)) - offsets[0] * (dot(offsets[0], w3) / length(offsets[0]));
  offsets[1] = -offsets[0];
  offsets[3] = -offsets[2];

  float halfTipSize = tipSize * 0.5f;

  for (int i = 0; i < 4; ++i)
  {
    drawLine(pos1, pos2 + offsets[i] * halfTipSize, color);
  }
}

void DebugGeometryRenderer::drawSphere(const Vector3& center, float radius, uint32 color)
{
  const uint32 steps = 50;

  float angle = 0;
  float angleStep = TWO_PI / steps;

  for (uint32 i = 0; i <= steps; ++i, angle += angleStep)
  {
    float a = cos(angle) * radius;
    float b = sin(angle) * radius;
    float c = cos(angle+angleStep) * radius;
    float d = sin(angle+angleStep) * radius;

    // first
    drawLine(center + Vector3(a, b, 0), 
      center + Vector3(c, d, 0), color);

    // second
    drawLine(center + Vector3(a, 0, b),
      center + Vector3(c, 0, d), color);

    // third
    drawLine(center + Vector3(0, a, b),
      center + Vector3(0, c, d), color);
  }
}

void DebugGeometryRenderer::drawAABB(const Vector3& center, const Vector3& min, const Vector3& max, uint32 color)
{
  const Vector3 edges[8] = 
  {
    center + Vector3(min.x, max.y, max.z),
    center + Vector3(min.x, min.y, max.z),
    center + Vector3(max.x, min.y, max.z),
    center + Vector3(max.x, max.y, max.z),
    center + Vector3(min.x, max.y, min.z),
    center + Vector3(min.x, min.y, min.z),
    center + Vector3(max.x, min.y, min.z),
    center + Vector3(max.x, max.y, min.z)
  };

  for (int i = 0; i < 4; ++i)
  {
    drawLine(edges[i], edges[(i+1)%4], color);
    drawLine(edges[i+4], edges[((i+1)%4)+4], color);
    drawLine(edges[i], edges[i+4], color);
  }
}