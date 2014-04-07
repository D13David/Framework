#ifndef __VertexDeclaration_h_
#define __VertexDeclaration_h_

enum eVertexElementFormat
{
  VEF_NONE,
  // 32 bit float formats
  VEF_FLOAT1,
  VEF_FLOAT2,
  VEF_FLOAT3,
  VEF_FLOAT4,
  // 16 bit float formats
  VEF_HALF1,
  VEF_HALF2,
  VEF_HALF4,
  // 16 bit integer formats
  VEF_SHORT2,
  VEF_SHORT4,
  VEF_SHORT4N,
  // special formats
  VEF_COLOR,

  VEF_MAX
};

struct VertexElement
{
  String semantic;
  uint32 semanticIndex;
  eVertexElementFormat format;
  uint32 stream;
  uint32 byteOffset;
  bool usePerInstance;
};

class VertexDeclaration
{
public:
  static const uint32 MaxVertexElements = 16;

  VertexDeclaration();

  void clear();
  void add(const VertexElement* elements, uint32 numElements);
  void add(const VertexElement& element);
  const VertexElement& add(const String semantic, uint32 semanticIndex, eVertexElementFormat format, uint32 stream, uint32 byteOffset, bool usePerInstance);

  const VertexElement* getElement(uint32 index) const;

  static uint32 sizeOfElementType(eVertexElementFormat format);

private:
  VertexElement m_elements[MaxVertexElements];
  uint32 m_numElements;
};

#endif // __VertexDeclaration_h_