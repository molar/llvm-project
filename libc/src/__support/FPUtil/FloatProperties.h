//===-- Properties of floating point numbers --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIBC_SRC___SUPPORT_FPUTIL_FLOATPROPERTIES_H
#define LLVM_LIBC_SRC___SUPPORT_FPUTIL_FLOATPROPERTIES_H

#include "src/__support/UInt128.h"
#include "src/__support/macros/properties/architectures.h" // LIBC_TARGET_ARCH_XXX

#include <stdint.h>

// https://developer.arm.com/documentation/dui0491/i/C-and-C---Implementation-Details/Basic-data-types
// https://developer.apple.com/documentation/xcode/writing-arm64-code-for-apple-platforms
// https://docs.amd.com/bundle/HIP-Programming-Guide-v5.1/page/Programming_with_HIP.html
#if defined(_WIN32) || defined(__arm__) || defined(__NVPTX__) ||               \
    defined(__AMDGPU__) || (defined(__APPLE__) && defined(__aarch64__))
#define LONG_DOUBLE_IS_DOUBLE
#endif

#if !defined(LONG_DOUBLE_IS_DOUBLE) && defined(LIBC_TARGET_ARCH_IS_X86)
#define SPECIAL_X86_LONG_DOUBLE
#endif

namespace LIBC_NAMESPACE {
namespace fputil {

template <typename T> struct FloatProperties {};

template <> struct FloatProperties<float> {
  typedef uint32_t BitsType;
  static_assert(sizeof(BitsType) == sizeof(float),
                "Unexpected size of 'float' type.");

  static constexpr uint32_t BIT_WIDTH = sizeof(BitsType) * 8;

  static constexpr uint32_t MANTISSA_WIDTH = 23;
  // The mantissa precision includes the implicit bit.
  static constexpr uint32_t MANTISSA_PRECISION = MANTISSA_WIDTH + 1;
  static constexpr uint32_t EXPONENT_WIDTH = 8;
  static constexpr BitsType MANTISSA_MASK = (BitsType(1) << MANTISSA_WIDTH) - 1;
  static constexpr BitsType SIGN_MASK = BitsType(1)
                                        << (EXPONENT_WIDTH + MANTISSA_WIDTH);
  static constexpr BitsType EXPONENT_MASK = ~(SIGN_MASK | MANTISSA_MASK);
  static constexpr uint32_t EXPONENT_BIAS = 127;

  static constexpr BitsType EXP_MANT_MASK = MANTISSA_MASK + EXPONENT_MASK;
  static_assert(EXP_MANT_MASK == ~SIGN_MASK,
                "Exponent and mantissa masks are not as expected.");

  // If a number x is a NAN, then it is a quiet NAN if:
  //   QuietNaNMask & bits(x) != 0
  // Else, it is a signalling NAN.
  static constexpr BitsType QUIET_NAN_MASK = 0x00400000U;
};

template <> struct FloatProperties<double> {
  typedef uint64_t BitsType;
  static_assert(sizeof(BitsType) == sizeof(double),
                "Unexpected size of 'double' type.");

  static constexpr uint32_t BIT_WIDTH = sizeof(BitsType) * 8;

  static constexpr uint32_t MANTISSA_WIDTH = 52;
  static constexpr uint32_t MANTISSA_PRECISION = MANTISSA_WIDTH + 1;
  static constexpr uint32_t EXPONENT_WIDTH = 11;
  static constexpr BitsType MANTISSA_MASK = (BitsType(1) << MANTISSA_WIDTH) - 1;
  static constexpr BitsType SIGN_MASK = BitsType(1)
                                        << (EXPONENT_WIDTH + MANTISSA_WIDTH);
  static constexpr BitsType EXPONENT_MASK = ~(SIGN_MASK | MANTISSA_MASK);
  static constexpr uint32_t EXPONENT_BIAS = 1023;

  static constexpr BitsType EXP_MANT_MASK = MANTISSA_MASK + EXPONENT_MASK;
  static_assert(EXP_MANT_MASK == ~SIGN_MASK,
                "Exponent and mantissa masks are not as expected.");

  // If a number x is a NAN, then it is a quiet NAN if:
  //   QuietNaNMask & bits(x) != 0
  // Else, it is a signalling NAN.
  static constexpr BitsType QUIET_NAN_MASK = 0x0008000000000000ULL;
};

#if defined(LONG_DOUBLE_IS_DOUBLE)
// Properties for numbers represented in 64 bits long double on Windows
// platform.
template <> struct FloatProperties<long double> {
  typedef uint64_t BitsType;
  static_assert(sizeof(BitsType) == sizeof(double),
                "Unexpected size of 'double' type.");

  static constexpr uint32_t BIT_WIDTH = FloatProperties<double>::BIT_WIDTH;

  static constexpr uint32_t MANTISSA_WIDTH =
      FloatProperties<double>::MANTISSA_WIDTH;
  static constexpr uint32_t MANTISSA_PRECISION = MANTISSA_WIDTH + 1;
  static constexpr uint32_t EXPONENT_WIDTH =
      FloatProperties<double>::EXPONENT_WIDTH;
  static constexpr BitsType MANTISSA_MASK =
      FloatProperties<double>::MANTISSA_MASK;
  static constexpr BitsType SIGN_MASK = FloatProperties<double>::SIGN_MASK;
  static constexpr BitsType EXPONENT_MASK =
      FloatProperties<double>::EXPONENT_MASK;
  static constexpr uint32_t EXPONENT_BIAS =
      FloatProperties<double>::EXPONENT_BIAS;

  static constexpr BitsType EXP_MANT_MASK =
      FloatProperties<double>::EXP_MANT_MASK;
  static_assert(EXP_MANT_MASK == ~SIGN_MASK,
                "Exponent and mantissa masks are not as expected.");

  // If a number x is a NAN, then it is a quiet NAN if:
  //   QuietNaNMask & bits(x) != 0
  // Else, it is a signalling NAN.
  static constexpr BitsType QUIET_NAN_MASK =
      FloatProperties<double>::QUIET_NAN_MASK;
};
#elif defined(SPECIAL_X86_LONG_DOUBLE)
// Properties for numbers represented in 80 bits long double on non-Windows x86
// platforms.
template <> struct FloatProperties<long double> {
  typedef UInt128 BitsType;
  static_assert(sizeof(BitsType) == sizeof(long double),
                "Unexpected size of 'long double' type.");

  static constexpr uint32_t BIT_WIDTH = (sizeof(BitsType) * 8) - 48;
  static constexpr BitsType FULL_WIDTH_MASK = ((BitsType(1) << BIT_WIDTH) - 1);

  static constexpr uint32_t MANTISSA_WIDTH = 63;
  static constexpr uint32_t MANTISSA_PRECISION = MANTISSA_WIDTH + 1;
  static constexpr uint32_t EXPONENT_WIDTH = 15;
  static constexpr BitsType MANTISSA_MASK = (BitsType(1) << MANTISSA_WIDTH) - 1;

  // The x86 80 bit float represents the leading digit of the mantissa
  // explicitly. This is the mask for that bit.
  static constexpr BitsType EXPLICIT_BIT_MASK = (BitsType(1) << MANTISSA_WIDTH);

  static constexpr BitsType SIGN_MASK =
      BitsType(1) << (EXPONENT_WIDTH + MANTISSA_WIDTH + 1);
  static constexpr BitsType EXPONENT_MASK =
      ((BitsType(1) << EXPONENT_WIDTH) - 1) << (MANTISSA_WIDTH + 1);
  static constexpr uint32_t EXPONENT_BIAS = 16383;

  static constexpr BitsType EXP_MANT_MASK =
      MANTISSA_MASK | EXPLICIT_BIT_MASK | EXPONENT_MASK;
  static_assert(EXP_MANT_MASK == (~SIGN_MASK & FULL_WIDTH_MASK),
                "Exponent and mantissa masks are not as expected.");

  // If a number x is a NAN, then it is a quiet NAN if:
  //   QuietNaNMask & bits(x) != 0
  // Else, it is a signalling NAN.
  static constexpr BitsType QUIET_NAN_MASK = BitsType(1)
                                             << (MANTISSA_WIDTH - 1);
};
#else
// Properties for numbers represented in 128 bits long double on non x86
// platform.
template <> struct FloatProperties<long double> {
  typedef UInt128 BitsType;
  static_assert(sizeof(BitsType) == sizeof(long double),
                "Unexpected size of 'long double' type.");

  static constexpr uint32_t BIT_WIDTH = sizeof(BitsType) << 3;

  static constexpr uint32_t MANTISSA_WIDTH = 112;
  static constexpr uint32_t MANTISSA_PRECISION = MANTISSA_WIDTH + 1;
  static constexpr uint32_t EXPONENT_WIDTH = 15;
  static constexpr BitsType MANTISSA_MASK = (BitsType(1) << MANTISSA_WIDTH) - 1;
  static constexpr BitsType SIGN_MASK = BitsType(1)
                                        << (EXPONENT_WIDTH + MANTISSA_WIDTH);
  static constexpr BitsType EXPONENT_MASK = ~(SIGN_MASK | MANTISSA_MASK);
  static constexpr uint32_t EXPONENT_BIAS = 16383;

  static constexpr BitsType EXP_MANT_MASK = MANTISSA_MASK | EXPONENT_MASK;
  static_assert(EXP_MANT_MASK == ~SIGN_MASK,
                "Exponent and mantissa masks are not as expected.");

  // If a number x is a NAN, then it is a quiet NAN if:
  //   QuietNaNMask & bits(x) != 0
  // Else, it is a signalling NAN.
  static constexpr BitsType QUIET_NAN_MASK = BitsType(1)
                                             << (MANTISSA_WIDTH - 1);
};
#endif

#if (defined(LIBC_COMPILER_HAS_FLOAT128) &&                                    \
     !defined(LIBC_FLOAT128_IS_LONG_DOUBLE))
// Properties for numbers represented in 128 bits long double on non x86
// platform.
template <> struct FloatProperties<float128> {
  typedef UInt128 BitsType;
  static_assert(sizeof(BitsType) == sizeof(float128),
                "Unexpected size of 'float128' type.");

  static constexpr uint32_t BIT_WIDTH = sizeof(BitsType) << 3;

  static constexpr uint32_t MANTISSA_WIDTH = 112;
  static constexpr uint32_t MANTISSA_PRECISION = MANTISSA_WIDTH + 1;
  static constexpr uint32_t EXPONENT_WIDTH = 15;
  static constexpr BitsType MANTISSA_MASK = (BitsType(1) << MANTISSA_WIDTH) - 1;
  static constexpr BitsType SIGN_MASK = BitsType(1)
                                        << (EXPONENT_WIDTH + MANTISSA_WIDTH);
  static constexpr BitsType EXPONENT_MASK = ~(SIGN_MASK | MANTISSA_MASK);
  static constexpr uint32_t EXPONENT_BIAS = 16383;

  static constexpr BitsType EXP_MANT_MASK = MANTISSA_MASK | EXPONENT_MASK;
  static_assert(EXP_MANT_MASK == ~SIGN_MASK,
                "Exponent and mantissa masks are not as expected.");

  // If a number x is a NAN, then it is a quiet NAN if:
  //   QuietNaNMask & bits(x) != 0
  // Else, it is a signalling NAN.
  static constexpr BitsType QUIET_NAN_MASK = BitsType(1)
                                             << (MANTISSA_WIDTH - 1);
};
#endif // LIBC_COMPILER_HAS_FLOAT128

// Define the float type corresponding to the BitsType.
template <typename BitsType> struct FloatType;

template <> struct FloatType<uint32_t> {
  static_assert(sizeof(uint32_t) == sizeof(float),
                "Unexpected size of 'float' type.");
  typedef float Type;
};

template <> struct FloatType<uint64_t> {
  static_assert(sizeof(uint64_t) == sizeof(double),
                "Unexpected size of 'double' type.");
  typedef double Type;
};

template <typename BitsType>
using FloatTypeT = typename FloatType<BitsType>::Type;

} // namespace fputil
} // namespace LIBC_NAMESPACE

#endif // LLVM_LIBC_SRC___SUPPORT_FPUTIL_FLOATPROPERTIES_H
