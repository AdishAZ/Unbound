#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>
#include <type_traits>

namespace unboundmp::network {

class Serializer {
 public:
  Serializer() = default;
  explicit Serializer(std::vector<uint8_t> buffer);

  // Write primitives
  void WriteU8(uint8_t value);
  void WriteU16(uint16_t value);
  void WriteU32(uint32_t value);
  void WriteU64(uint64_t value);
  void WriteI32(int32_t value);
  void WriteFloat(float value);
  void WriteDouble(double value);
  void WriteBool(bool value);
  void WriteString(const std::string& value);
  void WriteBytes(const std::vector<uint8_t>& value);

  // Read primitives
  uint8_t ReadU8();
  uint16_t ReadU16();
  uint32_t ReadU32();
  uint64_t ReadU64();
  int32_t ReadI32();
  float ReadFloat();
  double ReadDouble();
  bool ReadBool();
  std::string ReadString();
  std::vector<uint8_t> ReadBytes();

  const std::vector<uint8_t>& GetBuffer() const { return buffer_; }
  size_t GetPosition() const { return position_; }

 private:
  template <typename T>
  void WritePrimitive(T value);

  template <typename T>
  T ReadPrimitive();

  std::vector<uint8_t> buffer_;
  size_t position_ = 0;
};

}  // namespace unboundmp::network
