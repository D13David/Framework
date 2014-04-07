#ifndef __GameClient_h_
#define __GameClient_h_

struct SceneView
{
  Matrix4 m_viewMatrix;
  Matrix4 m_projectionMatrix;
  Vector3 m_worldPosition;
};

class GameMode
{
public:
  virtual void tick(float time, GameClient& client) = 0;

  /// this is only temporary for test use, remove this
  virtual void getCurrentView(SceneView& view) = 0;

protected:
  virtual void handleUserInput() = 0;

private:
};

class GameClient
{
public:
  GameClient();
  ~GameClient();

  virtual void initResources() {}

  void tick(float time);
  void render(float time);
  void pushGameMode(WeakPtr<GameMode> gameMode);
  void popGameMode();
  void getCurrentView(SceneView& view);

protected:
  virtual void tickIntern(float time) {}
  virtual void renderIntern(float time) {}

  std::stack<WeakPtr<GameMode>> m_gameModeStack; // FIXME: should hold WeakPtr
  WeakPtr<GameMode> m_currentGameMode;
};

#endif // __GameClient_h_