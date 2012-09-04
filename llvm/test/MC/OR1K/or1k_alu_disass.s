# RUN: llvm-mc -arch=or1k -mattr=mul,div,ror -disassemble %s | FileCheck %s

    0x00 0x18 0x22 0xe0
# CHECK: l.add r1, r2, r3

    0x02 0x00 0x64 0x9c
# CHECK: l.addi r3, r4, 2

    0x03 0x18 0x22 0xe0
# CHECK: l.and r1, r2, r3

    0x02 0x00 0x64 0xa4
# CHECK: l.andi r3, r4, 2

    0x09 0x18 0x22 0xe0
# CHECK: l.div r1, r2, r3

    0x0a 0x18 0x22 0xe0
# CHECK: l.divu r1, r2, r3

    0x02 0x00 0x20 0x18
# CHECK: l.movhi r1, 2

    0x06 0x18 0x22 0xe0
# CHECK: l.mul r1, r2, r3

    0x02 0x00 0x64 0xb0
# CHECK: l.muli r3, r4, 2

    0x0b 0x18 0x22 0xe0
# CHECK: l.mulu r1, r2, r3

    0x04 0x18 0x22 0xe0
# CHECK: l.or r1, r2, r3

    0x02 0x00 0x64 0xa8
# CHECK: l.ori r3, r4, 2

    0xc8 0x18 0x22 0xe0
# CHECK: l.ror r1, r2, r3

    0xc2 0x00 0x64 0xb8
# CHECK: l.rori r3, r4, 2

    0x08 0x18 0x22 0xe0
# CHECK: l.sll r1, r2, r3

    0x02 0x00 0x64 0xb8
# CHECK: l.slli r3, r4, 2

    0x88 0x18 0x22 0xe0
# CHECK: l.sra r1, r2, r3

    0x82 0x00 0x64 0xb8
# CHECK: l.srai r3, r4, 2

    0x48 0x18 0x22 0xe0
# CHECK: l.srl r1, r2, r3

    0x42 0x00 0x64 0xb8
# CHECK: l.srli r3, r4, 2

    0x02 0x18 0x22 0xe0
# CHECK: l.sub r1, r2, r3

    0x05 0x18 0x22 0xe0
# CHECK: l.xor r1, r2, r3

    0x02 0x00 0x64 0xac
# CHECK: l.xori r3, r4, 2