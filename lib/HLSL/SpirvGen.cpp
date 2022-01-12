///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SpirvGen.cpp                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/HLSL/DxilGenerationPass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"

using namespace llvm;

class SpirvGen : public ModulePass {
public:
  static char ID;
  SpirvGen() : ModulePass(ID) {
    initializeSpirvGenPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override;
};
char SpirvGen::ID;

bool SpirvGen::runOnModule(Module &M) {
  printf("!!! Running SpirvGen module pass\n");
  bool Changed = false;
  printf("!!! module = %s\n", M.getName().data());
  for (Function &F : M) {
    printf("!!! function = %s\n", F.getName().data());
    for (BasicBlock &BB : F) {
      printf("!!! basic block = %s\n", BB.getName().data());
      for (auto it = BB.begin(), end = BB.end(); it != end;) {
        Instruction *I = &*(it++);
        printf("!!! opcode = %s\n", I->getOpcodeName());
      }
    }
  }


  return Changed;
}

ModulePass *llvm::createSpirvGenPass() {
  return new SpirvGen();
}

INITIALIZE_PASS_BEGIN(SpirvGen, "spirv-gen", "Generate SPIR-V from DXIL", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(StructurizeCFG)
INITIALIZE_PASS_END(SpirvGen, "spirv-gen", "Generate SPIR-V from DXIL", false,
                    false)
