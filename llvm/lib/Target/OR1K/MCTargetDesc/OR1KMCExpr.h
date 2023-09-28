//===- OR1KMCExpr.h - OR1K specific MC expression classes -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef OR1K_MCEXPR_H
#define OR1K_MCEXPR_H

#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCValue.h"

namespace llvm {

class OR1KMCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_OR1K_None,
    VK_OR1K_ABS_HI,
    VK_OR1K_ABS_LO,
    VK_OR1K_PLT,
    VK_OR1K_GOT,
    VK_OR1K_GOTPCHI,
    VK_OR1K_GOTPCLO,
    VK_OR1K_GOTOFFHI,
    VK_OR1K_GOTOFFLO,
  };

private:
  const MCExpr *Expr;
  const VariantKind Kind;

  explicit OR1KMCExpr(const MCExpr *Expr, VariantKind Kind)
      : Expr(Expr), Kind(Kind) {}

public:
  static const OR1KMCExpr *create(const MCExpr *Expr, VariantKind Kind,
                                  MCContext &Ctx);

  VariantKind getKind() const { return Kind; }

  const MCExpr *getSubExpr() const { return Expr; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;

  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;

  void visitUsedExpr(MCStreamer &Streamer) const override;

  MCFragment *findAssociatedFragment() const override;

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static VariantKind getVariantKindForName(StringRef Name);
  static StringRef getVariantKindName(VariantKind Kind);
};

} // end namespace llvm

#endif
