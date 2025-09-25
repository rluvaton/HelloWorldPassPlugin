#pragma once

#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"

namespace HelloWorldPlugin {

class HelloWorldPass : public llvm::PassInfoMixin<HelloWorldPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM);
  
  llvm::PreservedAnalyses run(llvm::Loop &L, llvm::LoopAnalysisManager &AM,
                                            llvm::LoopStandardAnalysisResults &AR,
                                            llvm::LPMUpdater &);
};
} // namespace HelloWorldPlugin

void registerHelloWorldPlugin(llvm::PassBuilder &PB);