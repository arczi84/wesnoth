#include <stdlib.h>
#include <inttypes.h>

#include <proto/exec.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include "VBLServer.h"

static struct Interrupt *VblNode;
static volatile int32_t frameSig = 0;

void ApolloInitVBLServer()
{
	VblNode = (struct Interrupt *)AllocVec(sizeof(struct Interrupt), MEMF_PUBLIC | MEMF_CLEAR);
	VblNode->is_Node.ln_Type = NT_INTERRUPT;
	VblNode->is_Node.ln_Pri = -60;
	VblNode->is_Node.ln_Name = "ApolloVBLServer";
	VblNode->is_Data = (APTR)&frameSig;
	VblNode->is_Code = ApolloVerticalBlankServer;

	AddIntServer(INTB_VERTB, VblNode); // This syncs to the native monitor

}

void ApolloShutdownVBLServer()
{
	if(VblNode)
	{
		RemIntServer(INTB_VERTB, VblNode);
		FreeVec(VblNode);
	}
}


void ApolloWaitVBLPassed()
{
	while(!frameSig)
		;
	frameSig = 0;
}
