// mem[imm] <- fs2
require_extension('F');
require_fp;
MMU.store<uint32_t>(insn.macc_imm(), READ_FREG(19).v[0]);
WRITE_FREG(19, f32(0));