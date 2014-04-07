#include "Core.h"
#include "ResourceManager.h"
#include "StringUtils.h"
#include "Hash.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

Resource* ResourceManager::load(const String& fileName)
{
  Resource* result = 0;
  String name = PathUtils::getPath(fileName, true);
  uint32 resourceHash = crc32Hash((const ubyte*)name.c_str(), name.length());
  
  ResourceTable::iterator it = m_resources.find(resourceHash);
  if (it == m_resources.end())
  {
    result = createResource(fileName);
    m_resources.insert(std::make_pair(resourceHash, result));
  }
  else
  {
    result = it->second;
  }

  return result;
}

void ResourceManager::unload(Resource* resource)
{
}

void ResourceManager::unloadAll()
{
}