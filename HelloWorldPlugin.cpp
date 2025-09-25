//===- SCCP.cpp - Sparse Conditional Constant Propagation -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements sparse conditional constant propagation and merging:
//
// Specifically, this:
//   * Assumes values are constant unless proven otherwise
//   * Assumes BasicBlocks are dead unless proven otherwise
//   * Proves values to be constant, and replaces them with constants
//   * Proves conditional branches to be unconditional
//
//===----------------------------------------------------------------------===//

#include "HelloWorldPlugin.h"

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Scalar/IndVarSimplify.h"

// #include "llvm/Transforms/Scalar/SCCP.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueLatticeUtils.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"
// #include "llvm/Transforms/Utils/SCCPSolver.h"
#include "MySCCPSolver.h"

namespace HelloWorldPlugin {

using namespace llvm;

#define DEBUG_TYPE "sccp"

STATISTIC(NumInstRemoved, "Number of instructions removed");
STATISTIC(NumDeadBlocks, "Number of basic blocks unreachable");
STATISTIC(NumInstReplaced,
          "Number of instructions replaced with (simpler) instruction");

// runSCCP() - Run the Sparse Conditional Constant Propagation algorithm,
// and return true if the function was modified.
static bool runSCCP(Function &F, const DataLayout &DL,
                    const TargetLibraryInfo *TLI, DomTreeUpdater &DTU) {
  llvm::errs() << "SCCP on function '" << F.getName() << "'\n";
  MySCCPSolver Solver(
      DL, [TLI](Function &F) -> const TargetLibraryInfo & { return *TLI; },
      F.getContext());

  // While we don't do any actual inter-procedural analysis, still track
  // return values so we can infer attributes.
  if (canTrackReturnsInterprocedurally(&F))
    Solver.addTrackedFunction(&F);

  // Mark the first block of the function as being executable.
  Solver.markBlockExecutable(&F.front());

  // Initialize arguments based on attributes.
  for (Argument &AI : F.args())
    Solver.trackValueOfArgument(&AI);

  // Solve for constants.
  bool ResolvedUndefs = true;
  while (ResolvedUndefs) {
    Solver.solve();
    llvm::errs() << "RESOLVING UNDEFs\n";
    ResolvedUndefs = Solver.resolvedUndefsIn(F);
  }

  bool MadeChanges = false;

  // If we decided that there are basic blocks that are dead in this function,
  // delete their contents now.  Note that we cannot actually delete the blocks,
  // as we cannot modify the CFG of the function.

  SmallPtrSet<Value *, 32> InsertedValues;
  SmallVector<BasicBlock *, 8> BlocksToErase;
  for (BasicBlock &BB : F) {
    if (!Solver.isBlockExecutable(&BB)) {
      llvm::errs() << "  BasicBlock Dead:" << BB;
      ++NumDeadBlocks;
      BlocksToErase.push_back(&BB);
      MadeChanges = true;
      continue;
    } else {
      llvm::errs() << "  BasicBlock Reachable:" << BB;

      llvm::errs() << "############\n";
    }

    MadeChanges |= Solver.simplifyInstsInBlock(BB, InsertedValues,
                                               NumInstRemoved, NumInstReplaced);
  }

  // Remove unreachable blocks and non-feasible edges.
  for (BasicBlock *DeadBB : BlocksToErase)
    NumInstRemoved += changeToUnreachable(&*DeadBB->getFirstNonPHIIt(),
                                          /*PreserveLCSSA=*/false, &DTU);

  BasicBlock *NewUnreachableBB = nullptr;
  for (BasicBlock &BB : F)
    MadeChanges |= Solver.removeNonFeasibleEdges(&BB, DTU, NewUnreachableBB);

  for (BasicBlock *DeadBB : BlocksToErase)
    if (!DeadBB->hasAddressTaken())
      DTU.deleteBB(DeadBB);

  Solver.inferReturnAttributes();

  return MadeChanges;
}

llvm::PreservedAnalyses HelloWorldPass::run(llvm::Function &F,
                                            llvm::FunctionAnalysisManager &AM) {
  const DataLayout &DL = F.getDataLayout();
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  llvm::ScalarEvolution &ScalarEvolutionAnalysisResult =
      AM.getResult<ScalarEvolutionAnalysis>(F);

  ScalarEvolutionAnalysisResult.print(llvm::errs());

  // auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  // DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);
  // if (!runSCCP(F, DL, &TLI, DTU))
  return PreservedAnalyses::all();

  // auto PA = PreservedAnalyses();
  // PA.preserve<DominatorTreeAnalysis>();
  // return PA;
  // llvm::errs() << F.getName() << "\n";

  // for (llvm::BasicBlock &B : F) {
  //   llvm::errs() << " llvm::BasicBlock:\n";

  //   B.print(
  //     llvm::errs(),
  //     nullptr,
  //     true,
  //     true
  //   );

  //   for (llvm::Instruction &I : B) {

  //     llvm::errs() << "  llvm::Instruction:";
  //     I.print(
  //       llvm::errs(),
  //       true // Is for debug
  //     );
  //     llvm::errs() << "\n";

  //     if (auto *op = dyn_cast<llvm::BinaryOperator>(&I)) {
  //       // Insert at the point where the instruction `op`
  //       // appears.
  //       llvm::IRBuilder<> builder(op);

  //       // Make a multiply with the same operands as `op`.
  //       llvm::Value *lhs = op->getOperand(0);
  //       llvm::Value *rhs = op->getOperand(1);
  //       llvm::Value *mul = builder.CreateMul(lhs, rhs);

  //       // Everywhere the old instruction was used as an
  //       // operand, use our new multiply instruction instead.
  //       for (auto &U : op->uses()) {
  //         // A User is anything with operands.
  //         llvm::User *user = U.getUser();
  //         user->setOperand(U.getOperandNo(), mul);
  //       }

  //       // We modified the code.
  //       return llvm::PreservedAnalyses::none();
  //     }
  //   }
  // }

  // llvm::MySCCPPass pass;
  // auto res = pass.run(F, AM);

  // return res;
  // return llvm::PreservedAnalyses::all();
}

PreservedAnalyses HelloWorldPass::run(Loop &L, LoopAnalysisManager &AM,
                                      LoopStandardAnalysisResults &AR,
                                      LPMUpdater &) {
  Function *F = L.getHeader()->getParent();

  // Access ScalarEvolution through LoopStandardAnalysisResults
  llvm::ScalarEvolution &ScalarEvolutionAnalysisResult = AR.SE;
  if (!L.isLoopSimplifyForm()) {
    errs() << "#### isLoopSimplifyForm\n";
    return PreservedAnalyses::all();
  }

  errs() << "######## Loop \n";
  L.print(errs(), true);
  errs() << "test\n";

  // Now you can use ScalarEvolution analysis
  // For example, get the backedge taken count:
  const SCEV *BackedgeTakenCount =
      ScalarEvolutionAnalysisResult.getBackedgeTakenCount(&L);
  errs() << "Backedge taken count: ";
  BackedgeTakenCount->print(errs());
  errs() << "\n";

  
  // Get and print the induction variable
  PHINode *InductionVariable = L.getInductionVariable(ScalarEvolutionAnalysisResult);
  if (InductionVariable) {
    errs() << "Induction variable found: ";
    InductionVariable->print(errs());
    errs() << "\n";
    
    // Get the SCEV expression for the induction variable
    const SCEV *InductionSCEV = ScalarEvolutionAnalysisResult.getSCEV(InductionVariable);
    errs() << "Induction variable SCEV: ";
    InductionSCEV->print(errs());
    errs() << "\n";
  } else {
    errs() << "No canonical induction variable found\n";
    
    // Try to find any induction variables
    errs() << "Looking for any induction variables:\n";
    for (PHINode &PHI : L.getHeader()->phis()) {
      if (ScalarEvolutionAnalysisResult.isSCEVable(PHI.getType())) {
        const SCEV *PHISCEV = ScalarEvolutionAnalysisResult.getSCEV(&PHI);
        if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(PHISCEV)) {
          if (AddRec->getLoop() == &L) {
            errs() << "  Induction-like PHI: ";
            PHI.print(errs());
            errs() << "\n  SCEV: ";
            PHISCEV->print(errs());
            errs() << "\n";
          }
        }
      }
    }
  }
  

  return PreservedAnalyses::all();
}
} // namespace HelloWorldPlugin

void registerHelloWorldPlugin(llvm::PassBuilder &PB) {
  PB.registerPipelineParsingCallback(
      [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
         llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
        if (Name == "hello-world-pass") {
          FPM.addPass(HelloWorldPlugin::HelloWorldPass());
          {
            llvm::LoopPassManager LPM;
            LPM.addPass(HelloWorldPlugin::HelloWorldPass());
            LPM.addPass(llvm::IndVarSimplifyPass());
            FPM.addPass(
                llvm::createFunctionToLoopPassAdaptor<llvm::LoopPassManager>(
                    std::move(LPM), /*UseMemorySSA=*/false,
                    /*UseBlockFrequencyInfo=*/true));
          }

          // FPM.addPass(HelloWorldPlugin::MySCCPPass());
          return true;
        }
        return false;
      });

  // THIS IS THE IMPORTANT PART - Register required analyses
  //
  // PB.registerAnalysisRegistrationCallback(
  //     [](llvm::CGSCCAnalysisManager &AM) {

  //     });
  PB.registerAnalysisRegistrationCallback(
      [](llvm::FunctionAnalysisManager &FAM) {
        FAM.registerPass([] { return llvm::TargetIRAnalysis(); });
        FAM.registerPass([] { return llvm::AAManager(); });
        FAM.registerPass([] { return llvm::ScalarEvolutionAnalysis(); });
        // FAM.registerPass([] { return llvm::ScalarEvolutionAnalysis(); });
        FAM.registerPass([] { return llvm::PassInstrumentationAnalysis(); });
        FAM.registerPass([] { return llvm::TargetLibraryAnalysis(); });
        // AM.registerPass([] { return llvm::AssumptionAnalysis(); });
        FAM.registerPass([] { return llvm::DominatorTreeAnalysis(); });
        FAM.registerPass([] { return llvm::LoopAnalysis(); });

        // FAM.registerPass([&] { return llvm::DominatorTreeAnalysis(); });
        // FAM.registerPass([&] { return HelloWorldPlugin::HelloWorldPass(); });
        // FAM.registerPass([&] { return HelloWorldPlugin::HelloWorldPass(); });
        // FAM.registerPass([&] { return llvm::ScalarEvolutionAnalysis(); });

        // // We need DominatorTreeAnalysis for LoopAnalysis.
        // FAM.registerPass([&] { return llvm::DominatorTreeAnalysis(); });
        // FAM.registerPass([&] { return llvm::LoopAnalysis(); });
        // // We also allow loop passes to assume a set of other analyses and so
        // need
        // // those.
        // FAM.registerPass([&] { return llvm::AAManager(); });
        // // FAM.registerPass([&] { return llvm::AssumptionAnalysis(); });
        // // FAM.registerPass([&] { return llvm::BlockFrequencyAnalysis(); });
        // // FAM.registerPass([&] { return llvm::BranchProbabilityAnalysis();
        // });
        // // FAM.registerPass([&] { return llvm::PostDominatorTreeAnalysis();
        // });
        // // FAM.registerPass([&] { return llvm::MemorySSAAnalysis(); });
        // FAM.registerPass([&] { return llvm::ScalarEvolutionAnalysis(); });
        // FAM.registerPass([&] { return llvm::TargetLibraryAnalysis(); });
        // FAM.registerPass([&] { return llvm::TargetIRAnalysis(); });

        // // Register required pass instrumentation analysis.
        llvm::LoopAnalysisManager LAM;
        // LAM.registerPass([&] { return HelloWorldPlugin::HelloWorldPass(); });
        // LAM.registerPass([] { return llvm::ScalarEvolutionAnalysis(); });

        // // llvm::ModuleAnalysisManager MAM;

        // // // Cross-register proxies.
        LAM.registerPass(
            [&] { return llvm::FunctionAnalysisManagerLoopProxy(FAM); });
        // FAM.registerPass([&] { return
        // llvm::LoopAnalysisManagerFunctionProxy(LAM); }); FAM.registerPass([&]
        // { return llvm::ModuleAnalysisManagerFunctionProxy(MAM); });
        // MAM.registerPass([&] { return
        // llvm::FunctionAnalysisManagerModuleProxy(FAM); });
      });

  // // NEW: Register analyses for LoopAnalysisManager
  //  PB.registerAnalysisRegistrationCallback(
  //      [](llvm::LoopAnalysisManager &LAM) {
  //        LAM.registerPass([] { return llvm::ScalarEvolutionAnalysis(); });
  //      });
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HelloWorldPlugin", "v0.1",
          registerHelloWorldPlugin};
}
