/*
 ***********************************************************************************************
 *
 * Emulation of Lattice ispMach4000 JTAG Test access port
 *
 * Copyright 2009 2012 Jochen Karrer. All rights reserved.
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
#include "ispMach4000_jtag.h"
#include "jtag_tap.h"
#include "sgstring.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "signode.h"
#include "diskimage.h"
#include "configfile.h"
#include "sglib.h"

#if 0
#define dbgprintf(x...) { fprintf(stderr,x); }
#else
#define dbgprintf(x...)
#endif

#define I_BYPASS	(0xff)
#define I_SAMPLE	(0x1C)
#define I_PRELOAD	(0x1C)
#define I_EXTEST	(0x00)
#define	I_IDCODE	(0x16)
#define I_USERCODE	(0x17)
#define I_HIGHZ		(0x18)
#define	I_CLAMP		(0x20)

#define	ISC_ENABLE		(0x15)
#define	ISC_DISABLE		(0x1E)
#define ISC_NOOP		(0x30)
#define ISC_ADDRESS_SHIFT	(0x01)
#define ISC_DATA_SHIFT		(0x02)
#define	ISC_ERASE		(0x03)
#define ISC_DISCHARGE		(0x14)
#define	ISC_PROGRAM		(0x27)
#define	ISC_READ		(0x2A)
#define ISC_PROGRAM_SECURITY	(0x09)
#define	ISC_PROGRAM_DONE	(0x2F)
#define	ISC_ERASE_DONE		(0x24)
#define ISC_PROGRAM_USERCODE	(0x1A)
#define ISC_ADDRESS_INIT	(0x21)
#define	I_PRIVATE		(0x2C)

typedef struct PLD_Jtag {
	uint8_t ir[1];
	uint8_t dr[256];
	char *tapname;
	SigTrace *enTrace;
	SigNode *nSigJtagEn;
	SigNode *sigTck;
	SigNode *sigTms;
	SigNode *sigTdi;
	SigNode *sigTdo;
	uint32_t read_addr;
	uint32_t write_addr;
	int progr_incr;
	int read_incr;
	int isc_enable;
	uint8_t *data;
	int datasize;
	uint8_t *usercode;
	DiskImage *diskimage;
	uint32_t idcode;
	uint32_t dshift_len;
	uint32_t ashift_len;
	uint32_t boundary_len;
} PLD_Jtag;

typedef struct  Variant {
	char *name;
	uint32_t idcode;
	uint32_t idcode_mask;
	uint32_t dshift_len; /** For Shift data instructions (PGM, READ ) */
	uint32_t ashift_len; /** For shift in address instruction */
	uint32_t boundary_len;
	uint32_t datasize;
} Variant;

/* 
 ************************************************************************
 * List of supported Chips with the technical data of the chips.
 * Taken from BSDL file 
 ************************************************************************
 */
static Variant variants[] = {
	{
		.name = "LC4128V_XXT100",
		.idcode = 0x01811043,
		.idcode_mask = 0x0fffffff,
		.dshift_len = 740,
		.ashift_len = 100,
		.datasize = 9300,
		.boundary_len = 196,
	},
	{
		.name = "LC4256V_XXT100",
		.idcode = 0x11815043,
		.idcode_mask = 0x1fffffff,
		.dshift_len = 1592,
		.ashift_len = 95,
		.datasize = 18905,
		.boundary_len = 260,
	},
}; 
#define FIX_BITORDER8(x)	Bitreverse8(x)

static void
store_data(PLD_Jtag *pj,unsigned int bytes) {
	unsigned int i;
	if(pj->write_addr + bytes > pj->datasize) {
		fprintf(stderr,"PLD: to much data\n");
		return;
	}
	for(i = 0; i < bytes; i++) {
		pj->data[pj->write_addr + i] &= pj->dr[i];
		if(pj->data[pj->write_addr + i] != pj->dr[i]) {
			fprintf(stderr,"PLD: Writing to area which was not erased before\n");
		}
	}
	pj->write_addr += bytes;
}

static void
read_data(PLD_Jtag *pj,unsigned int bytes) {
	if(pj->read_addr + bytes >=  pj->datasize) {
		fprintf(stderr,"PLD: to much data\n");
		return;
	}
	memcpy(pj->dr,pj->data + pj->read_addr,bytes);
	pj->read_addr += bytes;
}

/**
 ***************************************************************************************
 * \fn static void ispMach_CaptureDR(void *owner,uint8_t **data,int *len) 
 *
 * Interface proc to the TAP statemachine.
 * Called by the TAP statemachine whenever the state CaptureDR is entered.
 ***************************************************************************************
 */

static void 
ispMach_CaptureDR(void *owner,uint8_t **data,int *len) 
{
	PLD_Jtag *pj = owner;
	uint8_t *buf = *data = pj->dr;
	dbgprintf("Capture DR with IR %02x\n",FIX_BITORDER8(pj->ir[0]));
	switch(FIX_BITORDER8(pj->ir[0])) {
		/* Address Shift instruction */
		case ISC_ADDRESS_SHIFT:
			*len = pj->ashift_len;
			break;
		/* IDCODE instruction */
		case I_IDCODE:
			*len = 32;
			#if  0
			buf[3] = (pj->idcode >> 24) & 0xff;
			buf[2] = (pj->idcode >> 16) & 0xff;
			buf[1] = (pj->idcode >> 8) & 0xff;
			buf[0] = (pj->idcode >> 0) & 0xff;
			#else
			buf[0] = FIX_BITORDER8((pj->idcode >> 24) & 0xff);
			buf[1] = FIX_BITORDER8((pj->idcode >> 16) & 0xff);
			buf[2] = FIX_BITORDER8((pj->idcode >> 8) & 0xff);
			buf[3] = FIX_BITORDER8((pj->idcode >> 0) & 0xff);
			#endif
			break;

		/* Program Usercode */
		case ISC_PROGRAM_USERCODE:
			*len = 32;
			break;

		/* Read usercode */
		case I_USERCODE:
			buf[0] = pj->usercode[0];
			buf[1] = pj->usercode[1];
			buf[2] = pj->usercode[2];
			buf[3] = pj->usercode[3];
			*len = 32;
			break;

		/* Boundary SCAN / Preload instruction */
		case I_SAMPLE:
			*len = pj->boundary_len;
			memset(pj->dr,0,sizeof(pj->dr));
			break;

		/* ISC Program */
		case ISC_PROGRAM:
			*len = pj->dshift_len;
			if(pj->read_incr) {
				read_data(pj,(pj->dshift_len + 7) / 8);
				pj->read_incr = 0;
			}
			break;

		case ISC_READ:
			*len = pj->dshift_len;
			pj->read_incr = 0;
			if(!pj->progr_incr) {
				read_data(pj,(pj->dshift_len + 7) / 8);
			}
			break;

		/* Bypass instruction */
		case I_BYPASS:
			*len = 1;
			break;
		default:
			fprintf(stderr,"Capture DR with unknown IR 0x%02x \n",FIX_BITORDER8(pj->ir[0]));
	}
}

/**
 ***********************************************************************
 * \fn void ispMach_CaptureIR(void *owner,uint8_t **data,int *len) 
 * CaptureIR. Real device has an IR readback of 0x1D tested with 
 * the IDCODE and USERCODE commands
 ***********************************************************************
 */
static void 
ispMach_CaptureIR(void *owner,uint8_t **data,int *len) 
{
	PLD_Jtag *pj = owner;
	pj->ir[0] = FIX_BITORDER8(0x1D);
	*len = 8;
	*data = pj->ir; 
}

/**
 *****************************************************************************
 * \fn static void ispMach_UpdateIR(void *owner)
 * Interface proc to the TAP statemachine. Called whenever the
 * Update IR state is entered.
 *****************************************************************************
 */
static void 
ispMach_UpdateIR(void *owner)
{
	PLD_Jtag *pj = owner;
	dbgprintf("Update IR: 0x%02x\n",FIX_BITORDER8(pj->ir[0]));
	switch(FIX_BITORDER8(pj->ir[0])) {
		/* Erase instruction */
		case ISC_ERASE:
			if(!pj->isc_enable) {
				fprintf(stderr,"Erase instruction not in ISP mode\n");
				break;
			}
			/* Experiment shows that usercode is also erased */
			memset(pj->data,0xff,pj->datasize);
			pj->usercode[0] = pj->usercode[1] = 
			pj->usercode[2] = pj->usercode[3] = 0xff;
			break;

		/* ISC Enable */
		case ISC_ENABLE:
			pj->isc_enable = 1;
			pj->read_incr = pj->progr_incr = 0;
			break;

		/* ISC Disable */
		case ISC_DISABLE:
			pj->isc_enable = 0;
			break;

		/* Init address instruction */
		case ISC_ADDRESS_INIT:
			pj->read_addr = pj->write_addr = 0;
			break;

		case ISC_PROGRAM:
			pj->progr_incr = 1;
			break;

		case ISC_READ:
			pj->read_incr = 1;
			break;

		/* Program done instruction */
		case ISC_PROGRAM_DONE:
			break;

	}
}

/**
 ********************************************************************
 * Find first nonzero bit. Returns "len" if nothing is found
 ********************************************************************
 */
static unsigned int
find_first_nonzero(uint8_t *data,unsigned int len) 
{
	unsigned int i;
	unsigned int by;
	unsigned int bi;
	for(i = 0; i < len; i++) {
		by = i >> 3;
		bi = i & 7;
		if(data[by] & (1 << bi)) {
			return i;
		}
	}
	return i;
}
/**
 ***************************************************************************************************
 * \fn static void ispMach_UpdateDR(void *owner)
 * Interface proc to the TAP statemachine. Called whenever the UpdateDR state is entered.
 ***************************************************************************************************
 */
static void 
ispMach_UpdateDR(void *owner)
{
	PLD_Jtag *pj = owner;
	unsigned int line;	
	switch(FIX_BITORDER8(pj->ir[0])) {
		case ISC_ADDRESS_SHIFT:
			line = find_first_nonzero(pj->dr,pj->ashift_len);
			if(line >= pj->ashift_len) {
				fprintf(stderr,"ispMach4000: Bad address encoding\n");
				break;
			} 
			pj->write_addr = pj->read_addr = line * ((pj->dshift_len + 7) >> 3);
			fprintf(stderr,"Shiftin address not tested: line %u, addr %u\n",line,pj->read_addr);
			break;

		case ISC_READ:
			if(!pj->isc_enable) {
				fprintf(stderr,"Not in ISP mode\n");
				return;
			}
			/* For Turbo mode */
			if(pj->progr_incr) {
				store_data(pj,(pj->dshift_len + 7) / 8);
				pj->progr_incr = 0;
			}
			break;

		case ISC_PROGRAM:
			if(!pj->isc_enable) {
				fprintf(stderr,"Not in ISP mode\n");
				return;
			}
			store_data(pj,(pj->dshift_len + 7) / 8);
			pj->progr_incr = 0;
			break;

		case ISC_PROGRAM_USERCODE:
			if(!pj->isc_enable) {
				fprintf(stderr,"Not in ISP mode\n");
				return;
			}
			pj->usercode[0] &= pj->dr[0];
			pj->usercode[1] &= pj->dr[1];
			pj->usercode[2] &= pj->dr[2];
			pj->usercode[3] &= pj->dr[3];
			//fprintf(stderr,"Usercode is %02x%02x%02x%02x\n",
			//	pj->dr[0],pj->dr[1],pj->dr[2],pj->dr[3]);
			break;

		case I_PRELOAD:
			#if 0
			fprintf(stderr,"PRELOAD \n");
			int i;
			for(i = 0; i < 196; i++) {
				int bit = 195-i;
				int bval = pj->dr[bit >> 3] >> ((bit & 7)) & 1;
				if(bval) {
					fprintf(stderr,"PRELOAD: %u: %u",i,bval);
				}
			}
			#endif
			break;
			
		default:
			//fprintf(stderr,"IR: %02x\n",FIX_BITORDER8(pj->ir[0]));
			break;
	}
}

/**
 ********************************************************
 * \fn void FS_PldJtagNew(const char *name) 
 * Enable/disable the JTAG port using the
 * 4066 analog switch. 
 ********************************************************
 */

static void
ispEnChange(SigNode *node,int value,void *clientData)
{
	PLD_Jtag *pj = clientData;
	SigNode *tms,*tck,*tdi,*tdo;
	dbgprintf("Isp En Change to %d\n",value);
	tms = SigNode_Find("%s.tms",pj->tapname);
	tck = SigNode_Find("%s.tck",pj->tapname);
	tdi = SigNode_Find("%s.tdi",pj->tapname);
	tdo = SigNode_Find("%s.tdo",pj->tapname);
	if(!tms || !tck || !tdi || !tdo) {
		fprintf(stderr,"But, missing JTAG-Tap signal line\n");
		exit(1);
	}
	if(value == SIG_LOW) {
		//fprintf(stderr,"Closed 4066 analog switch\n");
		SigNode_Link(pj->sigTms,tms);
		SigNode_Link(pj->sigTck,tck);
		SigNode_Link(pj->sigTdi,tdi);
		SigNode_Link(pj->sigTdo,tdo);
	} else {
		//fprintf(stderr,"Opened 4066 analog switch\n");
		SigNode_RemoveLink(pj->sigTms,tms);
		SigNode_RemoveLink(pj->sigTck,tck);
		SigNode_RemoveLink(pj->sigTdi,tdi);
		SigNode_RemoveLink(pj->sigTdo,tdo);
	}
}
       
static JTAG_Operations jtagOps = {
	.captureDR = ispMach_CaptureDR,
	.captureIR = ispMach_CaptureIR,
	.updateDR = ispMach_UpdateDR,
	.updateIR = ispMach_UpdateIR,
	.bitorder = JTAG_TAP_ORDER_LSBFIRST,
};

/**
 ********************************************************
 * \fn void FS_PldJtagNew(const char *name) 
 * Create a new Jtag port with TAP.
 ********************************************************
 */
void
IspMach4000_JtagNew(const char *name) 
{
	int i;
	char *tapname = alloca(strlen(name) + 50);
	char *dirname;
	char *variantname;
	Variant *variant;
	int imgsize;
	PLD_Jtag *pj = sg_new(PLD_Jtag);
	sprintf(tapname,"%s.tap",name);
	pj->tapname = sg_strdup(tapname);
	JTagTap_New(tapname,&jtagOps,pj);
	pj->nSigJtagEn = SigNode_New("%s.nJtagEn",name);
	pj->sigTck = SigNode_New("%s.tck",name);
	pj->sigTms = SigNode_New("%s.tms",name);
	pj->sigTdi = SigNode_New("%s.tdi",name);
	pj->sigTdo = SigNode_New("%s.tdo",name);
	if(!pj->nSigJtagEn || !pj->sigTck || !pj->sigTms || !pj->sigTdi || !pj->sigTdo) {
		fprintf(stderr,"Can not create signal lines for ispMach4000-JTag\n");
		exit(1);
	}
	SigNode_Set(pj->nSigJtagEn,SIG_PULLUP);
	pj->enTrace = SigNode_Trace(pj->nSigJtagEn,ispEnChange,pj); 
	variantname  = Config_ReadVar(name,"variant");
	if(variantname) {
		for(i = 0; i < array_size(variants); i++) {
			variant = &variants[i];
			if(strcmp(variant->name,variantname) == 0) {
				break;
			} 		
		}
		if(i >= array_size(variants)) {
			fprintf(stderr,"%s: Variant %s not found, Available:\n",name,variantname);
			for(i = 0; i < array_size(variants); i++) {
				fprintf(stderr,"\t%s\n",variants[i].name);
			}
			exit(1);
		}
	} else {
		variant = &variants[0];	
	}
	pj->dshift_len = variant->dshift_len;
	pj->ashift_len = variant->ashift_len;
	pj->boundary_len = variant->boundary_len;
	pj->idcode = variant->idcode;
	pj->datasize = variant->datasize;
	imgsize = pj->datasize + 4;
	dirname = Config_ReadVar("global","imagedir");
        if(dirname) {
		char *imagename = alloca(strlen(dirname) + strlen(name) + 20);
		sprintf(imagename,"%s/%s.img",dirname,name);
		pj->diskimage = DiskImage_Open(imagename,imgsize,DI_RDWR | DI_CREAT_FF);
		if(!pj->diskimage) {
			fprintf(stderr,"Failed to open diskimage \"%s\"\n",imagename);
			exit(1);
		}
		pj->data = DiskImage_Mmap(pj->diskimage);
        } else {
		pj->data = sg_calloc(imgsize);
		memset(pj->data,0xff,imgsize);
        }
	pj->usercode = pj->data + imgsize - 4;
	fprintf(stderr,"MACH4000 PLD, variant \"%s\"\n",variant->name);
}
