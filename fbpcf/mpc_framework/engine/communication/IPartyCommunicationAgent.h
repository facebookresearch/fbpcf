/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <emmintrin.h>
#include <stdint.h>
#include <string.h>
#include <vector>

#if __BYTE_ORDER != __LITTLE_ENDIAN
#error "Machine must be little endian"
#endif

namespace fbpcf::mpc_framework::engine::communication {

/**
 * This is the network API between two parties.
 * NOTE: sendT/receiveT only work when the two parties have the same endianness
 */
class IPartyCommunicationAgent {
 public:
  virtual ~IPartyCommunicationAgent() = default;

  /**
   * send a byte string to the partner
   * @param data the data to be sent
   */
  virtual void send(const std::vector<unsigned char>& data) = 0;

  /**
   * receive a byte string from the partner
   * @param size the expected size;
   * @return the received content
   */
  virtual std::vector<unsigned char> receive(int size) = 0;

  /**
   * send a byte string to the partner
   * @param data the data to be sent
   */
  void sendBool(const std::vector<bool>& data) {
    auto compressed = compressToBytes(data);
    send(compressed);
  }

  /**
   * receive a byte string from the partner
   * @param size the expected size;
   * @return the received content
   */
  std::vector<bool> receiveBool(int size) {
    int compressedSize = (size + 7) >> 3;
    auto compressed = receive(compressedSize);
    auto decompressed = decompressToBits(std::move(compressed));
    decompressed.erase(decompressed.begin() + size, decompressed.end());
    return decompressed;
  }

  template <typename T>
  void sendT(const std::vector<T>& src) {
    std::vector<unsigned char> buf(sizeof(T) * src.size());
    memcpy(
        buf.data(),
        reinterpret_cast<const unsigned char*>(src.data()),
        sizeof(T) * src.size());
    send(buf);
  }

  template <typename T>
  std::vector<T> receiveT(int size) {
    std::vector<T> rst(size);
    auto buf = receive(sizeof(T) * size);

    memcpy(
        reinterpret_cast<unsigned char*>(rst.data()),
        buf.data(),
        sizeof(T) * size);
    return rst;
  }

  template <typename T>
  void sendSingleT(const T& t) {
    sendT<T>({t});
  }

  template <typename T>
  T receiveSingleT() {
    return receiveT<T>(1).at(0);
  }

  /**
   * Get the total amount of traffic transmitted.
   * @return a pair of (sent, received) data in bytes.
   */
  virtual std::pair<uint64_t, uint64_t> getTrafficStatistics() const = 0;

 private:
  // convert a vector of bits into a vector of bytes
  static std::vector<unsigned char> compressToBytes(
      const std::vector<bool>& bits) {
    // each secret is 1 bit, calculate number of bytes needed
    uint64_t numberOfBytes = (bits.size() + 7) >> 3;
    std::vector<unsigned char> rst(numberOfBytes);
    uint64_t bitIndex = 0;
    uint64_t byteIndex = 0;
    while (bitIndex < bits.size()) {
      unsigned char byte = 0;
      uint8_t i = 0;
      for (; i < 8 && bitIndex < bits.size(); i++, bitIndex++) {
        byte = byte << 1;
        byte ^= bits[bitIndex] & 1;
      }
      // pad the byte when there are less than 8 bits to pack
      byte = byte << (8 - i);
      rst[byteIndex++] = byte;
    }
    return rst;
  }

  // decompress a byte vector to a bit vector
  static std::vector<bool> decompressToBits(
      std::vector<unsigned char>&& bytes) {
    std::vector<bool> bits(bytes.size() * 8);
    uint64_t bitIndex = 0;
    for (uint64_t i = 0; i < bytes.size(); i++) {
      for (uint8_t j = 0; j < 8; j++) {
        bits[bitIndex + 7 - j] = bytes[i] & 1;
        bytes[i] = bytes[i] >> 1;
      }
      bitIndex += 8;
    }
    return bits;
  }
};

template <>
inline void IPartyCommunicationAgent::sendT(
    const std::vector<unsigned char>& src) {
  send(src);
}

template <>
inline void IPartyCommunicationAgent::sendT(const std::vector<bool>& src) {
  sendBool(src);
}

template <>
inline std::vector<unsigned char> IPartyCommunicationAgent::receiveT(int size) {
  return receive(size);
}

template <>
inline std::vector<bool> IPartyCommunicationAgent::receiveT(int size) {
  return receiveBool(size);
}

} // namespace fbpcf::mpc_framework::engine::communication
