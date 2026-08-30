/****************************************************************************/
/*  OMAPL138.cmd                                                            */
/*  Copyright (c) 2010 Texas Instruments Incorporated                       */
/*  Author: Rafael de Souza                                                 */
/*                                                                          */
/*    Description: This file is a sample linker command file that can be    */
/*                 used for linking programs built with the C compiler and  */
/*                 running the resulting .out file on an OMAPL138           */
/*                 device.  Use it as a guideline.  You will want to        */
/*                 change the memory layout to match your specific          */
/*                 target system.  You may want to change the allocation    */
/*                 scheme according to the size of your program.            */
/*                                                                          */
/*    Usage:       The map below contains the local memory for each core    */
/*                 Use the linker option --define=DSP_CORE=n                */
/*                 Where n defines the core used: DSP (n=1) or ARM (n=0)    */
/*                                                                          */
/****************************************************************************/

MEMORY
{
    DSPL2RAM     o = 0x00800000  l = 0x00040000  /* 256kB L2 DSP local RAM */
    SHDSPL2RAM   o = 0x11800000  l = 0x00040000  /* 256kB L2 Shared Internal RAM */       

    SHRAM        o = 0x80000000  l = 0x00020000  /* 128kB Shared RAM */
    DDR2         o = 0xC0000000  l = 0x08000000  /* 128MB DDR2 Data */
}

SECTIONS
{
    .text          >  DSPL2RAM
    .stack         >  DSPL2RAM
    .bss           >  DSPL2RAM
    .cio           >  DSPL2RAM
    .const         >  DSPL2RAM
    .data          >  DSPL2RAM
    .switch        >  DSPL2RAM
    .sysmem        >  DSPL2RAM
    .far           >  DSPL2RAM
    .args          >  DSPL2RAM
    .ppinfo        >  DSPL2RAM
    .ppdata        >  DSPL2RAM
    
    /* TI-ABI or COFF sections */
    .pinit         >  DSPL2RAM
    .cinit         >  DSPL2RAM
    
    /* EABI sections */
    .binit         >  DSPL2RAM
    .init_array    >  DSPL2RAM
    .neardata      >  DSPL2RAM
    .fardata       >  DSPL2RAM
    .rodata        >  DSPL2RAM
    .c6xabi.exidx  >  DSPL2RAM
    .c6xabi.extab  >  DSPL2RAM
}
