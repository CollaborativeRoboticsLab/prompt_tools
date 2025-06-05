#pragma once

#include <Poco/Net/PartSource.h>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief FilePartSource
 *
 * A memory-based PartSource implementation for streaming binary content
 * such as audio files (e.g. WAV). Supports both raw byte input and int16_t PCM samples.
 */
class FilePartSource : public Poco::Net::PartSource
{
public:
  /**
   * @brief Constructor for binary (char) input
   * @param mediaType MIME type of the file (e.g. "audio/wav")
   * @param data Raw binary buffer (e.g. WAV file)
   */
  FilePartSource(const std::string& mediaType, const std::vector<char>& data)
    : Poco::Net::PartSource(mediaType), stream_(&buffer_)
  {
    buffer_.sputn(data.data(), data.size());
    buffer_.pubseekpos(0);
  }

  /**
   * @brief Constructor for PCM int16_t input (auto-converts to char)
   * @param mediaType MIME type of the file (e.g. "audio/wav")
   * @param data PCM buffer (e.g. raw 16-bit mono samples)
   */
  FilePartSource(const std::string& mediaType, const std::vector<int16_t>& data)
    : Poco::Net::PartSource(mediaType), stream_(&buffer_)
  {
    const char* raw = reinterpret_cast<const char*>(data.data());
    std::size_t byte_size = data.size() * sizeof(int16_t);
    buffer_.sputn(raw, byte_size);
    buffer_.pubseekpos(0);
  }

  ~FilePartSource() override = default;

  std::istream& stream() override
  {
    return stream_;
  }

private:
  std::stringbuf buffer_;
  std::istream stream_;
};
