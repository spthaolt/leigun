//===-- boards/imx1ads.c ------------------------------------------*- C -*-===//
//
//              The Leigun Embedded System Simulator Platform : modules
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//===----------------------------------------------------------------------===//
///
/// @file
/// Compose a Freescale iMX1ADS Board
///
//===----------------------------------------------------------------------===//

// clang-format off
/*
 *************************************************************************************************
 *  Compose a Freescale iMX1ADS Board
 *
 *  State: Nothing is working and probably never will be completed
 *	   because I have no real board and no CPU
 *
 * Copyright 2006 Jochen Karrer. All rights reserved.
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
// clang-format on

//==============================================================================
//= Dependencies
//==============================================================================
// Main Module Header

// Local/Private Headers
#include "aitc.h"
#include "amdflash.h"
#include "arm/mmu_arm9.h"
#include "controllers/imx21/imx21_uart.h"
#include "imx_timer.h"

// Leigun Core Headers
#include "bus.h"
#include "device.h"
#include "dram.h"
#include "signode.h"

// External headers

// System headers


//==============================================================================
//= Constants(also Enumerations)
//==============================================================================
#define BOARD_NAME "iMX1ADS"
#define BOARD_DESCRIPTION "iMX1ADS"
#define BOARD_DEFAULTCONFIG                                                    \
    "[global]\n"                                                               \
    "start_address: 0x0\n"                                                     \
    "\n"                                                                       \
    "[dram0]\n"                                                                \
    "size: 32M\n"                                                              \
    "\n"                                                                       \
    "[loader]\n"                                                               \
    "load_address: 0x10000000\n"                                               \
    "\n"                                                                       \
    "[dram0]\n"                                                                \
    "size: 32M\n"                                                              \
    "\n"                                                                       \
    "[flash0]\n"                                                               \
    "type: AM29BDS128H\n"                                                      \
    "chips: 2\n"


//==============================================================================
//= Types
//==============================================================================
typedef struct board_s {
    Device_Board_t base;
    Device_MPU_t *mpu;
} board_t;


//==============================================================================
//= Variables
//==============================================================================


//==============================================================================
//= Function declarations(static)
//==============================================================================
static void create_signal_links(void);
static Device_Board_t *create(void);


//==============================================================================
//= Function definitions(static)
//==============================================================================

static void create_signal_links(void) {
    /* Connect the interrupt controller to the CPU */
    SigName_Link("arm.irq", "aitc.irq");
    SigName_Link("arm.fiq", "aitc.fiq");

    // SigName_Link("aitc.nIntSrc0","uart3_mint_pferr");
    // SigName_Link("aitc.nIntSrc1","uart3_mint_rts");
    // SigName_Link("aitc.nIntSrc2","uart3_mint_dtr");
    // SigName_Link("aitc.nIntSrc3","uart3_mint_uartc");
    // SigName_Link("aitc.nIntSrc4","uart3_mint_tx");
    // SigName_Link("aitc.nIntSrc5","pen_up_int");
    // SigName_Link("aitc.nIntSrc6","csi_int");
    // SigName_Link("aitc.nIntSrc7","mma_mac_int");
    // SigName_Link("aitc.nIntSrc8","mma_int");
    // SigName_Link("aitc.nIntSrc9","comp_int");
    // SigName_Link("aitc.nIntSrc10","msirq");
    // SigName_Link("aitc.nIntSrc11","gpio_int_porta");
    // SigName_Link("aitc.nIntSrc12","gpio_int_portb");
    // SigName_Link("aitc.nIntSrc13","gpio_int_portc");
    // SigName_Link("aitc.nIntSrc14","lcdc_int");
    // SigName_Link("aitc.nIntSrc15","sim_irq");
    // SigName_Link("aitc.nIntSrc16","sim_data");
    // SigName_Link("aitc.nIntSrc17","rtc_int");
    // SigName_Link("aitc.nIntSrc18","rtc_sam_int");
    // SigName_Link("aitc.nIntSrc19","uart2_mint_pferr");
    // SigName_Link("aitc.nIntSrc20","uart2_mint_rts");
    // SigName_Link("aitc.nIntSrc21","uart2_mint_dtr");
    // SigName_Link("aitc.nIntSrc22","uart2_mint_uartc");
    // SigName_Link("aitc.nIntSrc23","uart2_mint_tx");
    // SigName_Link("aitc.nIntSrc24","uart2_mint_rx");

    // SigName_Link("aitc.nIntSrc25","uart1_mint_pferr");
    // SigName_Link("aitc.nIntSrc26","uart1_mint_rts");
    // SigName_Link("aitc.nIntSrc27","uart1_mint_dtr");
    // SigName_Link("aitc.nIntSrc28","uart1_mint_uartc");
    // SigName_Link("aitc.nIntSrc29","uart1_mint_tx");
    // SigName_Link("aitc.nIntSrc30","uart1_mint_rx");

    // SigName_Link("aitc.nIntSrc33","pen_data_int");
    // SigName_Link("aitc.nIntSrc34","pwm_int");
    // SigName_Link("aitc.nIntSrc35","mmc_irq");
    // SigName_Link("aitc.nIntSrc36","ssi_tx2_int");
    // SigName_Link("aitc.nIntSrc37","ssi_rx2_int");
    // SigName_Link("aitc.nIntSrc38","ssi_err_int");
    // SigName_Link("aitc.nIntSrc39","i2c_int");
    // SigName_Link("aitc.nIntSrc40","spi2_int");
    // SigName_Link("aitc.nIntSrc41","spi1_int");
    // SigName_Link("aitc.nIntSrc42","ssi_tx_int");
    // SigName_Link("aitc.nIntSrc43","ssi_tx_err_int");
    // SigName_Link("aitc.nIntSrc44","ssi_rx_int");
    // SigName_Link("aitc.nIntSrc45","ssi_rx_err_int");
    // SigName_Link("aitc.nIntSrc46","touch_int");
    // SigName_Link("aitc.nIntSrc47","usbd_int0");
    // SigName_Link("aitc.nIntSrc48","usbd_int1");
    // SigName_Link("aitc.nIntSrc49","usbd_int2");
    // SigName_Link("aitc.nIntSrc50","usbd_int3");
    // SigName_Link("aitc.nIntSrc51","usbd_int4");
    // SigName_Link("aitc.nIntSrc52","usbd_int5");
    // SigName_Link("aitc.nIntSrc53","usbd_int6");
    // SigName_Link("aitc.nIntSrc54","uart_mint_rx");
    // SigName_Link("aitc.nIntSrc55","btsys");
    // SigName_Link("aitc.nIntSrc56","bttim");
    // SigName_Link("aitc.nIntSrc57","btwui");
    SigName_Link("aitc.nIntSrc58", "timer2.irq");
    SigName_Link("aitc.nIntSrc59", "timer1.irq");
    // SigName_Link("aitc.nIntSrc60","dma_err");
    // SigName_Link("aitc.nIntSrc61","dma_int");
    // SigName_Link("aitc.nIntSrc62","gpio_int_portd");
    // SigName_Link("aitc.nIntSrc63","wdt_int");
}

static Device_Board_t *create(void) {
    ArmCoprocessor *copro;
    BusDevice *dev;
    board_t *board = calloc(1, sizeof(*board));
    board->base.base.self = board;

    Bus_Init(MMU_InvalidateTlb, 4 * 1024);
    board->mpu = Device_CreateMPU("ARM9");
    copro = MMU9_Create("mmu", BYTE_ORDER_LITTLE, MMU_ARM920T);
    ARM9_RegisterCoprocessor(copro, 15);

    /* Currently I have no dram controller */
    dev = DRam_New("dram0");
    if (dev) {
        Mem_AreaAddMapping(dev, 0x08000000, 0x04000000,
                           MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);
    }
    dev = AMDFlashBank_New("flash0");
    if (dev) {
        Mem_AreaAddMapping(dev, 0x10000000, 0x04000000,
                           MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);
        Mem_AreaAddMapping(dev, 0x00000000, 0x00100000,
                           MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);
    }

    dev = Aitc_New("aitc");
    Mem_AreaAddMapping(dev, 0x00223000, 0x00001000,
                       MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);

    dev = IMXUart_New("uart1");
    Mem_AreaAddMapping(dev, 0x00206000, 0x00001000,
                       MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);
    dev = IMXUart_New("uart2");
    Mem_AreaAddMapping(dev, 0x00207000, 0x00001000,
                       MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);
    dev = IMXUart_New("uart3");
    Mem_AreaAddMapping(dev, 0x0020a000, 0x00001000,
                       MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);

    dev = IMXTimer_New("timer1");
    Mem_AreaAddMapping(dev, 0x00202000, 0x00001000,
                       MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);
    dev = IMXTimer_New("timer2");
    Mem_AreaAddMapping(dev, 0x00203000, 0x00001000,
                       MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);

    create_signal_links();
    return &board->base;
}


//==============================================================================
//= Function definitions(global)
//==============================================================================
DEVICE_REGISTER_BOARD(BOARD_NAME, BOARD_DESCRIPTION, &create,
                      BOARD_DEFAULTCONFIG);
