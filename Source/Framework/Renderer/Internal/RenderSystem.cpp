#include "Core.h"
#include "RenderSystem.h"
#include "RenderTarget.h"
#include "InitParams.h"


RenderSystem::RenderSystem()
  : m_device(0)
  , m_renderContext(0)
  , m_swapChain(0)
  , m_depthBuffer(0)
  , m_backBufferDSV(0)
  , m_backBufferRTV(0)
  , m_isFullScreen(false)
  , m_renderTargetDirtyMask(0)
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

  createFrameBuffer();

  initDefaultResources();

  return (m_device && m_swapChain);
}

void RenderSystem::shutdown()
{
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

  m_renderContext->OMSetRenderTargets(1, &m_backBufferRTV, m_backBufferDSV);

  D3D11_VIEWPORT vp = {0};
  vp.Width = (FLOAT)sd.BufferDesc.Width;
  vp.Height = (FLOAT)sd.BufferDesc.Height;
  vp.MaxDepth = 1.0f;
  vp.MinDepth = 0.0f;
  vp.TopLeftX = 0.0f;
  vp.TopLeftY = 0.0f;
  m_renderContext->RSSetViewports(1, &vp);
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

}

#define DEPTH_STENCIL_TARGET_BIT (MAX_RENDER_TARGETS+1)

void RenderSystem::bindRenderTarget(uint32 slotMask, const SharedPtr<RenderTarget>& renderTarget)
{
  if (slotMask & (RT_DEPTH | RT_STENCIL))
  {
    ASSERT(renderTarget->getDefaultDSV() != 0, "target is not a depthstencil target");
    m_boundDepthStencilTarget = renderTarget;
    m_renderTargetDirtyMask |= (1 << DEPTH_STENCIL_TARGET_BIT);
  }
  else
  {
    for (uint32 i = RT_RENDER_TARGET0, slot = 0; i < RT_RENDER_TARGET8 && slotMask != 0; i <<= 1, ++slot)
    {
      if ((slotMask & i) && (m_boundRenderTargets[slot] != renderTarget))
      {
        m_boundRenderTargets[slot] = renderTarget;
        m_renderTargetDirtyMask |= i;
        slotMask &= ~i;
      }
    }
  }
}

void RenderSystem::endRenderTargetSetup()
{
  if (m_renderTargetDirtyMask != 0)
  {
    ID3D11RenderTargetView* renderTargetViews[MAX_RENDER_TARGETS] = {0};

    uint32 numRenderTargets = 0;

    for (uint32 i = 0; i < MAX_RENDER_TARGETS; ++i)
    {
      if (m_renderTargetDirtyMask & (1 << i))
      {
        numRenderTargets = i+1;
      }

      renderTargetViews[i] = m_boundRenderTargets[i] ? m_boundRenderTargets[i]->getDefaultRTV() : 0;
    }

    if (numRenderTargets > 0 || (m_renderTargetDirtyMask & DEPTH_STENCIL_TARGET_BIT) != 0)
    {
      m_renderContext->OMSetRenderTargets(numRenderTargets, renderTargetViews,  
        m_boundDepthStencilTarget ? m_boundDepthStencilTarget->getDefaultDSV() : 0);
    }

    m_renderTargetDirtyMask = 0;
  }
}

void RenderSystem::initDefaultResources()
{
}

void RenderSystem::setRasterizerState(ID3D11RasterizerState* rasterizerState)
{
  ASSERT(rasterizerState != 0, "invalid value");
  m_renderContext->RSSetState(rasterizerState);
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