#include "Core.h"
#include "GameClient.h"
#include "InputSystem.h"
#include "Game.h"
#include "Matrix.h"

#include <windows.h>

// implements game mode where the player can freely move through the world
class DefaultGameMode : public GameMode
{
public:
  DefaultGameMode();
  void tick(float time, GameClient& client);
  void getCurrentView(SceneView& view);

protected:
  virtual void handleUserInput();

  // this is only for testing
  Matrix4 m_viewMatrix;
  Matrix4 m_projectionMatrix;
  Vector3 m_viewPos;
  float m_viewYaw;
  float m_viewPitch;
};

DefaultGameMode::DefaultGameMode()
  : m_viewYaw(0)
  , m_viewPitch(0)
{
  m_viewPos = Vector3(0, 100, 1000);
  m_viewMatrix = Matrix4::Identity;
  makePerspectiveProjMatrix(m_projectionMatrix, 60.0f * DEG2RAD, (float)1024.0f / 768.0f, 10.0f, 50000.0f);
}

void DefaultGameMode::tick(float time, GameClient& client)
{
  handleUserInput();
}

void DefaultGameMode::handleUserInput()
{
  WeakPtr<InputSystem> inputSystem = g_Game->getInputSystem();

  const Point& mouseMoveDelta = inputSystem->getMouseMovement();

  static const float CameraMoveSpeed = 10.0f;
  static const float MouseMoveSpeed = 1.0f / 5.0f;

  float angularMovementX = mouseMoveDelta.x * MouseMoveSpeed;
  float angularMovementY = mouseMoveDelta.y * MouseMoveSpeed;
  float moveSpeedForward = 0.0f;
  float moveSpeedStrafe = 0.0f;
  float moveSpeedUp = 0.0f;
  
  const bool mouseButtonRightDown = inputSystem->isKeyDown(Keys::RightMouseButton);
  const bool mouseButtonLeftDown = inputSystem->isKeyDown(Keys::LeftMouseButton);
  const bool mouseButtonMiddleDown = inputSystem->isKeyDown(Keys::MiddleMouseButton);

  if (mouseButtonRightDown || mouseButtonLeftDown || mouseButtonMiddleDown)
  {
    if ((mouseButtonRightDown && mouseButtonLeftDown) || mouseButtonMiddleDown)
    {
      moveSpeedUp -= CameraMoveSpeed * angularMovementY;
      moveSpeedStrafe += CameraMoveSpeed * angularMovementX;
    }
    else
    {
      if (inputSystem->isKeyDown(Keys::RightMouseButton))
      {
        m_viewPitch -= angularMovementY;
        m_viewYaw -= angularMovementX;
      }

      if (inputSystem->isKeyDown(Keys::LeftMouseButton))
      {
        moveSpeedForward += CameraMoveSpeed * angularMovementY;
        m_viewYaw -= angularMovementX;
      }
    }

    // move camera forward/backward
    if (inputSystem->isKeyDown(Keys::W))
    {
      moveSpeedForward += CameraMoveSpeed;
    }
    else if (inputSystem->isKeyDown(Keys::S))
    {
      moveSpeedForward -= CameraMoveSpeed;
    }

    // strafe camera left/right
    if (inputSystem->isKeyDown(Keys::A))
    {
      moveSpeedStrafe -= CameraMoveSpeed;
    }
    else if (inputSystem->isKeyDown(Keys::D))
    {
      moveSpeedStrafe += CameraMoveSpeed;
    }
  }

  m_viewYaw = clampAngle360(m_viewYaw);

  m_viewPitch = clamp(m_viewPitch, -85.0f, 85.0f);

  // FIXME: only update then needed ------------->
  Matrix4 rotatePitch, rotateYaw;

  makeRotateX(rotatePitch, m_viewPitch * DEG2RAD);
  makeRotateY(rotateYaw, m_viewYaw * DEG2RAD);

  Matrix4 orientation = rotatePitch * rotateYaw;
  Vector3 cameraForward = -Vector3(orientation[8], orientation[9], orientation[10]);
  Vector3 cameraUp = Vector3(orientation[4], orientation[5], orientation[6]);
  Vector3 cameraRight = Vector3(orientation[0], orientation[1], orientation[2]);

  m_viewPos = m_viewPos + 
    cameraForward * moveSpeedForward +
    cameraRight * moveSpeedStrafe +
    cameraUp * moveSpeedUp;

  Vector3 target = m_viewPos + cameraForward;

  makeLookAt(m_viewMatrix, m_viewPos, target, Vector3(0, 1, 0));
  // <-------------
}

void DefaultGameMode::getCurrentView(SceneView& view)
{
  view.m_viewMatrix = m_viewMatrix;
  view.m_projectionMatrix = m_projectionMatrix;
  view.m_worldPosition = m_viewPos;
}

void GameMode::tick(float time, GameClient& client)
{

}

void GameMode::getCurrentView(SceneView& view)
{
}

SharedPtr<DefaultGameMode> defaultGameMode = new DefaultGameMode();

GameClient::GameClient()
{
 pushGameMode(defaultGameMode); 
 m_currentGameMode = defaultGameMode;
}

GameClient::~GameClient()
{

}

void GameClient::tick(float time)
{
  m_currentGameMode->tick(time, *this);
  tickIntern(time);
}

void GameClient::render(float time)
{
  renderIntern(time);
}

void GameClient::pushGameMode(WeakPtr<GameMode> gameMode)
{
  m_gameModeStack.push(gameMode);
}

void GameClient::popGameMode()
{
  if (m_gameModeStack.size() > 1)
    m_gameModeStack.pop();

  m_currentGameMode = m_gameModeStack.top();
}

void GameClient::getCurrentView(SceneView& view)
{
  m_currentGameMode->getCurrentView(view);
}