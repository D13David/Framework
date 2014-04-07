#ifndef __Game_h_
#define __Game_h_

class Game
{
public:
  Game();
  virtual ~Game();

  bool init(InitParams& params);
  void shutdown();
  void run();

  WeakPtr<RenderSystem> getRenderSystem() const { return m_renderSystem; }
  WeakPtr<InputSystem> getInputSystem() const { return m_inputSystem; }
#if defined (_DEBUG)
  WeakPtr<DebugGeometryRenderer> getDebugGeometryRenderer() const { return m_debugGeomRenderer; }
#endif // _DEBUG

  uint32 getViewportWidth() const { return m_viewportWidth; }
  uint32 getViewportHeight() const { return m_viewportHeight; }

protected:
  virtual bool initGame(InitParams& params) { return true; }
  
  bool createRenderWindow(const InitParams& params);

  SharedPtr<RenderSystem> m_renderSystem;
  SharedPtr<InputSystem> m_inputSystem;
#if defined (_DEBUG)
  SharedPtr<DebugGeometryRenderer> m_debugGeomRenderer;
#endif // _DEBUG

  void* m_windowHandle;
  uint32 m_viewportWidth;
  uint32 m_viewportHeight;
  SharedPtr<GameClient> m_client; 
};

#endif // __Game_h_