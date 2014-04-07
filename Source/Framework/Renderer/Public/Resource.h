#ifndef __Resource_h_
#define __Resource_h_

class Resource
{
public:
  virtual bool load(const String& fileName) = 0;
  virtual void unload() = 0;
};

#endif // __Resource_h_