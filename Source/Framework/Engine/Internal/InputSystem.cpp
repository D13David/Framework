#include "Core.h"
#include "InputSystem.h"

#include <windows.h>
#include <cmath>

std::vector<Keys::eKeys> VirtualToKeyMap;

InputSystem::InputSystem()
  : m_mousePos(0, 0)
  , m_relativeMousePos(0, 0)
{
  for (uint32 i = 0; i < Keys::NumKeys; ++i)
  {
    m_keyState[i].isPressed = false;
    m_keyState[i].wasPressed = false;
  }
}

InputSystem::~InputSystem()
{

}

bool InputSystem::init()
{
  // initialize virtual key to key map
#define DEFINE_VIRTUAL_KEY_MAPPING(virtualKeyCode, keyCode) VirtualToKeyMap[(virtualKeyCode)] = (keyCode)

  VirtualToKeyMap.resize(0xffff);

  DEFINE_VIRTUAL_KEY_MAPPING(VK_LBUTTON, Keys::LeftMouseButton);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_RBUTTON, Keys::RightMouseButton);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_MBUTTON, Keys::MiddleMouseButton);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_XBUTTON1, Keys::ThumbMouseButton1);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_XBUTTON2, Keys::ThumbMouseButton2);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_BACK, Keys::Backspace);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_TAB, Keys::Tab);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_RETURN, Keys::Enter);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_PAUSE, Keys::Pause);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_CAPITAL, Keys::CapsLock);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_ESCAPE, Keys::Esc);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_SPACE, Keys::Substract);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_PRIOR, Keys::PgUp);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NEXT, Keys::PgDown);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_END, Keys::End);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_HOME, Keys::Home);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_LEFT, Keys::Left);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_RIGHT, Keys::Right);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_UP, Keys::Up);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_DOWN, Keys::Down);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD0, Keys::NumPad0);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD1, Keys::NumPad1);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD2, Keys::NumPad2);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD3, Keys::NumPad3);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD4, Keys::NumPad4);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD5, Keys::NumPad5);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD6, Keys::NumPad6);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD7, Keys::NumPad7);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD8, Keys::NumPad8);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMPAD9, Keys::NumPad9);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_ADD, Keys::Add);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_SUBTRACT, Keys::Substract);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_MULTIPLY, Keys::Multiply);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_DIVIDE, Keys::Divide);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_F1, Keys::F1);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_F2, Keys::F2);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_F3, Keys::F3);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_F4, Keys::F4);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_F5, Keys::F5);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_F6, Keys::F6);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_F7, Keys::F7);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_F8, Keys::F8);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_F9, Keys::F9);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_NUMLOCK, Keys::NumLock);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_SCROLL, Keys::ScrollLock);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_LSHIFT, Keys::LeftShift);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_RSHIFT, Keys::RightShift);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_LCONTROL, Keys::LeftCtrl);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_RCONTROL, Keys::RightCtrl);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_LMENU, Keys::LeftAlt);
  DEFINE_VIRTUAL_KEY_MAPPING(VK_RMENU, Keys::RightAlt);

  for (uint16 i = '0'; i < '9'; ++i)
    DEFINE_VIRTUAL_KEY_MAPPING(i, (Keys::eKeys)(Keys::Key0 + (i - '0')));

  for (uint16 i = 'A'; i < 'Z'; ++i)
    DEFINE_VIRTUAL_KEY_MAPPING(i, (Keys::eKeys)(Keys::A + (i - 'A')));

  VirtualToKeyMap.shrink_to_fit();

#undef DEFINE_VIRTUAL_KEY_MAPPING

  POINT cursorPos;
  if (GetCursorPos(&cursorPos))
  {
    m_mousePos.x = (float)cursorPos.x;
    m_mousePos.y = (float)cursorPos.y;
  }

  return true;
}

bool InputSystem::isKeyDown(Keys::eKeys key) const
{
  return m_keyState[key].isPressed;
}

bool InputSystem::isKeyPressed(Keys::eKeys key) const
{
  return m_keyState[key].isPressed && !m_keyState[key].wasPressed;
}

bool InputSystem::isKeyReleased(Keys::eKeys key) const
{
  return !m_keyState[key].isPressed;
}

void InputSystem::tick(float deltaT)
{
  POINT cursorPos;
  if (GetCursorPos(&cursorPos))
  {
    m_relativeMousePos.x = m_mousePos.x - cursorPos.x;
    m_relativeMousePos.y = m_mousePos.y - cursorPos.y;
    m_mousePos.x = (float)cursorPos.x;
    m_mousePos.y = (float)cursorPos.y;
  }

  const bool swapRL = GetSystemMetrics(SM_SWAPBUTTON) != 0;

  m_keyState[Keys::RightMouseButton].isPressed = (GetAsyncKeyState(swapRL ? VK_RBUTTON : VK_LBUTTON) & 0x8000) != 0;
  m_keyState[Keys::LeftMouseButton].isPressed = (GetAsyncKeyState(swapRL ? VK_LBUTTON : VK_RBUTTON) & 0x8000) != 0;
  m_keyState[Keys::MiddleMouseButton].isPressed = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
  m_keyState[Keys::ThumbMouseButton1].isPressed = (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;  
  m_keyState[Keys::ThumbMouseButton2].isPressed = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
}

void InputSystem::onKeyDown(uint16 keyCode, bool repeated)
{
  m_keyState[VirtualToKeyMap[keyCode]].isPressed = true;
  m_keyState[VirtualToKeyMap[keyCode]].wasPressed = repeated;
}

void InputSystem::onKeyUp(uint16 keyCode)
{
  m_keyState[VirtualToKeyMap[keyCode]].wasPressed = false;
  m_keyState[VirtualToKeyMap[keyCode]].isPressed = false;
}