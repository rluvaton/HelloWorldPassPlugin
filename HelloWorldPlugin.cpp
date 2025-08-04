#include "HelloWorldPlugin.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

namespace HelloWorldPlugin {
llvm::PreservedAnalyses HelloWorldPass::run(llvm::Function &F,
                                            llvm::FunctionAnalysisManager &AM) {
  llvm::errs() << F.getName() << "\n";
  return llvm::PreservedAnalyses::all();
}
} // namespace HelloWorldPlugin

void registerHelloWorldPlugin(llvm::PassBuilder &PB) {
  PB.registerPipelineParsingCallback(
      [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
         llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
        if (Name == "hello-world-pass") {
          FPM.addPass(HelloWorldPlugin::HelloWorldPass());
          return true;
        }
        return false;
      });
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HelloWorldPlugin", "v0.1",
          registerHelloWorldPlugin};
}