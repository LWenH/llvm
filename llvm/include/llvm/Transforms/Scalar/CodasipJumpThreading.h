//===- CodasipJumpThreading.h -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_CODASIP_JUMPTHREADING_CODASIPJUMPTHREADING_H_
#define INCLUDE_CODASIP_JUMPTHREADING_CODASIPJUMPTHREADING_H_

#include <memory>
#include <utility>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/JTBuilder.h"
#include "llvm/IR/JTGraph.h"
#include "llvm/IR/JTThreadableNode.h"
#include "llvm/Transforms/Scalar/JTAnalyzer.h"
#include "llvm/Transforms/Scalar/JTDebug.h"
#include "llvm/Transforms/Scalar/JTPhiRebuilder.h"
#include "llvm/IR/JTFwd.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/Utils/ValueMapper.h"



namespace llvm {

class BasicBlock;
class BinaryOperator;
class BranchInst;
class CmpInst;
class Constant;
class Function;
class Instruction;
class IntrinsicInst;
class LazyValueInfo;
class LoadInst;
class PHINode;
class TargetLibraryInfo;
class Value;

/// This pass performs 'jump threading', which looks at blocks that have
/// multiple predecessors and multiple successors.  If one or more of the
/// predecessors of the block can be proven to always jump to one of the
/// successors, we forward the edge from the predecessor to the successor by
/// duplicating the contents of this block.
///
/// An example of when this can occur is code like this:
///
///   if () { ...
///     X = 4;
///   }
///   if (X < 3) {
///
/// In this case, the unconditional branch at the end of the first if can be
/// revectored to the false side of the second if.

namespace jumpthreading {
  class CodasipJumpThreading {
  public:
    // static char ID; // Pass identification

    CodasipJumpThreading(int T = -1){}

    bool runPass(Function &F, const LoopInfo &LI, const TargetLibraryInfo &TLI);

    // void getAnalysisUsage(AnalysisUsage &AU) const override {
    //   //AU.addRequired<LazyValueInfoWrapperPass>();
    //   AU.addRequired<LoopInfoWrapperPass>();
    //   AU.addRequired<TargetLibraryInfoWrapperPass>();
    // }

  private:
    void createJTGraph(JTGraph &G, Function &F, JTBuilder &Builder);
    // Merging functions
    bool mergeThreadableGraphs(JTAnalyzer::ThreadableGraphs &Graphs, const unsigned int Iteration);
    static bool merge(JTGraph &Dst, JTGraph &Src);
    static void mergeSameEntryEdgeGraphs(JTAnalyzer::ThreadableGraphs &Graphs);
    static bool mergeSameEntryEdgeGraphs(JTGraph &Dst, JTGraph &Src);
    // Attaching functions
    void attachGraphs(JTGraph &Dst, JTAnalyzer::ThreadableGraphs &Src, bool &Changed);
    //static bool hasIntersection(ThreadableGraphs &Src);
    void attachGraph(JTGraph &Dst, JTGraph &Src, bool &Changed);
    // LLVM final processing functions
    static void cloneBlocks(JTGraph::JTBlocks &Blocks, JTGraph &G, JTBuilder &Builder);
    void fixTerminators(JTGraph::JTBlocks &Blocks, Function &F);
    void fixConditionalBranch(Instruction *Terminator, JTBlock &Block);
    void fixSwitch(Instruction *Terminator, JTBlock &Block);
    void placeDuplicatedBlocksIntoFunction(Function &F, JTGraph::JTBlocks &Blocks);

    // Utility functions
    void createDuplicateMap(ValueToValueMapTy &Duplicate2Original, JTBlock &Duplicate);
  };
}

class CodasipJumpThreadingPass : public PassInfoMixin<CodasipJumpThreadingPass> {
  jumpthreading::CodasipJumpThreading pass;
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

};

} // end namespace llvm

#endif  // INCLUDE_CODASIP_JUMPTHREADING_CODASIPJUMPTHREADING_H_
