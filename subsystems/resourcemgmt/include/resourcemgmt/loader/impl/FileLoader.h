#ifndef FILELOADER_H
#define FILELOADER_H

#include "resourcemgmt/ResourceManagement.h"
#include "resourcemgmt/loader/IResourceLoader.h"

namespace resourcemgmt::loaders {

/**
 * @brief Loads files from the device file-system
 */
class RESOURCE_API FileLoader : public IResourceLoader {
public:
  /**
   * @brief GetAsInt id of this resource loader. This id will be used to
   * determin the resource loader when the resource manager receives a resource
   * request.
   * @return id of this loader
   */
  const std::string &GetId() const noexcept override;

  /**
   * @brief Loads some resource of arbitrary type (e.g. from filesystem or
   * network) using the URI of the resource.
   * @note The implementation of this function may be blocking because it will
   * be wrapped in an asynchronous function.
   * @attention If the loading process fails, this function can throw arbitrary
   * exceptions depending on the implementation and concrete resource type.
   * @param uri resource locator
   * @return the loaded resource
   */
  SharedResource Load(const std::string &uri) override;
};

inline const std::string &FileLoader::GetId() const noexcept {
  static std::string id = "file";
  return id;
}

} // namespace resourcemgmt::loaders

#endif /* FILELOADER_H */
