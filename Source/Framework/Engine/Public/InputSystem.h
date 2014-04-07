#ifndef __InputSystem_h_
#define __InputSystem_h_

#include "Keys.h"

struct KeyState
{
  bool isPressed : 1;
  bool wasPressed : 1;
};

class InputSystem
{
public:
  InputSystem();
  ~InputSystem();

  bool init();
  void tick(float deltaT);

  bool isKeyDown(Keys::eKeys key) const;
  bool isKeyPressed(Keys::eKeys key) const;
  bool isKeyReleased(Keys::eKeys key) const;

  const Point& getMouseMovement() const { return m_relativeMousePos; }

  void onKeyDown(uint16 keyCode, bool repeated);
  void onKeyUp(uint16 keyCode);

private:
  Point m_mousePos;
  Point m_relativeMousePos;
  mutable KeyState m_keyState[Keys::NumKeys];
};

#endif // __InputSystem_h_