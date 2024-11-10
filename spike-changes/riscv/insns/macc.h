// fs2 <- fs + mem[s2] * mem[f3]
require_extension('F');
require_fp;
WRITE_FREG(19, 
    f32_add(
        f32_add(
            f32(READ_FREG(19)), 
            f32_mul(
                f32(MMU.load<uint32_t>(READ_REG(18))), 
                f32(MMU.load<uint32_t>(READ_REG(19)))
            )
        ),
        f32_mul(
            f32(MMU.load<uint32_t>(READ_REG(18)+4)), 
            f32(MMU.load<uint32_t>(READ_REG(19)+4))
        )
    )
);
