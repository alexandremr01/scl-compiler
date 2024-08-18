MEMORY
{
   RAM : ORIGIN = 0x10000, LENGTH = 0x4000
}
SECTIONS
{
    everything :
    {
    . = 0x0b0;
	 *(.*)
    } >RAM
}
