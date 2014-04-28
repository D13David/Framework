#ifndef __VertexDeclaration_h_
#define __VertexDeclaration_h_

#include "RenderSystemPrerequisites.h"

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