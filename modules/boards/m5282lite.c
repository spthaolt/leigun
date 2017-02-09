//===-- boards/m5282lite.c ----------------------------------------*- C -*-===//
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
/// Compose a MCF5282LITE Board 
///
//===----------------------------------------------------------------------===//

/*
 * -----------------------------------------------------------------------------
 *  Compose a MCF5282LITE Board 
 *
 * (C) 2008 Jochen Karrer
 *   Author: Jochen Karrer
 *
 * -----------------------------------------------------------------------------
 */

//==============================================================================
//= Dependencies
//==============================================================================
// Main Module Header

// Local/Private Headers
#include "amdflash.h"
#include "coldfire/cpu_cf.h"
#include "coldfire/mcf5282_csm.h"
#include "coldfire/mcf5282_scm.h"

// Leigun Core Headers
#include "bus.h"
#include "core/device.h"
#include "initializer.h"
#include "sram.h"

// External headers

// System headers


//==============================================================================
//= Constants(also Enumerations)
//==============================================================================
static const char *BOARD_NAME = "M5282LITE";
static const char *BOARD_DESCRIPTION = "M5282LITE";
static const char *BOARD_DEFAULTCONFIG = 
"[global]\n"
"start_address: 0x00000000\n"
"cpu_clock: 66000000\n"
"\n"
"[sram0]\n"
"size: 64k"
"\n"
"[flash0]\n"
"type: AM29LV160BB\n"
"chips: 1\n"
"\n";


//==============================================================================
//= Types
//==============================================================================


//==============================================================================
//= Variables
//==============================================================================


//==============================================================================
//= Function declarations(static)
//==============================================================================
static void create_clock_links(void);
static void create_signal_links(void);
static void create_i2c_devices(void);
static Device_Board_t *create(void);
static int run(Device_Board_t *board);


//==============================================================================
//= Function definitions(static)
//==============================================================================

static void
create_clock_links(void)
{
//      Clock_Link("st.slck","pmc.slck");       
}

static void
create_signal_links(void)
{
}

static void
create_i2c_devices(void)
{
}

static Device_Board_t *
create(void)
{
	BusDevice *dev;
	MCF5282ScmCsm *scmcsm;
	Bus_Init(NULL, 4 * 1024);
	scmcsm = MCF5282_ScmCsmNew("scmcsm");
	dev = AMDFlashBank_New("flash0");
	MCF5282Csm_RegisterDevice(scmcsm, dev, CSM_CS0);
	Device_Board_t *board;
	board = malloc(sizeof(*board));
	board->run = &run;

	//dev = CFM_New("cfm");
#if 0
	if (dev) {
		Mem_AreaAddMapping(dev, 0xffe00000, 2 * 1024 * 1024,
				   MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);
	}
#endif
	dev = SRam_New("sram0");
	if (dev) {
		Mem_AreaAddMapping(dev, 0x04000000, 64 * 1024,
				   MEM_FLAG_WRITABLE | MEM_FLAG_READABLE);
	}
	CF_CpuInit();
	create_i2c_devices();
	create_signal_links();
	create_clock_links();
	return board;
}

static int
run(Device_Board_t *board)
{
	CF_CpuRun();
	return 0;
}


//==============================================================================
//= Function definitions(global)
//==============================================================================
INITIALIZER(init) {
    Device_RegisterBoard(BOARD_NAME, BOARD_DESCRIPTION, &create, BOARD_DEFAULTCONFIG);
}
