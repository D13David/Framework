#include "Core.h"
#include "RenderSystem.h"
#include "RenderTarget.h"
#include "InitParams.h"
#include "ShaderDrawBundle.h"
#include "Shader.h"

#if defined (SUPPORT_GPU_DEBUG_MARKERS)
# include <d3d9.h>
# pragma comment(lib, "d3d9.lib")
#endif // SUPPORT_GPU_DEBUG_MARKERS

class PipelineStateCache
{
public:
  PipelineStateCache(ID3D11DeviceContext* deviceContext);

  void setRenderTargets(uint32 numViews, ID3D11RenderTargetView* const * renderTargetViews, ID3D11DepthStencilView* depthStencilView);
  void setRasterizerState(ID3D11RasterizerState* rasterizerState);
  void depthStencilState(ID3D11DepthStencilState* depthStencilState, uint8 stencilRef);
  void setVertexShader(const VertexShader& vertexShader);
  void setPixelShader(const PixelShader& pixelShader);
  void setInputLayout(ID3D11InputLayout* inputLayout);
  void setViewport(D3D11_VIEWPORT viewport);

private:
  ID3D11DeviceContext* m_deviceContext;

  // currently bound render targets
  ID3D11RenderTargetView* m_currentlyBoundRTV[MAX_RENDER_TARGETS];
  ID3D11DepthStencilView* m_currentlyBoundDSV;

  // currently bound state objects
  ID3D11RasterizerState* m_currentRasterizerState;
  ID3D11DepthStencilState* m_currentDepthStencilState;
  ID3D11InputLayout* m_currentInputLayout;

  // current shader state
  ID3D11VertexShader* m_currentVertexShader;
  ID3D11PixelShader* m_currentPixelShader;

  uint32 m_stencilRef;
  D3D11_VIEWPORT m_currentViewport;
};

PipelineStateCache::PipelineStateCache(ID3D11DeviceContext* deviceContext)
  : m_deviceContext(deviceContext)
  , m_currentlyBoundDSV(0)
  , m_currentRasterizerState(0)
  , m_currentDepthStencilState(0)
  , m_currentInputLayout(0)
  , m_currentVertexShader(0)
  , m_currentPixelShader(0)
  , m_stencilRef(0xff)
{
  ASSERT(m_deviceContext != 0, "no valid device context");
  memset(m_currentlyBoundRTV, 0, sizeof(m_currentlyBoundRTV));
  memset(&m_currentViewport, 0, sizeof(m_currentViewport));
}

void PipelineStateCache::setRenderTargets(uint32 numViews, ID3D11RenderTargetView* const * renderTargetViews, ID3D11DepthStencilView* depthStencilView)
{
  bool hasChanges = false;

  for (uint32 i = 0; i < MAX_RENDER_TARGETS; ++i)
  {
    hasChanges |= renderTargetViews[i] != m_currentlyBoundRTV[i];
    m_currentlyBoundRTV[i] = renderTargetViews[i];
  }

  if (depthStencilView != m_currentlyBoundDSV)
  {
    hasChanges = true;
    m_currentlyBoundDSV = depthStencilView;
  }

  if (hasChanges)
  {
    m_deviceContext->OMSetRenderTargets(numViews, m_currentlyBoundRTV, m_currentlyBoundDSV);
  }
}

void PipelineStateCache::setRasterizerState(ID3D11RasterizerState* rasterizerState)
{
  if (rasterizerState != m_currentRasterizerState)
  {
    m_currentRasterizerState = rasterizerState;
    m_deviceContext->RSSetState(m_currentRasterizerState);
  }
}

void PipelineStateCache::depthStencilState(ID3D11DepthStencilState* depthStencilState, uint8 stencilRef)
{
  if (depthStencilState != m_currentDepthStencilState || m_stencilRef != stencilRef)
  {
    m_currentDepthStencilState = depthStencilState;
    m_deviceContext->OMSetDepthStencilState(depthStencilState, stencilRef);
  }
}

void PipelineStateCache::setVertexShader(const VertexShader& vertexShader)
{
  // FIXME: cache this
  ID3D11Buffer* const* constantBuffers;
  uint32 numConstantBuffers = vertexShader.queryBuffersArray(constantBuffers);
  m_deviceContext->VSSetConstantBuffers(0, numConstantBuffers, constantBuffers);

  ID3D11VertexShader* shaderResourcePtr = vertexShader.getResourcePtr();
  if (shaderResourcePtr != m_currentVertexShader)
  {
    m_currentVertexShader = shaderResourcePtr;
    m_deviceContext->VSSetShader(shaderResourcePtr, NULL, 0);
  }
}

void PipelineStateCache::setPixelShader(const PixelShader& pixelShader)
{
  // FIXME: cache this
  ID3D11Buffer* const* constantBuffers;
  uint32 numConstantBuffers = pixelShader.queryBuffersArray(constantBuffers);
  m_deviceContext->PSSetConstantBuffers(0, numConstantBuffers, constantBuffers);

  ID3D11PixelShader* shaderResourcePtr = pixelShader.getResourcePtr();
  if (shaderResourcePtr != m_currentPixelShader)
  {
    m_currentPixelShader = shaderResourcePtr;
    m_deviceContext->PSSetShader(shaderResourcePtr, NULL, 0);
  }
}

void PipelineStateCache::setInputLayout(ID3D11InputLayout* inputLayout)
{
  if (inputLayout != m_currentInputLayout)
  {
    m_currentInputLayout = inputLayout;
    m_deviceContext->IASetInputLayout(inputLayout);
  }
}

void PipelineStateCache::setViewport(D3D11_VIEWPORT viewport)
{
  const bool needsChange = (memcmp(&m_currentViewport, &viewport, sizeof(D3D11_VIEWPORT)) != 0);
  if (needsChange)
  {
    memcpy(&m_currentViewport, &viewport, sizeof(D3D11_VIEWPORT));
    m_deviceContext->RSSetViewports(1, &m_currentViewport);
  }
}

RenderSystem::RenderSystem()
  : m_device(0)
  , m_renderContext(0)
  , m_swapChain(0)
  , m_depthBuffer(0)
  , m_backBufferDSV(0)
  , m_backBufferRTV(0)
  , m_isFullScreen(false)
  , m_stateCache(0)
{
}

RenderSystem::~RenderSystem()
{
}

bool RenderSystem::init(void* windowHandle, const InitParams& params)
{
  IDXGIFactory1* factory = 0;
  IDXGIOutput* output = 0;

  DXGI_MODE_DESC modeDesc;
  ::ZeroMemory(&modeDesc, sizeof(DXGI_MODE_DESC));
  modeDesc.Format = params.srgbTarget ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
  modeDesc.Width = params.width;
  modeDesc.Height = params.height;

  if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory)))
  {
    IDXGIAdapter1* adapter = 0;
    for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
      if (adapter->CheckInterfaceSupport(__uuidof(ID3D11Device), NULL))
        break;

      adapter->Release();
    }

    D3D_FEATURE_LEVEL supportedFeatureLevels[] =
    {
      D3D_FEATURE_LEVEL_11_0,
    };

    UINT flags = 0;

#if defined (_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

    if (adapter)
    {
      if (SUCCEEDED(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags, supportedFeatureLevels, _countof(supportedFeatureLevels),
        D3D11_SDK_VERSION, &m_device, NULL, &m_renderContext)))
      {
        for (UINT i = 0; adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; ++i)
        {
          if (SUCCEEDED(output->FindClosestMatchingMode(&modeDesc, &modeDesc, m_device)))
          {
            // additional checks??
            break;
          }

          output->Release();
          output = 0;
        }

        /*if (output)
          output->Release();*/
      }

      adapter->Release();
    }

    if (m_device)
    {
      DXGI_SWAP_CHAIN_DESC sd;
      ::ZeroMemory(&sd, sizeof(DXGI_SWAP_CHAIN_DESC));
      sd.BufferCount = 1;
      sd.BufferDesc = modeDesc;
      sd.BufferUsage = D3D11_BIND_RENDER_TARGET;
      sd.Flags = 0;
      sd.OutputWindow = (HWND)windowHandle;
      sd.SampleDesc.Quality = 0;
      sd.SampleDesc.Count = 1;
      sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      sd.Windowed = TRUE;

      if (params.msaaSamples > 0)
      {
        UINT numLevels = 0;
        if (SUCCEEDED(m_device->CheckMultisampleQualityLevels(modeDesc.Format, params.msaaSamples, &numLevels)) && numLevels > 0)
        {
          sd.SampleDesc.Quality = numLevels-1;
          sd.SampleDesc.Count = params.msaaSamples;
        }
        else
        {
          printf("multisample quality not supported");
        }
      }

      VALIDATE(factory->CreateSwapChain(m_device, &sd, &m_swapChain));
    }

    if (params.fullscreen)
    {
      m_isFullScreen = SUCCEEDED(m_swapChain->SetFullscreenState(TRUE, output));
    }

    // setup debug queue

    factory->MakeWindowAssociation((HWND)windowHandle, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

    if (output)
      output->Release();

    factory->Release();
  }

  m_stateCache = new PipelineStateCache(m_renderContext);

  createFrameBuffer();

  initDefaultResources();

  return (m_device && m_swapChain);
}

void RenderSystem::shutdown()
{
  delete m_stateCache;
  m_stateCache = 0;

  if (m_isFullScreen && m_swapChain)
    m_swapChain->SetFullscreenState(FALSE, NULL);

  if (m_renderContext)
    m_renderContext->ClearState();

  releaseFrameBuffer();
  
  SAFE_RELEASE(m_swapChain);
  SAFE_RELEASE(m_renderContext);
  SAFE_RELEASE(m_device);
}

void RenderSystem::createFrameBuffer()
{
  releaseFrameBuffer();

  ID3D11Texture2D* backBuffer;
  if (SUCCEEDED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
  {
    VALIDATE(m_device->CreateRenderTargetView(backBuffer, NULL, &m_backBufferRTV));
    backBuffer->Release();
  }

  DXGI_SWAP_CHAIN_DESC sd;
  m_swapChain->GetDesc(&sd);

  D3D11_TEXTURE2D_DESC desc;
  ::ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
  desc.ArraySize = 1;
  desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
  desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
  desc.Width = sd.BufferDesc.Width;
  desc.Height = sd.BufferDesc.Height;
  desc.MipLevels = sd.SampleDesc.Count > 0 ? 1 : 0;
  desc.SampleDesc = sd.SampleDesc;
  desc.Usage = D3D11_USAGE_DEFAULT;

  if (SUCCEEDED(m_device->CreateTexture2D(&desc, NULL, &m_depthBuffer)))
  {
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ::ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    descDSV.ViewDimension = sd.SampleDesc.Count > 0 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    VALIDATE(m_device->CreateDepthStencilView(m_depthBuffer, &descDSV, &m_backBufferDSV));
  }

  m_stateCache->setRenderTargets(1, &m_backBufferRTV, m_backBufferDSV);

  setViewport((float)sd.BufferDesc.Width, (float)sd.BufferDesc.Height);
}

void RenderSystem::releaseFrameBuffer()
{
  SAFE_RELEASE(m_backBufferRTV);
  SAFE_RELEASE(m_backBufferDSV);
  SAFE_RELEASE(m_depthBuffer);
}

void RenderSystem::clearRenderTargets(int clearBits, float depthValue, int stencilValue)
{
  ID3D11RenderTargetView* rtvs[8] = {0};
  ID3D11DepthStencilView* dsv = 0;

  if (clearBits & (RT_DEPTH | RT_STENCIL))
    m_renderContext->OMGetRenderTargets(8, rtvs, &dsv);
  else
    m_renderContext->OMGetRenderTargets(8, rtvs, NULL);

  float clearColor[] = {0, 0, 0, 1};

  for (int i = 0; i < 8; ++i)
  {
    if (rtvs[i] && clearBits & (1 << i))
    {
      m_renderContext->ClearRenderTargetView(rtvs[i], clearColor);
      rtvs[i]->Release();
    }
  }

  if (dsv)
  {
    UINT flags = 0;
    if (clearBits & RT_DEPTH)
      flags |= D3D11_CLEAR_DEPTH;
    if (clearBits & RT_STENCIL)
      flags |= D3D11_CLEAR_STENCIL;

    m_renderContext->ClearDepthStencilView(dsv, flags, depthValue, stencilValue);
    dsv->Release();
  }
}

void RenderSystem::beginFrame()
{
}

void RenderSystem::endFrame()
{
  m_swapChain->Present(1, 0);
}

void RenderSystem::beginRenderTargetSetup()
{
  for (uint32 i = 0; i < MAX_RENDER_TARGETS; ++i)
  {
    m_boundRenderTargets[i] = 0;
  }

  m_boundDepthStencilTarget = 0;
}

#define DEPTH_STENCIL_TARGET_BIT (MAX_RENDER_TARGETS+1)

void RenderSystem::bindRenderTarget(uint32 slotMask, const SharedPtr<RenderTarget>& renderTarget)
{
  if (slotMask & (RT_DEPTH | RT_STENCIL))
  {
    ASSERT(renderTarget->getDefaultDSV() != 0, "target is not a depthstencil target");
    m_boundDepthStencilTarget = renderTarget;
  }
  else
  {
    for (uint32 i = RT_RENDER_TARGET0, slot = 0; i < RT_RENDER_TARGET7 && slotMask != 0; i <<= 1, ++slot)
    {
      if ((slotMask & i) && (m_boundRenderTargets[slot] != renderTarget))
      {
        m_boundRenderTargets[slot] = renderTarget;
        slotMask &= ~i;
      }
    }
  }
}

void RenderSystem::endRenderTargetSetup()
{
  ID3D11RenderTargetView* renderTargetViews[MAX_RENDER_TARGETS] = {0};

  uint32 numRenderTargets = 0;

  for (uint32 i = 0; i < MAX_RENDER_TARGETS; ++i)
  {
    renderTargetViews[i] = m_boundRenderTargets[i] ? m_boundRenderTargets[i]->getDefaultRTV() : 0;
    if (renderTargetViews[i])
    {
      numRenderTargets = i+1;
    }
  }

  if (numRenderTargets > 0)
  {
    m_stateCache->setRenderTargets(numRenderTargets, renderTargetViews, 
      m_boundDepthStencilTarget ? m_boundDepthStencilTarget->getDefaultDSV() : 0);
  }
}

void RenderSystem::setShaderDrawBundle(const ShaderDrawBundle* shaderDrawBundle)
{
  ASSERT(shaderDrawBundle != 0, "invalid value");
  m_stateCache->setVertexShader(*shaderDrawBundle->getVertexShader());
  m_stateCache->setPixelShader(*shaderDrawBundle->getPixelShader());
  m_stateCache->setInputLayout(shaderDrawBundle->getInputLayout());
}

void RenderSystem::initDefaultResources()
{
}

void RenderSystem::setRasterizerState(ID3D11RasterizerState* rasterizerState)
{
  m_stateCache->setRasterizerState(rasterizerState);
}

void RenderSystem::setDepthStencilState(ID3D11DepthStencilState* depthStencilState, uint8 stencilRef)
{
  m_stateCache->depthStencilState(depthStencilState, stencilRef);
}

void RenderSystem::setViewport(float width, float height, float topLeftX, float topLeftY, float minZ, float maxZ)
{
  D3D11_VIEWPORT vp = 
  {
    topLeftX, topLeftY,
    width, height,
    minZ, maxZ
  };

  m_stateCache->setViewport(vp);
}

void RenderSystem::debugEventPush(const char* name, uint32 color)
{
#if defined (SUPPORT_GPU_DEBUG_MARKERS)
  WCHAR buffer[255] = {0};
  MultiByteToWideChar(CP_ACP, 0, name, -1, buffer, 255);

  ID3DUserDefinedAnnotation* annotation = 0;
  if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&annotation)))
  {
    annotation->BeginEvent(buffer);
    annotation->Release();
  }
  else
  {
    D3DPERF_BeginEvent(color, buffer);
  }
#endif // SUPPORT_GPU_DEBUG_MARKERS
}

void RenderSystem::debugEventPop()
{
#if defined (SUPPORT_GPU_DEBUG_MARKERS)
  ID3DUserDefinedAnnotation* annotation = 0;
  if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&annotation)))
  {
    annotation->EndEvent();
    annotation->Release();
  }
  else
  {
    D3DPERF_EndEvent();
  }
#endif // SUPPORT_GPU_DEBUG_MARKERS
}

void RenderSystem::debugMarker(const char* name, uint32 color)
{
#if defined (SUPPORT_GPU_DEBUG_MARKERS)
  WCHAR buffer[255] = {0};
  MultiByteToWideChar(CP_ACP, 0, name, -1, buffer, 255);

  ID3DUserDefinedAnnotation* annotation = 0;
  if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&annotation)))
  {
    annotation->SetMarker(buffer);
    annotation->Release();
  }
  else
  {
    D3DPERF_SetMarker(color, buffer);
  }
#endif // SUPPORT_GPU_DEBUG_MARKERS
}

struct IncludeHandler : public ID3DInclude
{
};

bool ShaderCompiler::compile(const String& fileName, const String& entryPoint, const String& target, DataBlob& byteCode)
{
  DataBlob buffer;
  if (!readRawBlob(fileName, buffer))
    return false;

  UINT flags = D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

#if defined (_DEBUG)
  flags |= (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION);
#else
  flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif // _DEBUG

  ID3DBlob* shaderByteCode = 0;
  ID3DBlob* errorMsgs = 0;
  if (FAILED(D3DCompile(buffer.getPtr(), buffer.getSize(), NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
    entryPoint.c_str(), target.c_str(), flags, 0, &shaderByteCode, &errorMsgs)))
  {
    // parse error message
    const char* msg = (const char*)errorMsgs->GetBufferPointer();
    MessageBox(NULL, msg, "Error", MB_OK);
    if (errorMsgs)
      errorMsgs->Release();
  }
  else
  {
    byteCode.allocate(shaderByteCode->GetBufferSize());
    memcpy(byteCode.getPtr(), shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize());
    shaderByteCode->Release();
  }

  return true;
}