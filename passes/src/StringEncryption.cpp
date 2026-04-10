#include "obfuscator/Transforms.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#include <random>

using namespace llvm;

namespace obfuscator {

static bool isEligibleGlobal(GlobalVariable &GV) {
  if (!GV.hasInitializer() || !GV.isConstant())
    return false;

  if (GV.getName().starts_with("llvm."))
    return false;

  if (!GV.hasPrivateLinkage() && !GV.hasInternalLinkage())
    return false;

  if (GV.hasSection())
    return false;

  auto *CDS = dyn_cast<ConstantDataSequential>(GV.getInitializer());
  if (!CDS || !CDS->isString())
    return false;

  if (CDS->getNumElements() <= 1)
    return false;

  return true;
}

PreservedAnalyses StringEncryption::run(Module &M, ModuleAnalysisManager &AM) {
  SmallVector<GlobalVariable *, 16> Candidates;
  for (GlobalVariable &GV : M.globals()) {
    if (isEligibleGlobal(GV))
      Candidates.push_back(&GV);
  }

  if (Candidates.empty())
    return PreservedAnalyses::all();

  LLVMContext &Ctx = M.getContext();
  IntegerType *I8Ty = Type::getInt8Ty(Ctx);
  IntegerType *I64Ty = Type::getInt64Ty(Ctx);

  FunctionType *CtorTy = FunctionType::get(Type::getVoidTy(Ctx), false);
  Function *CtorFn = Function::Create(CtorTy, GlobalValue::InternalLinkage,
                                      "__strenc_ctor", &M);
  BasicBlock *EntryBB = BasicBlock::Create(Ctx, "entry", CtorFn);
  IRBuilder<> Builder(EntryBB);

  std::mt19937 RNG(std::random_device{}());
  std::uniform_int_distribution<unsigned> Dist(1, 255);

  BasicBlock *CurrentBB = EntryBB;

  for (GlobalVariable *GV : Candidates) {
    auto *CDS = cast<ConstantDataSequential>(GV->getInitializer());
    StringRef RawData = CDS->getRawDataValues();
    unsigned Len = RawData.size();

    uint8_t Key = static_cast<uint8_t>(Dist(RNG));

    SmallVector<uint8_t, 64> Encrypted(Len);
    for (unsigned i = 0; i < Len; i++)
      Encrypted[i] = static_cast<uint8_t>(RawData[i]) ^ Key;

    GV->setInitializer(ConstantDataArray::get(Ctx, Encrypted));
    GV->setConstant(false);

    // Emit a decryption loop: for (i = 0; i < Len; ++i) GV[i] ^= Key
    BasicBlock *LoopHdr = BasicBlock::Create(Ctx, "", CtorFn);
    BasicBlock *LoopBody = BasicBlock::Create(Ctx, "", CtorFn);
    BasicBlock *LoopExit = BasicBlock::Create(Ctx, "", CtorFn);

    Builder.SetInsertPoint(CurrentBB);
    Builder.CreateBr(LoopHdr);

    Builder.SetInsertPoint(LoopHdr);
    PHINode *Idx = Builder.CreatePHI(I64Ty, 2);
    Idx->addIncoming(ConstantInt::get(I64Ty, 0), CurrentBB);
    Value *Cmp = Builder.CreateICmpULT(Idx, ConstantInt::get(I64Ty, Len));
    Builder.CreateCondBr(Cmp, LoopBody, LoopExit);

    Builder.SetInsertPoint(LoopBody);
    Value *ElemPtr = Builder.CreateGEP(I8Ty, GV, Idx);
    Value *Byte = Builder.CreateLoad(I8Ty, ElemPtr);
    Value *Decrypted = Builder.CreateXor(Byte, ConstantInt::get(I8Ty, Key));
    Builder.CreateStore(Decrypted, ElemPtr);
    Value *NextIdx = Builder.CreateAdd(Idx, ConstantInt::get(I64Ty, 1));
    Idx->addIncoming(NextIdx, LoopBody);
    Builder.CreateBr(LoopHdr);

    CurrentBB = LoopExit;
  }

  Builder.SetInsertPoint(CurrentBB);
  Builder.CreateRetVoid();

  appendToGlobalCtors(M, CtorFn, 0);

  return PreservedAnalyses::none();
}

} // namespace obfuscator
