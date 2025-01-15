#!/usr/bin/env python3
# Copyright 2024 Google LLC
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import os
import sys

from gemm_compiler import generate_f32_gemm_microkernels as f32
from gemm_compiler import generate_qd8_f32_qc8w_gemm_microkernels as qd8_f32_qc8w

"""Generates all assembly gemm microkernels."""


def main(args):

  f32.generate_f32_gemm_microkernels()
  qd8_f32_qc8w.generate_qd8_f32_qc8w_gemm_microkernels()


if __name__ == "__main__":
  main(sys.argv[1:])
