//===----- COFF_x86_64.cpp - JIT linker implementation for COFF/x86_64 ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// COFF/x86_64 jit-link implementation.
//
//===----------------------------------------------------------------------===//

#include "llvm/ExecutionEngine/JITLink/COFF_arm64.h"
#include "COFFLinkGraphBuilder.h"
#include "llvm/ExecutionEngine/JITLink/aarch64.h"
#include "llvm/Object/COFF.h"

#define DEBUG_TYPE "jitlink"

using namespace llvm;
using namespace llvm::jitlink;

namespace {

enum EdgeKind_coff_arm64 : Edge::Kind { AAA };

class COFFJITLinker_arm64 : public JITLinker<COFFJITLinker_arm64> {
  friend class JITLinker<COFFJITLinker_arm64>;

public:
  COFFJITLinker_arm64(std::unique_ptr<JITLinkContext> Ctx,
                       std::unique_ptr<LinkGraph> G,
                       PassConfiguration PassConfig)
      : JITLinker(std::move(Ctx), std::move(G), std::move(PassConfig)) {}

private:
  Error applyFixup(LinkGraph &G, Block &B, const Edge &E) const {
    return aarch64::applyFixup(G, B, E);
  }
};

class COFFLinkGraphBuilder_arm64 : public COFFLinkGraphBuilder {
private:
  Error addRelocations() override { return Error::success(); }

public:
  COFFLinkGraphBuilder_arm64(const object::COFFObjectFile &Obj, const Triple T,
                             const SubtargetFeatures Features)
      : COFFLinkGraphBuilder(Obj, std::move(T), std::move(Features),
                             getCOFFARM64RelocationKindName) {}
};
} // namespace

namespace llvm {
namespace jitlink {

/// Return the string name of the given COFF ARM64 edge kind.
const char *getCOFFARM64RelocationKindName(Edge::Kind R) { return "aaa"; }

Expected<std::unique_ptr<LinkGraph>>
createLinkGraphFromCOFFObject_arm64(MemoryBufferRef ObjectBuffer) {
  LLVM_DEBUG({
    dbgs() << "Building jitlink graph for new input "
           << ObjectBuffer.getBufferIdentifier() << "...\n";
  });

  auto COFFObj = object::ObjectFile::createCOFFObjectFile(ObjectBuffer);
  if (!COFFObj)
    return COFFObj.takeError();

  auto Features = (*COFFObj)->getFeatures();
  if (!Features)
    return Features.takeError();

  return COFFLinkGraphBuilder_arm64(**COFFObj, (*COFFObj)->makeTriple(),
                                    std::move(*Features))
      .buildGraph();
}

void link_COFF_arm64(std::unique_ptr<LinkGraph> G,
                     std::unique_ptr<JITLinkContext> Ctx) {
  PassConfiguration Config;
  const Triple &TT = G->getTargetTriple();

  if (auto Err = Ctx->modifyPassConfig(*G, Config))
    return Ctx->notifyFailed(std::move(Err));

  COFFJITLinker_arm64::link(std::move(Ctx), std::move(G), std::move(Config));
}
} // namespace jitlink
} // namespace llvm