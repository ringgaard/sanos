//
// pic.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Programmable Interrupt Controller (PIC i8259)
//

#include <os/krnl.h>

// All IRQs disabled initially except cascade

unsigned int irq_mask = 0xFFFB; 

//
// Initialize the 8259 Programmable Interrupt Controller
//

void init_pic()
{
  _outp(PIC_MSTR_CTRL, PIC_MSTR_ICW1);
  _outp(PIC_SLV_CTRL, PIC_SLV_ICW1);
  _outp(PIC_MSTR_MASK, PIC_MSTR_ICW2);
  _outp(PIC_SLV_MASK, PIC_SLV_ICW2);
  _outp(PIC_MSTR_MASK, PIC_MSTR_ICW3);
  _outp(PIC_SLV_MASK, PIC_SLV_ICW3);
  _outp(PIC_MSTR_MASK, PIC_MSTR_ICW4);
  _outp(PIC_SLV_MASK, PIC_SLV_ICW4);
  _outp(PIC_MSTR_MASK, PIC_MSTR_DISABLE);
  _outp(PIC_SLV_MASK, PIC_SLV_DISABLE);
}

//
// Enable IRQ
//

void enable_irq(unsigned int irq)
{
  irq_mask &= ~(1 << irq);
  if (irq >= 8) irq_mask &= ~(1 << 2);
	
  _outp(PIC_MSTR_MASK, irq_mask & 0xFF);
  _outp(PIC_SLV_MASK, (irq_mask >> 8) & 0xFF);
}

//
// Disable IRQ
//

void disable_irq(unsigned int irq)
{
  irq_mask |= (1 << irq);
  if ((irq_mask & 0xFF00) == 0xFF00) irq_mask |= (1 << 2);
	
  _outp(PIC_MSTR_MASK, irq_mask & 0xFF);
  _outp(PIC_SLV_MASK, (irq_mask >> 8) & 0xFF);
}

//
// Signal end of interrupt to PIC

void eoi(unsigned int irq)
{
  if (irq < 8)
    _outp(PIC_MSTR_CTRL, irq + PIC_EOI_BASE);
  else
  {
    _outp(PIC_SLV_CTRL, (irq - 8) + PIC_EOI_BASE);
    _outp(PIC_MSTR_CTRL, PIC_EOI_CAS);
  }
}
