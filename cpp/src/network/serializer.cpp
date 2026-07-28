#include "network/serializer.h"
#include <cstring>
#include <stdexcept>

namespace unboundmp::network {

Serializer::Serializer(std::vector<uint8_t> buffer) : buffer_(std::move(buffer)) {}

template <typename T>
void Serializer::WritePrimitive(T value) {
  const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
  buffer_.insert(buffer_.end(), p, p + sizeof(T));
}

template <typename T>
T Serializer::ReadPrimitive() {
  if (position_ + sizeof(T) > buffer_.size()) {
    throw std::out_of_range("Serializer buffer underflow");
  }
  T value;
  std::memcpy(&value, buffer_.data() + position_, sizeof(T));
  position_ += sizeof(T);
  return value;
}

void Serializer::WriteU8(uint8_t value) { WritePrimitive(value); }
void Serializer::WriteU16(uint16_t value) { WritePrimitive(value); }
void Serializer::WriteU32(uint32_t value) { WritePrimitive(value); }
void Serializer::WriteU64(uint64_t value) { WritePrimitive(value); }
void Serializer::WriteI32(int32_t value) { WritePrimitive(value); }
void Serializer::WriteFloat(float value) { WritePrimitive(value); }
void Serializer::WriteDouble(double value) { WritePrimitive(value); }
void Serializer::WriteBool(bool value) { WritePrimitive(value); }

void Serializer::WriteString(const std::string& value) {
  WriteU32(static_cast<uint32_t>(value.size()));
  buffer_.insert(buffer_.end(), value.begin(), value.end());
}

void Serializer::WriteBytes(const std::vector<uint8_t>& value) {
  WriteU32(static_cast<uint32_t>(value.size()));
  buffer_.insert(buffer_.end(), value.begin(), value.end());
}

uint8_t Serializer::ReadU8() { return ReadPrimitive<uint8_t>(); }
uint16_t Serializer::ReadU16() { return ReadPrimitive<uint16_t>(); }
uint32_t Serializer::ReadU32() { return ReadPrimitive<uint32_t>(); }
uint64_t Serializer::ReadU64() { return ReadPrimitive<uint64_t>(); }
int32_t Serializer::ReadI32() { return ReadPrimitive<int32_t>(); }
float Serializer::ReadFloat() { return ReadPrimitive<float>(); }
double Serializer::ReadDouble() { return ReadPrimitive<double>(); }
bool Serializer::ReadBool() { return ReadPrimitive<bool>(); }

std::string Serializer::ReadString() {
  uint32_t length = ReadU32();
  if (position_ + length > buffer_.size()) {
    throw std::out_of_range("Serializer buffer underflow reading string");
  }
  std::string value(reinterpret_cast<const char*>(buffer_.data() + position_), length);
  position_ += length;
  return value;
}

std::vector<uint8_t> Serializer::ReadBytes() {
  uint32_t length = ReadU32();
  if (position_ + length > buffer_.size()) {
    throw std::out_of_range("Serializer buffer underflow reading bytes");
  }
  std::vector<uint8_t> value(buffer_.data() + position_, buffer_.data() + position_ + length);
  position_ += length;
  return value;
}

}  // namespace unboundmp::network
