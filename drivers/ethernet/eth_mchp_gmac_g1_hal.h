#ifndef ZEPHYR_DRIVERS_ETHERNET_ETH_MCHP_GMAC_G1_HAL_H_
#define ZEPHYR_DRIVERS_ETHERNET_ETH_MCHP_GMAC_G1_HAL_H_

#if defined(CONFIG_SOC_FAMILY_MICROCHIP_PIC32CK_SG_GC)
#define ETH_MCHP_GMAC_G1_SYNC_TIMEOUT_US 1000U

#define gmac_registers_t eth_registers_t

#define GMAC_AE                ETH_AE
#define GMAC_CSE               ETH_CSE
#define GMAC_DCFGR             ETH_DCFGR
#define GMAC_DCFGR_DRBS        ETH_DCFGR_DRBS
#define GMAC_DCFGR_FBLDO_INCR4 ETH_DCFGR_FBLDO(0x4)
#define GMAC_DCFGR_RXBMS       ETH_DCFGR_RXBMS
#define GMAC_DCFGR_TXCOEN_Msk  ETH_DCFGR_TXCOEN_Msk
#define GMAC_HRB               ETH_HRB
#define GMAC_HRT               ETH_HRT
#define GMAC_IDR               ETH_IDR
#define GMAC_IER               ETH_IER
#define GMAC_IER_HRESP_Msk     ETH_IER_HRESP_Msk
#define GMAC_IER_RCOMP_Msk     ETH_IER_RCOMP_Msk
#define GMAC_IER_RLEX_Msk      ETH_IER_RLEX_Msk
#define GMAC_IER_ROVR_Msk      ETH_IER_ROVR_Msk
#define GMAC_IER_RXUBR_Msk     ETH_IER_RXUBR_Msk
#define GMAC_IER_TCOMP_Msk     ETH_IER_TCOMP_Msk
#define GMAC_IER_TFC_Msk       ETH_IER_TFC_Msk
#define GMAC_IER_TUR_Msk       ETH_IER_TUR_Msk
#define GMAC_IHCE              ETH_IHCE
#define GMAC_ISR               ETH_ISR
#define GMAC_ISR_RCOMP_Msk     ETH_ISR_RCOMP_Msk
#define GMAC_ISR_TCOMP_Msk     ETH_ISR_TCOMP_Msk
#define GMAC_JR                ETH_JR
#define GMAC_LFFE              ETH_LFFE
#define GMAC_MAN               ETH_MAN
#define GMAC_MAN_CLTTO_Msk     ETH_MAN_CLTTO_Msk
#define GMAC_MAN_DATA          ETH_MAN_DATA
#define GMAC_MAN_DATA_Msk      ETH_MAN_DATA_Msk
#define GMAC_MAN_OP            ETH_MAN_OP
#define GMAC_MAN_PHYA          ETH_MAN_PHYA
#define GMAC_MAN_REGA          ETH_MAN_REGA
#define GMAC_MAN_WTN           ETH_MAN_WTN
#define GMAC_MCF               ETH_MCF
#define GMAC_MFR               ETH_MFR
#define GMAC_MFT               ETH_MFT
#define GMAC_NCFGR             ETH_NCFGR
#define GMAC_NCFGR_CLK_MCK8    ETH_NCFGR_CLK(0x0)
#define GMAC_NCFGR_CLK_MCK16   ETH_NCFGR_CLK(0x1)
#define GMAC_NCFGR_CLK_MCK32   ETH_NCFGR_CLK(0x2)
#define GMAC_NCFGR_CLK_MCK48   ETH_NCFGR_CLK(0x3)
#define GMAC_NCFGR_CLK_MCK64   ETH_NCFGR_CLK(0x4)
#define GMAC_NCFGR_CLK_MCK96   ETH_NCFGR_CLK(0x5)
#define GMAC_NCFGR_FD_Msk      ETH_NCFGR_FD_Msk
#define GMAC_NCFGR_LFERD_Msk   ETH_NCFGR_LFERD_Msk
#define GMAC_NCFGR_MAXFS       ETH_NCFGR_MAXFS_Msk
#define GMAC_NCFGR_MTIHEN_Msk  ETH_NCFGR_MTIHEN_Msk
#define GMAC_NCFGR_RFCS_Msk    ETH_NCFGR_RFCS_Msk
#define GMAC_NCFGR_RXCOEN_Msk  ETH_NCFGR_RXCOEN_Msk
#define GMAC_NCFGR_SPD_Msk     ETH_NCFGR_SPD_Msk
#define GMAC_NCR               ETH_NCR
#define GMAC_NCR_CLRSTAT_Msk   ETH_NCR_CLRSTAT_Msk
#define GMAC_NCR_MPE_Msk       ETH_NCR_MPE_Msk
#define GMAC_NCR_RXEN_Msk      ETH_NCR_RXEN_Msk
#define GMAC_NCR_TSTART_Msk    ETH_NCR_TSTART_Msk
#define GMAC_NCR_TXEN_Msk      ETH_NCR_TXEN_Msk
#define GMAC_NSR               ETH_NSR
#define GMAC_NSR_IDLE_Msk      ETH_NSR_IDLE_Msk
#define GMAC_OFR               ETH_OFR
#define GMAC_RBQB              ETH_RBQB
#define GMAC_RBQB_ADDR_Msk     ETH_RBQB_ADDR_Msk
#define GMAC_ROE               ETH_ROE
#define GMAC_RRE               ETH_RRE
#define GMAC_RSE               ETH_RSE
#define GMAC_RSR               ETH_RSR
#define GMAC_RSR_RESETVALUE    ETH_RSR_RESETVALUE
#define GMAC_SAB               ETH_SAB
#define GMAC_SAB_ADDR          ETH_SAB_ADDR
#define GMAC_SAT               ETH_SAT
#define GMAC_SAT_ADDR          ETH_SAT_ADDR
#define GMAC_SCF               ETH_SCF
#define GMAC_TBQB              ETH_TBQB
#define GMAC_TBQBAPQ           ETH_TBQBAPQ
#define GMAC_TBQB_ADDR_Msk     ETH_TBQB_ADDR_Msk
#define GMAC_TCE               ETH_TCE
#define GMAC_TUR               ETH_TUR
#define GMAC_UCE               ETH_UCE
#define GMAC_UFR               ETH_UFR
#define GMAC_UR                ETH_UR
#endif

#endif
