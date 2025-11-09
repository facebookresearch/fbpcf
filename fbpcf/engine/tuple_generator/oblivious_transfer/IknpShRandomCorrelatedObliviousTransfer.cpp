/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "fbpcf/engine/tuple_generator/oblivious_transfer/IknpShRandomCorrelatedObliviousTransfer.h"
#include <emmintrin.h>
#include <stdexcept>
#include "fbpcf/engine/util/util.h"

namespace fbpcf::engine::tuple_generator::oblivious_transfer {

IknpShRandomCorrelatedObliviousTransfer::
    IknpShRandomCorrelatedObliviousTransfer(
        __m128i delta,
        std::unique_ptr<IBaseObliviousTransfer> baseOt,
        std::unique_ptr<util::IPrgFactory> prgFactory)
    : decomposedDelta_(127) {
  if (util::getLsb(delta) != 1) {
    throw std::runtime_error("Lsb of delta must be 1!");
  }
  uint64_t tmp = _mm_extract_epi64(delta, 0);
  tmp = tmp >> 1;
  // decomposedDelta_ represents the 2nd to 128-th bit of delta;
  for (size_t i = 1; i < 64; i++) {
    decomposedDelta_[i - 1] = tmp & 1;
    tmp = tmp >> 1;
  }
  tmp = _mm_extract_epi64(delta, 1);
  for (size_t i = 64; i < 128; i++) {
    decomposedDelta_[i - 1] = tmp & 1;
    tmp = tmp >> 1;
  }
  assert(baseOt != nullptr);
  assert(prgFactory != nullptr);
  auto seeds = baseOt->receive(decomposedDelta_);
  agent_ = baseOt->extractCommunicationAgent();

  // Our output message size is 128 bit. The LSB is reserved for choice bit, so
  // we only need to generate the rest 127 bits.
  for (size_t i = 0; i < 127; i++) {
    senderPrgs_.push_back(prgFactory->create(seeds.at(i)));
  }
  role_ = util::Role::sender;
}

// receiver constructor
IknpShRandomCorrelatedObliviousTransfer::
    IknpShRandomCorrelatedObliviousTransfer(
        std::unique_ptr<IBaseObliviousTransfer> baseOt,
        std::unique_ptr<util::IPrgFactory> prgFactory) {
  auto seeds = baseOt->send(127);
  agent_ = baseOt->extractCommunicationAgent();

  for (size_t i = 0; i < 127; i++) {
    receiverPrgs0_.push_back(prgFactory->create(seeds.first.at(i)));
    receiverPrgs1_.push_back(prgFactory->create(seeds.second.at(i)));
  }
  choiceBitPrg_ = prgFactory->create(util::getRandomM128iFromSystemNoise());
  role_ = util::Role::receiver;
}

std::vector<__m128i> IknpShRandomCorrelatedObliviousTransfer::matrixTranspose(
    const std::vector<__m128i>& src) {
  // ensure the size of src is a multiplication of 128
  assert((src.size() & 0x7F) == 0);
  std::vector<__m128i> rst(src.size());

  /**
   *The matrix transpose is done with SIMD instructions.
   *This function deals with 128 by 128 bit matrixes. Each 128 __m128i values
   *in the input vector represents 1 such matrix to be transposed. The output
   *will be the transposed 128 by 128 bit matrixes. Decomposing a byte into bits
   *is extremely expensive compared to SIMD instructions. Therefore we leverage
   *the instruction _mm_movemask_epi8() to do this. _mm_movemask_epi8() takes
   *the msb of each byte in the input and put these bits together into a 16bit
   *word. Therefore, we want to collect the first bytes of all __m128is, the
   *second bytes of all __m128is... This is done by using unpackhi/lo family
   *instructions. These instructions takes in two
   *__m128i inputs, and take the first/last significant 8/16/32/64-bit words of
   *these two inputs. For example, the first 2 bytes of the output of
   *_mm_unpackhi_epi8() are the first bytes of some original inputs; sending the
   *outputs of _mm_unpackhi_epi8() to _mm_unpackhi_epi16(), this instruction's
   *output's first 4 bytes will be the first bytes of some original inputs. With
   *4 rounds of iteration, we can get vectors of all first/second/third bytes of
   *original inputs. Then we can use _mm_movemask_epi8() to collect the msb of
   *these bytes, and use _mm_slli_epi16 to shift the bytes left after collecting
   *all the msbs. This procedure can be repeats until all the bits in each byte
   *are processed.
   **/

  for (size_t index = 0; index < src.size(); index += 128) {
    std::array<__m128i, 128> buffer0;
    std::array<__m128i, 128> buffer1;

    for (int i = 0; i < 64; i++) {
      buffer0[i] =
          _mm_unpacklo_epi8(src.at(index + 2 * i), src.at(index + 2 * i + 1));

      buffer0[i + 64] =
          _mm_unpackhi_epi8(src.at(index + 2 * i), src.at(index + 2 * i + 1));
    }

    for (int j = 0; j < 2; j++) {
      for (int i = 0; i < 32; i++) {
        buffer1[i + (j << 6) /* j * 64 */] = _mm_unpacklo_epi16(
            buffer0.at((i << 1) /* 2 * i */ + (j << 6) /* j * 64 */),
            buffer0.at((i << 1) /* 2 * i */ + 1 + (j << 6) /* j * 64 */));

        buffer1[i + (j << 6) /* j * 64 */ + 32] = _mm_unpackhi_epi16(
            buffer0.at((i << 1) /* 2 * i */ + (j << 6) /* j * 64 */),
            buffer0.at((i << 1) /* 2 * i */ + 1 + (j << 6) /* j * 64 */));
      }
    }

    for (int j = 0; j < 4; j++) {
      for (int i = 0; i < 16; i++) {
        buffer0[i + (j << 5) /* j * 32 */] = _mm_unpacklo_epi32(
            buffer1.at((i << 1) /* 2 * i */ + (j << 5) /* j * 32 */),
            buffer1.at((i << 1) /* 2 * i */ + 1 + (j << 5) /* j * 32 */));

        buffer0[i + (j << 5) /* j * 32 */ + 16] = _mm_unpackhi_epi32(
            buffer1.at((i << 1) /* 2 * i */ + (j << 5) /* j * 32 */),
            buffer1.at((i << 1) /* 2 * i */ + 1 + (j << 5) /* j * 32 */));
      }
    }

    for (int j = 0; j < 8; j++) {
      for (int i = 0; i < 8; i++) {
        buffer1[i + (j << 4) /* j * 16 */] = _mm_unpacklo_epi64(
            buffer0.at((i << 1) /* 2 * i */ + (j << 4) /* j * 16 */),
            buffer0.at((i << 1) /* 2 * i */ + 1 + (j << 4) /* j * 16 */));

        buffer1[i + (j << 4) /* j * 16 */ + 8] = _mm_unpackhi_epi64(
            buffer0.at((i << 1) /* 2 * i */ + (j << 4) /* j * 16 */),
            buffer0.at((i << 1) /* 2 * i */ + 1 + (j << 4) /* j * 16 */));
      }
    }

    for (int i = 7; i >= 0; i--) {
      for (int j = 15; j >= 0; j--) {
        rst[index + ((j << 3) + i)] = _mm_set_epi16(
            _mm_movemask_epi8(buffer1.at((j << 3) + 7)),
            _mm_movemask_epi8(buffer1.at((j << 3) + 6)),
            _mm_movemask_epi8(buffer1.at((j << 3) + 5)),
            _mm_movemask_epi8(buffer1.at((j << 3) + 4)),
            _mm_movemask_epi8(buffer1.at((j << 3) + 3)),
            _mm_movemask_epi8(buffer1.at((j << 3) + 2)),
            _mm_movemask_epi8(buffer1.at((j << 3) + 1)),
            _mm_movemask_epi8(buffer1.at((j << 3) + 0)));
      }
      for (int j = 0; j < 128; j++) {
        buffer1[j] = _mm_slli_epi16(buffer1.at(j), 1);
      }
    }
  }
  return rst;
}

std::vector<__m128i> IknpShRandomCorrelatedObliviousTransfer::rcot(
    int64_t size) {
  std::vector<__m128i> rst;
  // must work with a multiplication of 128;
  size_t blockCount = ((size + 127) / 128);
  if (role_ == util::receiver) {
    std::vector<__m128i> t0(blockCount * 128);
    std::vector<__m128i> u(blockCount * 127);
    for (size_t i = 0, indexT = 0, indexU = 0; i < blockCount;
         i++, indexT += 128, indexU += 127) {
      // t0[indexT] stores the choice, which will become the LSBs after
      // transpose
      t0[indexT] = choiceBitPrg_->getRandomM128i();
      std::vector<__m128i> t1(127);
      for (size_t j = 0; j < 127; j++) {
        t0[indexT + 1 + j] = receiverPrgs0_[j]->getRandomM128i();
        t1[j] = receiverPrgs1_[j]->getRandomM128i();
        u[indexU + j] = _mm_xor_si128(t0.at(indexT + 1 + j), t1.at(j));
        u[indexU + j] = _mm_xor_si128(u.at(indexU + j), t0.at(indexT));
      }
    }
    agent_->sendT(u);
    rst = matrixTranspose(t0);

  } else {
    std::vector<__m128i> t(blockCount * 128);
    for (size_t i = 0, indexT = 0; i < blockCount; i++, indexT += 128) {
      for (size_t j = 0; j < 127; j++) {
        t[indexT + 1 + j] = senderPrgs_[j]->getRandomM128i();
      }
      //  t[indexT] will always be 0. This vector will be the LSBs after
      //  transpose.
      t[indexT] = _mm_set_epi64x(0, 0);
    }

    auto u = agent_->receiveT<__m128i>(blockCount * 127);
    for (size_t i = 0, indexT = 0, indexU = 0; i < blockCount;
         i++, indexT += 128, indexU += 127) {
      for (size_t j = 0; j < 127; j++) {
        if (decomposedDelta_.at(j)) {
          t[indexT + 1 + j] =
              _mm_xor_si128(t.at(indexT + 1 + j), u.at(indexU + j));
        }
      }
    }
    rst = matrixTranspose(t);
  }
  rst.erase(rst.begin() + size, rst.end());
  return rst;
}

} // namespace fbpcf::engine::tuple_generator::oblivious_transfer
