#include "Core.h"
#include "Mesh.h"
#include "Game.h"
#include "RenderSystem.h"
#include "StringUtils.h"
#include "RenderStates.h"
#include "VertexDeclaration.h"
#include "ShaderDrawBundle.h"
#include "Shader.h"
#include "SystemTextures.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
  destroy();
}

void Mesh::destroy()
{
  for (std::vector<MeshChunk*>::iterator it = m_meshChunks.begin();
    it != m_meshChunks.end(); ++it)
  {
    for (uint32 i = 0; i < MAX_VERTEX_STREAMS; ++i)
      SAFE_RELEASE((*it)->streams[i]);
    SAFE_RELEASE((*it)->indices);
    (*it)->streamMask =0;

    delete *it;
  }

  m_meshChunks.clear();
}

void Mesh::render(const Matrix4& view, const Matrix4& proj)
{
  m_vertexShader->beginUpdateParameters();
  m_vertexShader->setParamByName("projMat", proj.getPtr(), sizeof(float)*16);
  m_vertexShader->setParamByName("viewMat", view.getPtr(), sizeof(float)*16);
  m_vertexShader->endUpdateParameters();

  RENDER_CONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  SharedPtr<ShaderDrawBundle> shaderDrawBundle = ShaderDrawBundle::createShaderDrawBundle(m_vertexShader, m_pixelShader, m_vertexDeclaration);
  g_Game->getRenderSystem()->setShaderDrawBundle(shaderDrawBundle.get());

  for (std::vector<MeshChunk*>::iterator it = m_meshChunks.begin();
    it != m_meshChunks.end(); ++it)
  {
    MeshChunk* chunk = *it;

    // material
    Texture* texture = chunk->m_diffuseMap;
    if (!texture)
    {
      texture = SystemTextures::Default.get();
    }

    if (texture)
    {
      ID3D11ShaderResourceView* srv = texture->getDefaultSRV();
      RENDER_CONTEXT->PSSetShaderResources(0, 1, &srv);
      ID3D11SamplerState* samplerStates = SamplerState<>::get();
      RENDER_CONTEXT->PSSetSamplers(0, 1, &samplerStates);
    }

    ID3D11Buffer* buffers[MAX_VERTEX_STREAMS] = {0};
    UINT offsets[MAX_VERTEX_STREAMS] = {0};
    UINT strides[MAX_VERTEX_STREAMS] = {0};
    
    for (uint32 stream = 0, mask = 1; stream < MAX_VERTEX_STREAMS; ++stream, mask <<= 1)
    {
      if ((chunk->streamMask & mask) == 0)
        continue;

      buffers[stream] = chunk->streams[stream];
      offsets[stream] = 0;
      strides[stream] = chunk->vertexSize;
    }

    RENDER_CONTEXT->IASetIndexBuffer(chunk->indices, DXGI_FORMAT_R16_UINT, 0);
    RENDER_CONTEXT->IASetVertexBuffers(0, MAX_VERTEX_STREAMS, buffers, strides, offsets);
    RENDER_CONTEXT->DrawIndexed(chunk->indexCount, 0, 0);
  }
}

void Mesh::initDummyMaterial()
{
  DataBlob buffer;
  if (ShaderCompiler::compile("Data\\Shaders\\renderMesh.hlsl", "vs_main", "vs_5_0", buffer))
  {
    m_vertexShader = new VertexShader();
    m_vertexShader->init(buffer);
  }

  if (ShaderCompiler::compile("Data\\Shaders\\renderMesh.hlsl", "ps_main", "ps_5_0", buffer))
  {
    m_pixelShader = new PixelShader();
    m_pixelShader->init(buffer);
  }
}

void Mesh::fixTangentData(IntermediateMeshData& data)
{
  if (data.uv0.size() == 0)
  {
    return;
  }

  uint32 numTris = data.position.size() / 3;
  for (uint32 i = 0; i < numTris; ++i)
  {
    uint32 idx[3] = {0};

    idx[0] = i*3+0;
    idx[1] = i*3+1;
    idx[2] = i*3+2;

    const Vector3& edgeA = data.position[idx[1]] - 
      data.position[idx[0]];

    const Vector3& edgeB = data.position[idx[2]] - 
      data.position[idx[0]];

    const Vector2& uv0 = data.uv0[idx[0]];
    const Vector2& uv1 = data.uv0[idx[1]];
    const Vector2& uv2 = data.uv0[idx[2]];

    float s1 = uv1.x - uv0.x;
    float t1 = uv0.y - uv1.y;
    float s2 = uv2.x - uv0.x;
    float t2 = uv0.y - uv2.y;

    float det = 1.0f / (s1 * t2 - s2 * t1);
    if (fabs(det) <= EPSILON)
    {
      det = 1.0f;
    }

    Vector3 tangent = Vector3((t2*edgeA.x - t1*edgeB.x) * det,
      (t2*edgeA.y - t1*edgeB.y) * det,
      (t2*edgeA.z - t1*edgeB.z) * det);

    Vector3 bitangent = Vector3((s1*edgeB.x - s2*edgeA.x) * det,
      (s1*edgeB.y - s2*edgeA.y) * det,
      (s1*edgeB.z - s2*edgeA.z) * det);

    Vector3 normal = cross(tangent, bitangent);

    normalize(tangent);
    normalize(bitangent);
    normalize(normal);
      
    Vector3 surfaceNormal = cross(edgeA, edgeB);
    normalize(surfaceNormal);

    bool needFixTB = dot(surfaceNormal, normal) < 0;

    for (uint32 vertex = 0; vertex < 3; ++vertex)
    {
      Vector3 vertexTangent = Vector3(data.tangent[idx[vertex]]);
      Vector3 vertexBitangent = data.bitangent[idx[vertex]];
      float handeness = 1.0f;

      if (fabs(squaredLength(vertexTangent)) < EPSILON)
      {
        vertexTangent = tangent;
      }
      else
      {
        if (needFixTB)
        {
          if (dot(vertexTangent, tangent) < 0)
          {
            vertexTangent = -vertexTangent;
          }
          else
          {
            handeness = -1.0f;
          }
        }
      }

      // FIXME: handeness

      data.tangent[idx[vertex]] = vertexTangent; 
    }
  }
}

void Mesh::mergeDuplicateVertices(const IntermediateMeshData& source, IntermediateMeshData& data)
{
  std::multimap<uint32, int> indexRemapping;

  bool hasNormals = source.normal.size() > 0;
  bool hasUv0 = source.uv0.size() > 0;

  uint32 duplicateVertices = 0;
  uint32 numTriangles = source.position.size() / 3;

  for (uint32 i = 0; i < numTriangles; ++i)
  {
    for (uint32 j = 0; j < 3; ++j)
    {
      uint32 index = i*3+j;

      Vector3 pos = source.position[index];

      uint32 vertexHash = 5381;
      vertexHash = ((vertexHash << 5) + vertexHash) + *(uint32*)&pos.x;
      vertexHash = ((vertexHash << 5) + vertexHash) + *(uint32*)&pos.y;
      vertexHash = ((vertexHash << 5) + vertexHash) + *(uint32*)&pos.z;
      
      bool foundMatchingVertex = false;
      uint32 duplicateIndex = 0;

      for(std::pair<std::multimap<uint32, int>::iterator, std::multimap<uint32, int>::iterator> it = indexRemapping.equal_range(vertexHash);
        it.first != it.second && !foundMatchingVertex; ++it.first)
      {
        foundMatchingVertex = true;

        const int destIndex = data.position.size() - 1;

        duplicateIndex = (it.first)->second;
        int index = duplicateIndex;
        foundMatchingVertex &= equals(data.position[index], pos);

        if (hasNormals)
        {
          foundMatchingVertex &= equals(data.normal[index], source.normal[index]);
        }

        if (hasUv0)
        {
          foundMatchingVertex &= equals(data.uv0[index], source.uv0[index]);
        }

        // FIXME: other attributes...
      }

      if (foundMatchingVertex)
      {
        data.indices.push_back(duplicateIndex);
        duplicateVertices++;
      }
      else
      {
        data.position.push_back(pos);

        const int destIndex = data.position.size() - 1;

        if (hasNormals)
        {
          data.normal.push_back(source.normal[index]);
        }

        if (hasUv0)
        {
          data.uv0.push_back(source.uv0[index]);
        }

        // FIXME: other attributes...

        data.indices.push_back(destIndex);

        if (!foundMatchingVertex)
          indexRemapping.insert(std::make_pair(vertexHash, destIndex));
      }
    }
  }
}

MeshChunk* Mesh::createMeshChunk(const IntermediateMeshData& data, Mesh& mesh)
{
  MeshChunk* newChunk = new MeshChunk();
  newChunk->streamMask = 1;
  newChunk->indexCount = data.indices.size();

  mesh.m_meshChunks.push_back(newChunk);

  // vertices
  bool hasNormal = (data.normal.size() > 0);
  bool hasTangent = (data.tangent.size() > 0);
  bool hasBitangent = (data.bitangent.size() > 0);
  bool hasUv0 = (data.uv0.size() > 0);
  bool hasUv1 = (data.uv1.size() > 0);
  bool hasUv2 = (data.uv2.size() > 0);

  // create vertex declaration if not already available
  if (!mesh.m_vertexDeclaration)
  {
    VertexDeclaration* vertexDecl = new VertexDeclaration();

    uint32 offset = 0;
    const VertexElement& element = vertexDecl->add("Position", 0, VEF_FLOAT3, 0, offset, false);
    offset += VertexDeclaration::sizeOfElementType(element.format);
    if (hasNormal)
    {
      const VertexElement& element = vertexDecl->add("Normal", 0, VEF_FLOAT3, 0, offset, false);
      offset += VertexDeclaration::sizeOfElementType(element.format);
    }
    if (hasTangent)
    {
      const VertexElement& element = vertexDecl->add("Tangent", 0, VEF_FLOAT3, 0, offset, false);
      offset += VertexDeclaration::sizeOfElementType(element.format);
    }
    if (hasBitangent)
    {
      const VertexElement& element = vertexDecl->add("Bitangent", 0, VEF_FLOAT3, 0, offset, false);
      offset += VertexDeclaration::sizeOfElementType(element.format);
    }
    if (hasUv0)
    {
      const VertexElement& element = vertexDecl->add("Texcoord", 0, VEF_FLOAT2, 0, offset, false);
      offset += VertexDeclaration::sizeOfElementType(element.format);
    }
    if (hasUv1)
    {
      const VertexElement& element = vertexDecl->add("Texcoord", 1, VEF_FLOAT2, 0, offset, false);
      offset += VertexDeclaration::sizeOfElementType(element.format);
    }
    if (hasUv2)
    {
      const VertexElement& element = vertexDecl->add("Texcoord", 2, VEF_FLOAT2, 0, offset, false);
      offset += VertexDeclaration::sizeOfElementType(element.format);
    }

    mesh.m_vertexDeclaration = vertexDecl;
  } 

  newChunk->vertexSize = 0;
  for (uint32 elementIdx = 0; ; ++elementIdx)
  {
    const VertexElement* element = mesh.m_vertexDeclaration->getElement(elementIdx);
    if (!element)
      break;
    newChunk->vertexSize += VertexDeclaration::sizeOfElementType(element->format);
  }

  size_t bufferSize = newChunk->vertexSize * data.position.size();
  void* buffer = malloc(bufferSize);
  float* dest = (float*)buffer;

  for (uint32 i = 0; i < data.position.size(); ++i)
  {
    *dest++ = data.position[i].x;
    *dest++ = data.position[i].y;
    *dest++ = data.position[i].z;

    if (hasNormal)
    {
      *dest++ = data.normal[i].x;
      *dest++ = data.normal[i].y;
      *dest++ = data.normal[i].z;
    }

    if (hasTangent)
    {
      *dest++ = data.tangent[i].x;
      *dest++ = data.tangent[i].y;
      *dest++ = data.tangent[i].z;
    }

    if (hasBitangent)
    {
      *dest++ = data.bitangent[i].x;
      *dest++ = data.bitangent[i].y;
      *dest++ = data.bitangent[i].z;
    }

    if (hasUv0)
    {
      *dest++ = data.uv0[i].x;
      *dest++ = data.uv0[i].y;
    }

    if (hasUv1)
    {
      *dest++ = data.uv1[i].x;
      *dest++ = data.uv1[i].y;
    }

    if (hasUv2)
    {
      *dest++ = data.uv2[i].x;
      *dest++ = data.uv2[i].y;
    }
  }

  D3D11_BUFFER_DESC desc = {0};
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.ByteWidth = bufferSize;
  desc.Usage = D3D11_USAGE_IMMUTABLE;

  D3D11_SUBRESOURCE_DATA initialData = {0};
  initialData.pSysMem = buffer;
  VALIDATE(RENDER_DEVICE->CreateBuffer(&desc, &initialData, &newChunk->streams[0]));

  // indices
  bool use32BitIndices = data.position.size() > 0xffff;
  bufferSize = data.indices.size() * (use32BitIndices ? sizeof(uint32) : sizeof(uint16));
  buffer = malloc(bufferSize);
  if (use32BitIndices)
  {
    memcpy(buffer, &data.indices[0], sizeof(uint32) * data.indices.size());
  }
  else
  {
    uint16* dest = (uint16*)buffer;
    for (uint32 i = 0; i < data.indices.size(); ++i)
      *dest++ = (uint16)data.indices[i];
  }

  desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  desc.ByteWidth = bufferSize;
  desc.Usage = D3D11_USAGE_IMMUTABLE;

  initialData.pSysMem = buffer;
  VALIDATE(RENDER_DEVICE->CreateBuffer(&desc, &initialData, &newChunk->indices));

  free(buffer);

  return newChunk;
}