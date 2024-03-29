#ifndef SIMULATION_FRAMEWORK_GZIPRENDERRESULTENCODER_H
#define SIMULATION_FRAMEWORK_GZIPRENDERRESULTENCODER_H

#include "graphics/service/encoders/IRenderResultEncoder.h"

namespace graphics {

class GzipRenderResultEncoder : public IRenderResultEncoder {
public:
  std::string GetName() override { return "gzip"; };
  std::string Encode(const std::vector<unsigned char> &buffer) override;
  std::vector<unsigned char> Decode(const std::string &str) override;
};

} // namespace graphics

#endif // SIMULATION_FRAMEWORK_GZIPRENDERRESULTENCODER_H
