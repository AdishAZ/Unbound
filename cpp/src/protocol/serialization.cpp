#include "protocol/serialization.h"

#include <cstring>

#include "protocol/constants.h"

namespace unboundmp::protocol {

namespace {

void WriteBigEndianU32(uint32_t value, uint8_t out[4]) {
  out[0] = static_cast<uint8_t>((value >> 24) & 0xFF);
  out[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
  out[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
  out[3] = static_cast<uint8_t>(value & 0xFF);
}

uint32_t ReadBigEndianU32(const uint8_t in[4]) {
  return (static_cast<uint32_t>(in[0]) << 24) |
         (static_cast<uint32_t>(in[1]) << 16) |
         (static_cast<uint32_t>(in[2]) << 8) |
         static_cast<uint32_t>(in[3]);
}

}  // namespace

bool EncodeFrame(const Envelope& envelope, std::vector<uint8_t>& out_bytes) {
  const size_t payload_size = envelope.ByteSizeLong();
  if (payload_size == 0 || payload_size > kMaxFrameBytes) {
    return false;
  }

  const size_t original_size = out_bytes.size();
  out_bytes.resize(original_size + kFrameLengthPrefixBytes + payload_size);

  uint8_t length_prefix[4];
  WriteBigEndianU32(static_cast<uint32_t>(payload_size), length_prefix);
  std::memcpy(out_bytes.data() + original_size, length_prefix, 4);

  uint8_t* payload_dst = out_bytes.data() + original_size + kFrameLengthPrefixBytes;
  if (!envelope.SerializeToArray(payload_dst, static_cast<int>(payload_size))) {
    out_bytes.resize(original_size);
    return false;
  }

  return true;
}

void FrameDecoder::Feed(const uint8_t* data, size_t len) {
  if (corrupted_ || len == 0) {
    return;
  }
  buffer_.insert(buffer_.end(), data, data + len);
}

std::optional<Envelope> FrameDecoder::TryExtractNext() {
  if (corrupted_) {
    return std::nullopt;
  }

  if (buffer_.size() < kFrameLengthPrefixBytes) {
    return std::nullopt;  // need more bytes for the length prefix itself
  }

  const uint32_t payload_size = ReadBigEndianU32(buffer_.data());
  if (payload_size == 0 || payload_size > kMaxFrameBytes) {
    corrupted_ = true;
    return std::nullopt;
  }

  const size_t total_frame_size = kFrameLengthPrefixBytes + payload_size;
  if (buffer_.size() < total_frame_size) {
    return std::nullopt;  // need more bytes for the full payload
  }

  Envelope envelope;
  const uint8_t* payload_ptr = buffer_.data() + kFrameLengthPrefixBytes;
  if (!envelope.ParseFromArray(payload_ptr, static_cast<int>(payload_size))) {
    corrupted_ = true;
    return std::nullopt;
  }

  buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<long>(total_frame_size));
  return envelope;
}

}  // namespace unboundmp::protocol
