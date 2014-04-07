#ifndef __ResourceManager_h_
#define __ResourceManager_h_

#include "Resource.h"

class ResourceManager
{
public:
  ResourceManager();
  ~ResourceManager();

  Resource* load(const String& fileName);
  void unload(Resource* resource);
  void unloadAll();

protected:
  virtual Resource* createResource(const String& fileName) = 0;

private:
  ResourceManager(const ResourceManager&);
  ResourceManager& operator=(const ResourceManager&);

  typedef std::hash_map<uint32, Resource*> ResourceTable;
  ResourceTable m_resources;
};

#endif // __ResourceManager_h_