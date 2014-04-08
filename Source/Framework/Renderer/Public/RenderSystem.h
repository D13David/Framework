#ifndef __RenderSystem_h_
#define __RenderSystem_h_

#include "RenderSystemPrerequisites.h"
#include "Texture.h" // Manager

class PipelineStateCache;

#if defined (SUPPORT_GPU_DEBUG_MARKERS)
# define GPU_DEBUG_EVENT_PUSH(msg) { g_Game->getRenderSystem()->debugEventPush((msg), 0); }
# define GPU_DEBUG_EVENT_POP() { g_Game->getRenderSystem()->debugEventPop(); }
# define GPU_DEBUG_MARKER(msg) { g_Game->getRenderSystem()->debugMarker((msg), 0); }
#else
# define GPU_DEBUG_EVENT_PUSH(msg)
# define GPU_DEBUG_EVENT_POP(msg)
# define GPU_DEBUG_MARKER(msg)
#endif // SUPPORT_GPU_DEBUG_MARKERS

class RenderSystem
{
public:
  RenderSystem();
  ~RenderSystem();

  bool init(void* windowHandle, const InitParams& params);
  void shutdown();
  void clearRenderTargets(int clearBits = RT_ALL_TARGETS, float depthValue = 1.0f, int stencilValue = 0.0f);
  void beginFrame();
  void endFrame();

  void beginRenderTargetSetup();
  void bindRenderTarget(uint32 slot, const SharedPtr<RenderTarget>& renderTarget);
  void endRenderTargetSetup();

  void setShaderDrawBundle(const ShaderDrawBundle* shaderDrawBundle);

  ID3D11Device* getDevicePtr() const { return m_device; }
  ID3D11DeviceContext* getDeviceContextPtr() const { return m_renderContext; }

  TextureManager* getTextureManager() { return &m_textureManager; }

  // render state api
  void setRasterizerState(ID3D11RasterizerState* rasterizerState);
  void setDepthStencilState(ID3D11DepthStencilState* depthStencilState, uint8 stencilRef = 0xff);
  void setViewport(float width, float height, float topLeftX = 0.0f, float topLeftY = 0.0f, float minZ = 0.0f, float maxZ = 1.0f);

  // debugging
  void debugEventPush(const char* name, uint32 color);
  void debugEventPop();
  void debugMarker(const char* name, uint32 color);

private:
  void createFrameBuffer();
  void releaseFrameBuffer();
  void initDefaultResources();

  ID3D11Device* m_device;
  ID3D11DeviceContext* m_renderContext;
  IDXGISwapChain* m_swapChain;

  ID3D11Texture2D* m_depthBuffer;
  ID3D11DepthStencilView* m_backBufferDSV;
  ID3D11RenderTargetView* m_backBufferRTV;

  bool m_isFullScreen;

  //// pipeline state
  SharedPtr<RenderTarget> m_boundRenderTargets[MAX_RENDER_TARGETS];
  SharedPtr<RenderTarget> m_boundDepthStencilTarget;

  // managers
  TextureManager m_textureManager;
  PipelineStateCache* m_stateCache;
};

class ShaderCompiler
{
public:
  static bool compile(const String& fileName, const String& entryPoint, const String& target, DataBlob& byteCode);
};

#endif // __RenderSystem_h_