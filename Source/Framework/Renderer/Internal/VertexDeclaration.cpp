#include "Core.h"
#include "VertexDeclaration.h"

VertexDeclaration::VertexDeclaration()
  : m_numElements(0)
{
}

void VertexDeclaration::clear()
{
  m_numElements = 0;
}

void VertexDeclaration::add(const VertexElement* elements, uint32 numElements)
{
  for (uint32 i = 0; i < numElements; ++i)
  {
    ASSERT(m_numElements < MaxVertexElements-1, "element array full");
    m_elements[m_numElements++] = elements[i];
  }
}

void VertexDeclaration::add(const VertexElement& element)
{
  ASSERT(m_numElements < MaxVertexElements-1, "element array full");
  m_elements[m_numElements++] = element;
}

const VertexElement& VertexDeclaration::add(const String semantic, uint32 semanticIndex, eVertexElementFormat format, uint32 stream, uint32 byteOffset, bool usePerInstance)
{
  VertexElement element;
  element.semantic = semantic;
  element.semanticIndex = semanticIndex;
  element.format = format;
  element.stream = stream;
  element.byteOffset = byteOffset;
  element.usePerInstance = usePerInstance;

  add(element);

  return m_elements[m_numElements-1];
}

const VertexElement* VertexDeclaration::getElement(uint32 index) const
{
  ASSERT(index < MaxVertexElements-1, "element array full");
  if (index < m_numElements)
    return &m_elements[index];

  return 0;
}

uint32 VertexDeclaration::sizeOfElementType(eVertexElementFormat format)
{
  switch (format)
  {
  case VEF_NONE: return 0;
  // 32 bit float formats
  case VEF_FLOAT1: return sizeof(float);
  case VEF_FLOAT2: return sizeof(float)*2;
  case VEF_FLOAT3: return sizeof(float)*3;
  case VEF_FLOAT4: return sizeof(float)*4;
  // 16 bit float formats
  case VEF_HALF1: return sizeof(short);
  case VEF_HALF2: return sizeof(short)*2;
  case VEF_HALF4: return sizeof(short)*4;
  // 16 bit integer formats
  case VEF_SHORT2: return sizeof(short)*2;
  case VEF_SHORT4: return sizeof(short)*4;
  case VEF_SHORT4N: return sizeof(short)*4;
  // special formats
  case VEF_COLOR: return sizeof(uint32);
  }

  return 0;
}