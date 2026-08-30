/***********************************************************
* 	
*	EDMA McASP Module  
*                              
*	File Name : EDMA_McASP.c 
*	Date Created :
*	Last modified :
*
************************************************************/

#include "EDMA_McASP.h"
#include "MCASP.h"

CSL_Edma3ccRegsOvly edma3ccRegs = (CSL_Edma3ccRegsOvly)(CSL_EDMA30CC_0_REGS);

void ConfigEDMAForR( Uint16 ch, Uint32 dst, Uint16 buffer_size, Uint16 data_witdth, Uint16 link_num, Uint16 tcint )
{
	edma3ccRegs->PARAMSET[ch].OPT =   	  	  CSL_FMK(TPCC_PARAM_OPT_TCC, TCC_R)
											| CSL_FMKT(TPCC_PARAM_OPT_SYNCDIM, ASYNC);
	if( tcint )
		edma3ccRegs->PARAMSET[ch].OPT |=   	  CSL_FMKT(TPCC_PARAM_OPT_TCINTEN, ENABLE);
		
	edma3ccRegs->PARAMSET[ch].SRC = 	  	 (Uint32)(&mcaspRegs->RBUF14);
	edma3ccRegs->PARAMSET[ch].DST = 	  	  dst;
	edma3ccRegs->PARAMSET[ch].A_B_CNT =    	  CSL_FMK(TPCC_PARAM_A_B_CNT_BCNT, buffer_size)
											| CSL_FMK(TPCC_PARAM_A_B_CNT_ACNT, data_witdth);
	edma3ccRegs->PARAMSET[ch].SRC_DST_BIDX =  CSL_FMK(TPCC_PARAM_SRC_DST_BIDX_DSTBIDX, data_witdth);
	edma3ccRegs->PARAMSET[ch].LINK_BCNTRLD =  (Uint32)(&(edma3ccRegs->PARAMSET[link_num]));
	edma3ccRegs->PARAMSET[ch].SRC_DST_CIDX =  0;
	edma3ccRegs->PARAMSET[ch].CCNT = 	  	  1;
}

void ConfigEDMAForX( Uint16 ch, Uint32 src, Uint16 buffer_size, Uint16 data_witdth, Uint16 link_num, Uint16 tcint )
{
	edma3ccRegs->PARAMSET[ch].OPT = 	  	  CSL_FMK(TPCC_PARAM_OPT_TCC, TCC_X)
											| CSL_FMKT(TPCC_PARAM_OPT_SYNCDIM, ASYNC);

	if( tcint )
		edma3ccRegs->PARAMSET[ch].OPT |=   	  CSL_FMKT(TPCC_PARAM_OPT_TCINTEN, ENABLE);

	edma3ccRegs->PARAMSET[ch].SRC = 	  	  src;
	edma3ccRegs->PARAMSET[ch].DST = 	  	  (Uint32)(&mcaspRegs->XBUF13);
	edma3ccRegs->PARAMSET[ch].A_B_CNT =    	  CSL_FMK(TPCC_PARAM_A_B_CNT_BCNT, buffer_size)
											| CSL_FMK(TPCC_PARAM_A_B_CNT_ACNT, data_witdth);
	edma3ccRegs->PARAMSET[ch].SRC_DST_BIDX =  CSL_FMK(TPCC_PARAM_SRC_DST_BIDX_SRCBIDX, data_witdth);
	edma3ccRegs->PARAMSET[ch].LINK_BCNTRLD =  (Uint32)(&(edma3ccRegs->PARAMSET[link_num]));
	edma3ccRegs->PARAMSET[ch].SRC_DST_CIDX =  0;
	edma3ccRegs->PARAMSET[ch].CCNT = 		  1;
}


void EDMAReset( void )
{
	edma3ccRegs->TPCC_EMCR       = 0xFFFFFFFF;
    edma3ccRegs->TPCC_CCERRCLR   = 0xFFFFFFFF;
    edma3ccRegs->TPCC_SECR       = 0xFFFFFFFF;
    edma3ccRegs->TPCC_ECR        = 0xFFFFFFFF;
    edma3ccRegs->TPCC_ICR        = 0xFFFFFFFF;
    edma3ccRegs->TPCC_EECR       = 0xFFFFFFFF;
    edma3ccRegs->TPCC_IECR       = 0xFFFFFFFF;
}



	

	

