# RUN: llvm-mc -arch=or1k -disassemble %s | FileCheck %s

    0x00 0x18 0x22 0xc8
# CHECK: lf.add.s r1, r2, r3

    0x03 0x18 0x22 0xc8
# CHECK: lf.div.s r1, r2, r3

    0x05 0x00 0x22 0xc8
# CHECK: lf.ftoi.s r1, r2

    0x04 0x00 0x22 0xc8
# CHECK: lf.itof.s r1, r2

    0x02 0x18 0x22 0xc8
# CHECK: lf.mul.s r1, r2, r3

    0x06 0x18 0x22 0xc8
# CHECK: lf.rem.s r1, r2, r3

    0x01 0x18 0x22 0xc8
# CHECK: lf.sub.s r1, r2, r3
