#include "Core.h"
#include "Game.h"

#include "RenderSystem.h"
#include "InputSystem.h"
#include "InitParams.h"
#include "DebugGeometryRenderer.h"
#include "GameClient.h"
#include "StringUtils.h"
#include "SystemTextures.h"

// unit tests
#include "PtrTest.h"

#include <Windows.h>
#include <windowsx.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

Game::Game()
{
}

Game::~Game()
{
}

bool Game::init(InitParams& params)
{
  // try setting up correct working directory
  String relativeWorkingDir;
  if (readAllFile("working_dir", relativeWorkingDir))
  {
    String currentDir = getCurrentDirectory();
    if (!StringUtils::endsWith(currentDir, "/") && !StringUtils::endsWith(currentDir, "\\"))
      currentDir += "/";
    currentDir += relativeWorkingDir;
    setCurrentDirectory(currentDir);
  }

  // run unit tests
  //PtrTest::TestSharedPointer<PT_FAST>();

  if (!initGame(params))
    return false;

  // init subsystems
  m_inputSystem = new InputSystem();
  if (!m_inputSystem->init())
    return false;

  if (!createRenderWindow(params))
    return false;

  m_renderSystem = new RenderSystem();
  if (!m_renderSystem->init(m_windowHandle, params))
    return false;

#if defined (_DEBUG)
  m_debugGeomRenderer = new DebugGeometryRenderer();
  m_debugGeomRenderer->init();
#endif // _DEBUG

  SystemTextures::init();

  m_client->initResources();

  return true;
}

void Game::shutdown()
{
}

void Game::run()
{
  MSG msg;

  while (true)
  {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
        break;

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      m_inputSystem->tick(0);

#if defined (_DEBUG)
      m_debugGeomRenderer->prepareDebugRendering();
#endif // _DEBUG

      m_client->tick(0);

      m_renderSystem->beginFrame();
      m_client->render(0);

#if defined (_DEBUG)
      SceneView view;
      m_client->getCurrentView(view);
      m_debugGeomRenderer->drawAllDebugGeometry(view.m_viewMatrix, view.m_projectionMatrix);
#endif // _DEBUG

      m_renderSystem->endFrame();
    }
  }
}

bool Game::createRenderWindow(const InitParams& params)
{
  WNDCLASS wc = {0};
  wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpfnWndProc = &WndProc;
  wc.lpszClassName = "FrameworkGameWindow";
  wc.style = CS_HREDRAW | CS_VREDRAW;

  if (!RegisterClass(&wc))
    return false;

  DWORD dwStyle = WS_OVERLAPPEDWINDOW;

  int width = params.width;
  int height = params.height;

  if (!params.fullscreen)
  {
    RECT rc = {0, 0, width, height};
    if (AdjustWindowRect(&rc, dwStyle, FALSE) == TRUE)
    {
      width = rc.right - rc.left;
      height = rc.bottom - rc.top;
    }
  }

  HWND hWnd = CreateWindow("FrameworkGameWindow", params.gameTitle, WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL);
  if (!hWnd)
    return false;

  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);
  SetForegroundWindow(hWnd);

  m_windowHandle = hWnd;
  m_viewportWidth = width;
  m_viewportHeight = height;

  return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_CLOSE:
    PostQuitMessage(0);
    break;
  case WM_KEYDOWN:
    g_Game->getInputSystem()->onKeyDown(wParam, (lParam & 0xf) != 0);
    break;
  case WM_KEYUP:
    g_Game->getInputSystem()->onKeyUp(wParam);
    break;
  

  default:
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }

  return 0;
}

int main()
{
  InitParams params;

  params.gameTitle = "Framework Window";
  params.width = 1024;
  params.height = 768;
  params.fullscreen = false;
  params.msaaSamples = 4;
  params.srgbTarget = true;

  if (!g_Game->init(params))
    return 0;

  g_Game->run();

  g_Game->shutdown();
}