#include <fstream>
#include <iostream>
#include <iterator>

// >>>>>>>>>>>>>>>>>>>>>>            <<<<<<<<<<<<<<<<<<<<<<
// >>>>>>>>>>>>>>>>>>>>>> COPY-BEGIN <<<<<<<<<<<<<<<<<<<<<<
// >>>>>>>>>>>>>>>>>>>>>>            <<<<<<<<<<<<<<<<<<<<<<

// Borrowed from https://github.com/eustas/mar_parser

#include <cstdint>
#include <cstddef>
#include <vector>

struct MarEntry {
  const uint8_t* data;
  size_t size;
  const char* name;
};

namespace {
uint32_t readU32(const uint8_t* data) {
  return data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
}
}  // namespace

static bool parseMar(const void* data, size_t size,
                     std::vector<MarEntry>* result) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
  if (!result) return false;
  if (size < 8) return false;
  uint32_t sig = readU32(bytes);
  if (sig != 0x4D415231) return false;
  uint32_t index = readU32(bytes + 4);
  if ((index + 4 < index) || (index + 4 > size)) return false;
  uint32_t index_size = readU32(bytes + index);
  index += 4;
  if (index + index_size < index) return false;
  uint32_t index_end = index + index_size;
  if (index_end > size) return false;
  while (index < index_end) {
    if ((index + 13 < index) || (index + 13 > index_end)) return false;
    uint32_t offset = readU32(bytes + index);
    uint32_t len = readU32(bytes + index + 4);
    if (offset + len < offset || offset + len > size) return false;
    index += 12;
    const char* name = reinterpret_cast<const char*>(bytes + index);
    while (bytes[index++]) {
      if (index == index_end) return false;
    }
    result->emplace_back(MarEntry{bytes + offset, len, name});
  }
  return true;
}

// <<<<<<<<<<<<<<<<<<<<<<            >>>>>>>>>>>>>>>>>>>>>>
// <<<<<<<<<<<<<<<<<<<<<<  COPY-END  >>>>>>>>>>>>>>>>>>>>>>
// <<<<<<<<<<<<<<<<<<<<<<            >>>>>>>>>>>>>>>>>>>>>>

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cout << "Exactly one argument is expected" << std::endl;
    return -1;
  }
  std::ifstream ifs(argv[1], std::ios::binary);
  std::vector<char> contents((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
  ifs.close();
  if (!ifs.good()) {
    std::cout << "Read error" << std::endl;
    return -2;
  }

  std::vector<MarEntry> result;
  if (!parseMar(contents.data(), contents.size(), &result)) {
    std::cout << "Parse error" << std::endl;
    return -3;
  }

  size_t n = result.size();
  for (size_t i = 0; i < n; ++i) {
    std::cout << "Entry " << (i + 1) << " of " << n
              << " name: " << result[i].name << ", size: " << result[i].size
              << std::endl;
  }

  return 0;
}