#pragma once

#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"

namespace HelloWorldPlugin {

class HelloWorldPass : public llvm::PassInfoMixin<HelloWorldPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM);
};
} // namespace HelloWorldPlugin

void registerHelloWorldPlugin(llvm::PassBuilder &PB);