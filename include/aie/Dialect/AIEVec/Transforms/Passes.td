//===-- Passes.td - AIE vector pass definition file --------*- tablegen -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// (c) Copyright 2022 Xilinx Inc.
//
//===----------------------------------------------------------------------===//
// This file contains definitions for passes within the AIEVec/ directory.
//===----------------------------------------------------------------------===//

#ifndef AIE_DIALECT_AIEVEC_TRANSFORMS_PASSES
#define AIE_DIALECT_AIEVEC_TRANSFORMS_PASSES

include "mlir/Pass/PassBase.td"

def AIEVectorize : Pass<"aie-vectorize", "ModuleOp"> {
  let summary = "Vectorize the output of affine "
                "supervectorizer to AIE vector abstraction";
  let constructor = "xilinx::aievec::createAIEVectorizePass()";
  let dependentDialects = ["AffineDialect", 
                           "xilinx::aievec::AIEVecDialect",
                           "arith::ArithmeticDialect",
                           "memref::MemRefDialect",
                           "scf::SCFDialect", 
                           "vector::VectorDialect"];
  let options = [
    Option<"shiftParam", "shift", "unsigned", /*default=*/"0",
      "Shift parameter for rounding and saturation">,
    Option<"zeroOffset", "zero-offset", "unsigned", /*default=*/"0",
     "Zero offset for indicating the location of zeroes in convolution filter "
     "(useful for 16x16 scheme)">,
    Option<"dupFactor", "dup-factor", "unsigned", /*default=*/"2",
     "Duplication factor for each value in convolution filter "
     "(useful for 8x8 scheme)">,
  ];
}

#endif // AIE_DIALECT_AIEVEC_TRANSFORMS_PASSES
