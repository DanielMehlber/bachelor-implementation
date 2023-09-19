#ifndef RESOURCEFACTORY_H
#define RESOURCEFACTORY_H

#include "memory"
#include "resourcemgmt/Resource.h"

namespace resourcemgmt {
class ResourceFactory {
public:
  template <typename ResourceType>
  static SharedResource
  CreateSharedResource(std::shared_ptr<ResourceType> content_ptr);

  template <typename ResourceType>
  static SharedResource CreateSharedResource(const ResourceType &content);
};

template <typename ResourceType>
inline SharedResource resourcemgmt::ResourceFactory::CreateSharedResource(
    std::shared_ptr<ResourceType> content_ptr) {
  return std::make_shared<Resource>(content_ptr);
}

template <typename ResourceType>
inline SharedResource resourcemgmt::ResourceFactory::CreateSharedResource(
    const ResourceType &content) {
  auto ptr = std::make_shared<ResourceType>(content);
  return CreateSharedResource(ptr);
}

} // namespace resourcemgmt

#endif /* RESOURCEFACTORY_H */
