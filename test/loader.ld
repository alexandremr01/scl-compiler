MEMORY
{
   RAM : ORIGIN = 0x10000, LENGTH = 0x12000
}
SECTIONS
{
    everything :
    {
    . = 0x1000;
	 *(.*)
    } >RAM
}
