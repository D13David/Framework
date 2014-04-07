#ifndef __Core_h_
#define __Core_h_

#include <string>
#include <cmath>
#include <vector>
#include <map>
#include <hash_map>
#include <stack>

/////////////////////////////////////////////////////////////////////
//  compile options

#define SUPPORT_RUNTIME_SHADER_COMPILE

// windows/dx
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <dxgi.h>
#include <d3d11.h>
#if defined (SUPPORT_RUNTIME_SHADER_COMPILE)
# include <D3Dcompiler.h>
# pragma comment(lib, "d3dcompiler.lib")
# pragma comment(lib, "dxguid.lib")
#endif // SUPPORT_RUNTIME_SHADER_COMPILE

typedef std::string String;

typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned char ubyte;

class DataBlob
{
public:
  DataBlob();
  ~DataBlob();

  void allocate(int size);
  void free();
  void* getPtr() const { return m_data; }
  int getSize() const { return m_size; }

private:
  void* m_data;
  int m_size;
};

long fileSize(FILE* fp);
bool readRawBlob(const String& fileName, DataBlob& data);
uint16 readUInt16(const ubyte*& ptr);
uint32 readUInt32(const ubyte*& ptr);

#define VALIDATE(x) { \
  if (FAILED((x))) { MessageBox(NULL, #x " failed...", "Error", MB_OK); exit(0); } }

#define SAFE_RELEASE(x) if ((x)) { (x)->Release(); (x) = 0; }

#define ASSERT(x, msg)

#define RENDER_DEVICE g_Game->getRenderSystem()->getDevicePtr()
#define RENDER_CONTEXT g_Game->getRenderSystem()->getDeviceContextPtr()

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11Resource;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

class DebugGeometryRenderer;
class Game;
class GameClient;
struct InitParams;
class InputSystem;
class PixelShader;
class RenderSystem;
class RenderTarget;
class Shader;
class Texture;
class VertexDeclaration;
class VertexShader;

#include "Vector.h"
#include "Matrix.h"
#include "MathUtil.h"
#include "Point.h"
#include "Ptr.h"

extern Game* g_Game;

#endif // __Core_h_