#ifndef __RenderSystem_h_
#define __RenderSystem_h_

#include "RenderSystemPrerequisites.h"
#include "Texture.h" // Manager

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

  ID3D11Device* getDevicePtr() const { return m_device; }
  ID3D11DeviceContext* getDeviceContextPtr() const { return m_renderContext; }

  TextureManager* getTextureManager() { return &m_textureManager; }

  // render state api
  void setRasterizerState(ID3D11RasterizerState* rasterizerState);

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

  //// pipeline state //
  SharedPtr<RenderTarget> m_boundRenderTargets[MAX_RENDER_TARGETS];
  SharedPtr<RenderTarget> m_boundDepthStencilTarget;
  uint32 m_renderTargetDirtyMask;
  ////

  // managers
  TextureManager m_textureManager;
};

class ShaderCompiler
{
public:
  static bool compile(const String& fileName, const String& entryPoint, const String& target, DataBlob& byteCode);
};

#endif // __RenderSystem_h_