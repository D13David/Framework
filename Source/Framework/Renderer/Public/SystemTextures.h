#ifndef __SystemTextures_h_
#define __SystemTextures_h_

class SystemTextures
{
public:
  static void init();

  // default
  static SharedPtr<Texture> Default;
  static SharedPtr<Texture> White;
  static SharedPtr<Texture> Black;
};

#endif // __SystemTextures_h_