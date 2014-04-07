#ifndef __Mesh_h_
#define __Mesh_h_

//#include "Shader.h"

#define MAX_VERTEX_STREAMS 5

struct IntermediateMeshData
{
  std::vector<Vector3> position;
  std::vector<Vector3> normal;
  std::vector<Vector3> tangent;
  std::vector<Vector3> bitangent;
  std::vector<Vector2> uv0;
  std::vector<Vector2> uv1;
  std::vector<Vector2> uv2;
  std::vector<uint32> indices;
};

enum eMeshOptions
{
  // creation options
  OPT_CREATE_NORMALS       = 0x001,  // auto-create normal vectors
  OPT_CREATE_TANGENTS      = 0x002,  // auto-create tangent vectors
  OPT_FIX_TANGENT_FRAME    = 0x004,  // fix tangent frame on mirror edges
  OPT_OPTIMIZE_FOR_CACHE   = 0x008,  // optimize data for vertex cache

  // vertex layout options
  OPT_USE_MULTI_STREAM     = 0x010,  // split vertex attributes in multiple streams
  OPT_16BIT_VERTEX_DATA    = 0x020,  // use half for pos, normal, ...
  OPT_32BIT_INDEX_DATA     = 0x040,  // use 32-bit indices
};

class MeshChunk
{
  friend class Mesh;

public:
private:
  uint32 streamMask;
  uint32 indexCount;
  uint32 vertexSize;
  ID3D11Buffer* streams[MAX_VERTEX_STREAMS];
  ID3D11Buffer* indices;
  Texture* m_diffuseMap;
  Texture* m_bumpMap;
};

class Mesh
{
public:
  Mesh();
  ~Mesh();

  void destroy();
  void render(const Matrix4& view, const Matrix4& proj);

  void initDummyMaterial();

  static bool loadFromObj(const String& filename, Mesh& mesh);
  static bool createSphere(float radius, uint32 segments, Mesh& mesh);

private:
  static void fixTangentData(IntermediateMeshData& data);
  static void mergeDuplicateVertices(const IntermediateMeshData& source, IntermediateMeshData& data);
  static MeshChunk* createMeshChunk(const IntermediateMeshData& data, Mesh& mesh);

  // mesh parts
  std::vector<MeshChunk*> m_meshChunks;
  // vertex declaration
  SharedPtr<VertexDeclaration> m_vertexDeclaration;

  // only dummy data
  SharedPtr<VertexShader> m_vertexShader;
  SharedPtr<PixelShader> m_pixelShader;
};

#endif // __Mesh_h_