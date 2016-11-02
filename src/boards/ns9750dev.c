/*
 ************************************************************************************************
 *
 * Compose the NS9750 development board from Netsilicon
 * 
 *  State: working
 *
 * Copyright 2004 2005 Jochen Karrer. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY Jochen Karrer ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Jochen Karrer.
 *
 *************************************************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "signode.h"
#include "mmu_arm9.h"
#include "ns9750_mem.h"
#include "ns9750_pci.h"
#include "ns9750_serial.h"
#include "ns9750_timer.h"
#include "ns9750_bbus.h"
#include "ns9750_bbdma.h"
#include "ns9750_usb.h"
#include "ns9750_eth.h"
#include "fio.h"
#include "bus.h"
#include "lacc_can.h"
#include "amdflash.h"
#include "configfile.h"
#include "phy.h"
#include "lxt971a.h"
#include "loader.h"
#include "boards.h"

/*
 * -------------------------------------------------------------------------------
 * 
 * -------------------------------------------------------------------------------
 */
int
board_ns9750dev_create() 
{
	BusDevice *dev;
	BusDevice *bbus;
	BBusDMACtrl *bbdma;
	ArmCoprocessor *copro;
	NS9750_MemController *memco;
	PHY_Device *phy;
	PCI_Function *bridge;

	ARM9_New();
	copro = MMU9_Create("mmu",TARGET_BYTEORDER,MMU_ARM926EJS | MMUV_NS9750);
	ARM9_RegisterCoprocessor(copro,15);
	Bus_Init(MMU_InvalidateTlb,4*1024);

	bbus=NS9xxx_BBusNew("NS9750","bbus");
	bbdma=NS9750_BBusDMA_New("bbdma");
        NS9750_SerialInit(bbdma);
        NS9750_TimerInit("sysco");
	memco = NS9750_MemCoInit("memco");
	dev=NS9750_EthInit("ns9750_eth");

	phy=Lxt971a_New();
	NS9750_EthRegisterPhy(dev,phy,0);
	NS9750Usb_New("ns9750_usb");

	bridge=NS9750_PciInit("ns9750_pci",PCI_DEVICE(0));

	/* Now Create and Register the devices */
	dev = DRam_New("dram0");
	if(dev) {
		NS9750_RegisterDevice(memco,dev,NS9750_CS4);
	}
	dev = DRam_New("dram1");
	if(dev) {
		NS9750_RegisterDevice(memco,dev,NS9750_CS5);
	}
	dev = DRam_New("dram2");
	if(dev) {
		NS9750_RegisterDevice(memco,dev,NS9750_CS6);
	}
	dev = DRam_New("dram3");
	if(dev) {
		NS9750_RegisterDevice(memco,dev,NS9750_CS7);
	}

	dev=AMDFlashBank_New("flash0");
	if(dev) {
		NS9750_RegisterDevice(memco,dev,NS9750_CS0);
	}
	dev = AMDFlashBank_New("flash1");
	if(dev) {
		NS9750_RegisterDevice(memco,dev,NS9750_CS1);
	} else {
		fprintf(stderr,"Warning ! no boot Flash available !\n");
		sleep(2);
	}

	dev=LaccCAN_New();
	NS9750_RegisterDevice(memco,dev,NS9750_CS3);

        SigName_Link("serialA.RxDmaGnt","bbdma.0.FbwDmaGnt");
        SigName_Link("serialA.TxDmaReq","bbdma.1.FbrDmaReq");

        SigName_Link("serialB.RxDmaGnt","bbdma.2.FbwDmaGnt");
        SigName_Link("serialB.TxDmaReq","bbdma.3.FbrDmaReq");

        SigName_Link("serialC.RxDmaGnt","bbdma.4.FbwDmaGnt");
        SigName_Link("serialC.TxDmaReq","bbdma.5.FbrDmaReq");

        SigName_Link("serialD.RxDmaGnt","bbdma.6.FbwDmaGnt");
        SigName_Link("serialD.TxDmaReq","bbdma.7.FbrDmaReq");

        SigName_Link("serialA.tx_irq","bbus.irq_satx");
        SigName_Link("serialA.rx_irq","bbus.irq_sarx");
        SigName_Link("serialB.tx_irq","bbus.irq_sbtx");
        SigName_Link("serialB.rx_irq","bbus.irq_sbrx");
        SigName_Link("serialC.tx_irq","bbus.irq_sctx");
        SigName_Link("serialC.rx_irq","bbus.irq_scrx");
        SigName_Link("serialD.tx_irq","bbus.irq_sdtx");
        SigName_Link("serialD.rx_irq","bbus.irq_sdrx");
	SigName_Link("ns9750_eth.rx_irq","ns9750sysco.irq4");
	SigName_Link("ns9750_eth.tx_irq","ns9750sysco.irq5");

        /* Endian */
        SigName_Link("bbutil.endian_serA","serialA.endian");
        SigName_Link("bbutil.endian_serB","serialB.endian");
        SigName_Link("bbutil.endian_serC","serialC.endian");
        SigName_Link("bbutil.endian_serD","serialD.endian");
	SigName_Link("bbutil.endian_usb","ns9750_ohci.endian");
	SigName_Link("mmu.endian","ns9750_eth.dataendian");
	SigName_Link("mmu.endian","ns9750_pci.cpu_endian");
	SigName_Link("flash1.big_endian","memco.big_endian");

	return 0;
}

static void
board_ns9750dev_run(Board *bd) {
        ARM9_Run();
}

#define DEFAULTCONFIG \
"[loader]\n" \
"load_address: 0x50000000\n" \
"\n" \
"[dram0]\n" \
"size: 16M\n" \
"\n" \
"[flash1]\n"\
"type: M29W320DB\n"\
"chips: 2\n"\
"\n"

static Board board_ns9750dev = {
	name:	"NS9750DEV",
	description:	"Netsilicon NS9750 development Board",
	createBoard:	board_ns9750dev_create,
	runBoard:	board_ns9750dev_run,
	defaultconfig: DEFAULTCONFIG 
};

__CONSTRUCTOR__ static void
ns9750dev_init() {
        fprintf(stderr,"Loading NS9750 development board module\n");
        Board_Register(&board_ns9750dev);
}

