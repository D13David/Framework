#include "Core.h"
#include "Mesh.h"
#include "Game.h"
#include "RenderSystem.h"
#include "StringUtils.h"

struct materialParameters
{
  String diffuseMap;
  String normalMap;
};

struct vertexIndex
{
  uint32 posIndex;
  uint32 normalIndex;
  uint32 texcoordIndex;
};

struct geometryGroup
{
  std::vector<vertexIndex> indices;
  std::vector<uint32> indexCounts;
  materialParameters* material;
};

bool Mesh::loadFromObj(const String& filename, Mesh& mesh)
{
  DataBlob data;
  if (!readRawBlob(filename, data))
    return false;

  String path = StringUtils::stripFileName(filename);

  String materialLib;
  std::vector<float> positions;
  std::vector<float> normals;
  std::vector<float> texcoords;
  std::map<String, geometryGroup*> indices;  
  std::map<String, materialParameters> materials;

  const char* curr = (const char*)data.getPtr();
  const char* last = curr;
  const char* end = curr + data.getSize();

  // first pass - assable all geometry data
  while (true)
  {
    // eat whitespaces
    while (*curr <= ' ' && curr < end)
      ++curr;

    last = curr;

    // go to next line
    while (*curr != '\n' && curr < end)
      ++curr;

    // vertex data
    if (last[0] == 'v')
    {
      float x, y, z;
      char* next;

      if (last[1] == 't') // texcoord
      {
        x = (float)strtod(last+2, &next);
        y = (float)strtod(next, &next);
        z = (float)strtod(next, 0);
        texcoords.push_back(x);
        texcoords.push_back(y);
        texcoords.push_back(z);
      }
      else if (last[1] == 'n') // normal
      {
        x = (float)strtod(last+2, &next);
        y = (float)strtod(next, &next);
        z = (float)strtod(next, 0);
        normals.push_back(x);
        normals.push_back(y);
        normals.push_back(z);
      }
      else // pos
      {
        x = (float)strtod(last+1, &next);
        y = (float)strtod(next, &next);
        z = (float)strtod(next, 0);
        positions.push_back(x);
        positions.push_back(y);
        positions.push_back(z);
      }
    }
    else if (!strncmp(last, "mtllib", 6))
    {
      const char* nameEnd = curr;
      while (*nameEnd <= ' ')
        --nameEnd;

      last += 7;
      materialLib = String(last, nameEnd-last+1);
    }

    ++curr; // jump over \n

    // eof reached
    if (curr >= end)
      break;

    last = curr;
  }

  //// now read first the material library, to make it available for the next pass
  DataBlob data1;
  if (!readRawBlob(path + materialLib, data1))
    return false;

  curr = (const char*)data1.getPtr();
  last = curr;
  end = curr + data1.getSize();

  materialParameters* currMaterial = 0;

  while (true)
  {
    // eat whitespaces
    while (*curr <= ' ' && curr < end)
      ++curr;

    last = curr;

    // go to next line
    while (*curr != '\n' && curr < end)
      ++curr;

    if (!strncmp(last, "newmtl", 6))
    {
      const char* nameEnd = curr;
      while (*nameEnd <= ' ')
        --nameEnd;

      last += 7;
      String materialName = String(last, nameEnd-last+1);
      materials.insert(std::make_pair(materialName, materialParameters()));
      currMaterial = &materials[materialName];
    }
    else if (!strncmp(last, "map_Kd", 6))
    {
      const char* nameEnd = curr;
      while (*nameEnd <= ' ')
        --nameEnd;

      last += 7;
      currMaterial->diffuseMap = String(last, nameEnd-last+1);
    }
    else if (!strncmp(last, "map_bump", 8))
    {
      const char* nameEnd = curr;
      while (*nameEnd <= ' ')
        --nameEnd;

      last += 9;
      currMaterial->normalMap = String(last, nameEnd-last+1);
    }

    ++curr; // jump over \n

    // eof reached
    if (curr >= end)
      break;

    last = curr;
  }
  ////

  curr = (const char*)data.getPtr();
  last = curr;
  end = curr + data.getSize();

  String currentSubmeshName = "none";
  geometryGroup defaultGroup;
  geometryGroup* group = &defaultGroup;
  char lineBuffer[1024];

  // second pass - group data to submeshes
  while (true)
  {
    // eat whitespaces
    while (*curr <= ' ' && curr < end)
      ++curr;

    last = curr;

    // go to next line
    while (*curr != '\n' && curr < end)
      ++curr;

    if (last[0] == 'g')
    {
      const char* nameEnd = curr;
      while (*nameEnd <= ' ')
        --nameEnd;

      last += 2; // g tag
      currentSubmeshName = String(last, nameEnd-last+1);
      group = new geometryGroup();
      indices.insert(std::make_pair(currentSubmeshName, group));
    }
    else if (!strncmp(last, "usemtl", 6))
    {
      const char* nameEnd = curr;
      while (*nameEnd <= ' ')
        --nameEnd;

      last += 7; // g tag
      String materialName = String(last, nameEnd-last+1);
      std::map<String, materialParameters>::iterator matLibIt = materials.find(materialName);
      if (matLibIt != materials.end())
      {
        group->material = &matLibIt->second;
      }
      else
      {
        int sdf = 0;
      }
    }
    else if (last[0] == 'f')
    {
      ptrdiff_t size = (curr - last) - 2;

      memcpy(lineBuffer, last+2, size);

      // terminate line and trim whitespaces from the end
      do
      {
        lineBuffer[size] = '\0';  
      } while (lineBuffer[--size] <= ' ');

      uint32 indexCount = 0;
      vertexIndex index;
      char* p = lineBuffer;
      char* next = 0;

      while (*p)
      {
        //char* pos = strtok(p, "/");
        char* pos = strtok_s(p, "/", &next);
        index.posIndex = atoi(pos)-1;

        //pos = strtok(0, "/");
        pos = strtok_s(0, "/", &next);
        index.texcoordIndex = atoi(pos)-1;

        //pos = strtok(0, "  \0");
        pos = strtok_s(0, "  \0", &next);
        index.normalIndex = atoi(pos)-1;

        group->indices.push_back(index);

        ++indexCount;
        p = pos;

        // seek beginning of next index
        while (*p++)
          ;
      }

      group->indexCounts.push_back(indexCount);
    }

    ++curr; // jump over \n

    // eof reached
    if (curr >= end)
      break;

    last = curr;
  }

  mesh.destroy();

  bool hasNormal = normals.size() > 0;
  bool hasTexcoords = texcoords.size() > 0;
  bool hasTangent = false;

  int maxMeshes = 70;
  for (std::map<String, geometryGroup*>::iterator it = indices.begin();
    it != indices.end(), maxMeshes > 0; ++it, --maxMeshes)
  {
    IntermediateMeshData data, finalMeshData;

    // pack vertices, triangulate and extract for each submesh
    for (uint32 i = 0, j = 0; i < it->second->indexCounts.size(); ++i, ++j)
    {
      int numVertices = it->second->indexCounts[i];

      const vertexIndex& index0 = it->second->indices[j++];

      while (numVertices > 0)
      {
        const vertexIndex& index2 = it->second->indices[j++];
        const vertexIndex& index1 = it->second->indices[j];

        data.position.push_back(Vector3(positions[index0.posIndex*3+0],
          positions[index0.posIndex*3+1], positions[index0.posIndex*3+2]));

        data.position.push_back(Vector3(positions[index1.posIndex*3+0],
          positions[index1.posIndex*3+1], positions[index1.posIndex*3+2]));

        data.position.push_back(Vector3(positions[index2.posIndex*3+0],
          positions[index2.posIndex*3+1], positions[index2.posIndex*3+2]));

        if (hasNormal)
        {
          data.normal.push_back(Vector3(normals[index0.normalIndex*3+0],
            normals[index0.normalIndex*3+1], normals[index0.normalIndex*3+2]));

          data.normal.push_back(Vector3(normals[index1.normalIndex*3+0],
            normals[index1.normalIndex*3+1], normals[index1.normalIndex*3+2]));

          data.normal.push_back(Vector3(normals[index2.normalIndex*3+0],
            normals[index2.normalIndex*3+1], normals[index2.normalIndex*3+2]));
        }

        if (hasTexcoords)
        {
          data.uv0.push_back(Vector2(texcoords[index0.texcoordIndex*3+0],
            texcoords[index0.texcoordIndex*3+1]));

          data.uv0.push_back(Vector2(texcoords[index1.texcoordIndex*3+0],
            texcoords[index1.texcoordIndex*3+1]));

          data.uv0.push_back(Vector2(texcoords[index2.texcoordIndex*3+0],
            texcoords[index2.texcoordIndex*3+1]));
        }

        numVertices -= 3;
      }
    }

    if (hasTangent)
      fixTangentData(data);

    mergeDuplicateVertices(data, finalMeshData);

    MeshChunk* meshChunk = createMeshChunk(finalMeshData, mesh);
    if (meshChunk)
    {
      //meshChunk->m_texture->load(path + it->second->material->diffuseMap);
      meshChunk->m_diffuseMap = (Texture*)g_Game->getRenderSystem()->getTextureManager()->load(path + it->second->material->diffuseMap);
      meshChunk->m_bumpMap = (Texture*)g_Game->getRenderSystem()->getTextureManager()->load(path + it->second->material->normalMap);
    }
  }

  mesh.initDummyMaterial();

  return true;
}

bool Mesh::createSphere(float radius, uint32 segments, Mesh& mesh)
{
  IntermediateMeshData data;

  mesh.destroy();

  const uint32 numVertices = segments + 1;

  const float verticalStepSize = PI / segments;
  const float horizontalStepSize = TWO_PI / segments;

  for (uint32 i = 0; i < numVertices; ++i)
  {
    float theta = HALF_PI - verticalStepSize * i;

    for (uint32 j = 0; j < numVertices; ++j)
    {
      float phi = horizontalStepSize * j;

      const float cosTheta = cosf(theta);
      const float sinTheta = sinf(theta);
      const float cosPhi = cosf(phi);
      const float sinPhi = sinf(phi);

      Vector3 position = Vector3(
          cosTheta * cosPhi,
          sinTheta,
          cosTheta * sinPhi
        ) * radius;

      data.position.push_back(position);

      data.uv0.push_back(Vector2(
          j / (float)(numVertices-1),
          i / (float)(numVertices-1)
        ));

      data.normal.push_back(Vector3(
          normalize(position)
        ));
    }
  }

  for (uint32 i = 0; i < segments; ++i)
  {
    for (uint32 j = 0; j < segments; ++j)
    {
      uint32 topLeft = j + i * numVertices;
      uint32 topRight = (j+1) + i * numVertices;
      uint32 bottomLeft = j + (i+1) * numVertices;
      uint32 bottomRight = (j+1) + (i+1) * numVertices;

      data.indices.push_back(topLeft);
      data.indices.push_back(bottomLeft);
      data.indices.push_back(topRight);

      data.indices.push_back(topRight);
      data.indices.push_back(bottomLeft);
      data.indices.push_back(bottomRight);
    }
  }

  MeshChunk* meshChunk = createMeshChunk(data, mesh);

  meshChunk->m_diffuseMap = (Texture*)g_Game->getRenderSystem()->getTextureManager()->load("Data/Textures/sky.tga");

  mesh.initDummyMaterial();

  return true;
}