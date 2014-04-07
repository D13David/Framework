#ifndef __InitParams_h_
#define __InitParams_h_

struct InitParams
{
  const char* gameTitle;
  int width;
  int height;
  int msaaSamples;
  bool srgbTarget;
  bool fullscreen;
};

#endif // __InitParams_h_