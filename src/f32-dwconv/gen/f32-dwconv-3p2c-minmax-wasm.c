// Auto-generated file. Do not edit!
//   Template: src/f32-dwconv/unipass-scalar.c.in
//   Generator: tools/xngen
//
// Copyright 2019 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>

#include "xnnpack/dwconv.h"
#include "xnnpack/math.h"


void xnn_f32_dwconv_minmax_ukernel_3p2c__wasm(
    size_t channels,
    size_t output_width,
    const float** input,
    const float* weights,
    float* output,
    intptr_t input_stride,
    size_t output_increment,
    size_t input_offset,
    const float* zero,
    const union xnn_f32_minmax_params params[restrict XNN_MIN_ELEMENTS(1)])
{
  assert(channels != 0);
  assert(output_width != 0);

  const float vmin = params->scalar.min;
  const float vmax = params->scalar.max;
  do {
    const float* i0 = input[0];
    assert(i0 != NULL);
    if XNN_UNPREDICTABLE(i0 != zero) {
      i0 = (const float*) ((uintptr_t) i0 + input_offset);
    }
    const float* i1 = input[1];
    assert(i1 != NULL);
    if XNN_UNPREDICTABLE(i1 != zero) {
      i1 = (const float*) ((uintptr_t) i1 + input_offset);
    }
    const float* i2 = input[2];
    assert(i2 != NULL);
    if XNN_UNPREDICTABLE(i2 != zero) {
      i2 = (const float*) ((uintptr_t) i2 + input_offset);
    }
    input = (const float**) ((uintptr_t) input + input_stride);

    size_t c = channels;
    const float* w = weights;
    for (; c >= 2; c -= 2) {
      float vacc0p0 = w[0];
      float vacc1p0 = w[1];


      const float vi0x0 = i0[0];
      const float vi0x1 = i0[1];
      i0 += 2;

      const float vk0x0 = w[2];
      vacc0p0 = math_muladd_f32(vi0x0, vk0x0, vacc0p0);
      const float vk0x1 = w[3];
      vacc1p0 = math_muladd_f32(vi0x1, vk0x1, vacc1p0);

      const float vi1x0 = i1[0];
      const float vi1x1 = i1[1];
      i1 += 2;

      const float vk1x0 = w[4];
      vacc0p0 = math_muladd_f32(vi1x0, vk1x0, vacc0p0);
      const float vk1x1 = w[5];
      vacc1p0 = math_muladd_f32(vi1x1, vk1x1, vacc1p0);

      const float vi2x0 = i2[0];
      const float vi2x1 = i2[1];
      i2 += 2;

      const float vk2x0 = w[6];
      vacc0p0 = math_muladd_f32(vi2x0, vk2x0, vacc0p0);
      const float vk2x1 = w[7];
      vacc1p0 = math_muladd_f32(vi2x1, vk2x1, vacc1p0);

      w += 8;


      float vacc0 = __builtin_wasm_max_f32(vacc0p0, vmin);
      float vacc1 = __builtin_wasm_max_f32(vacc1p0, vmin);

      vacc0 = __builtin_wasm_min_f32(vacc0, vmax);
      vacc1 = __builtin_wasm_min_f32(vacc1, vmax);

      output[0] = vacc0;
      output[1] = vacc1;
      output += 2;
    }
    for (; c >= 1; c -= 1) {
      float vacc0p0 = *w++;

      const float vi0 = *i0++;
      const float vk0 = w[1];
      vacc0p0 = math_muladd_f32(vi0, vk0, vacc0p0);
      const float vi1 = *i1++;
      const float vk1 = w[3];
      vacc0p0 = math_muladd_f32(vi1, vk1, vacc0p0);
      const float vi2 = *i2++;
      const float vk2 = w[5];
      vacc0p0 = math_muladd_f32(vi2, vk2, vacc0p0);


      float vacc0 = __builtin_wasm_max_f32(vacc0p0, vmin);
      vacc0 = __builtin_wasm_min_f32(vacc0, vmax);
      *output++ = vacc0;
    }

    output = (float*) ((uintptr_t) output + output_increment);
  } while (--output_width != 0);
}
