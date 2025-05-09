##==============================================================================
##
##  crt0.S
##
##  startup code
##
##==============================================================================

#include <nds32_init.inc>
#include "interrupt.h"

##------------------------------------------------------------------------------
	.macro vector name
	.align 2
	j OS_Trap_\name
	.endm

	.macro ENABLE_IT_DT
	.align 2
		pushm   $r0, $r1
		mfsr    $r1, $IPSW
		andi	$r1, $r1, #0x20c0 !ʹ��ZOL��Cahce
		mfsr    $r0, $PSW
		add   	$r0, $r0, $r1
		mtsr    $r0, $PSW
		isb
		popm    $r0, $r1
	.endm

	.macro EXCEPTION_HANDLER NUM
	.align 2
		addi $sp,$sp,#0x800
		pushm $lp,$sp
		pushm $r0,$r20
		mov55 $r0, $sp
		li	$r1, \NUM
		b   ExceptionCommHandler
	.endm

	.section .vector, "ax"
!========================================================================
! Vector table
!========================================================================
	.align 2
exception_vector:
_start:
	j ___start                    !  (0) Trap Reset
	vector TLB_Fill             !  (1) Trap TLB fill
	vector PTE_Not_Present      !  (2) Trap PTE not present
	vector TLB_Misc             !  (3) Trap TLB misc
	vector TLB_VLPT_Miss        !  (4) Trap TLB VLPT miss
	vector Machine_Error        !  (5) Trap Machine error
	vector Debug_Related        !  (6) Trap Debug related
	vector General_Exception    !  (7) Trap General exception
	vector Syscall              !  (8) Syscall

	/* HW interrupt vector layout */
	hal_hw_vectors
exception_vector_end:

.section .startup_section, "ax"
	.align  2
ISR_TABLE:
	.long GpioInterrupt    //0
	.long WakeupInterrupt  //1
	.long PowerkeyInterrupt//2
	.long UsbInterrupt     //3
	.long FFTInterrupt     //4
	.long SWInterrupt      //5
	.long SystickInterrupt //6
	.long Timer2Interrupt  //7
	.long Timer3Interrupt  //8
	.long Timer4Interrupt  //9
	.long Timer5Interrupt  //10
	.long Timer6Interrupt  //11
	.long RtcInterrupt	   //12
	.long HosccntInterrupt //13
	.long rwip_isr	   //14
	.long rwbt_isr     //15
	.long BLE_Interrupt//rwble_isr    //16
	.long SPIM_Interrupt   //17
	.long SPIS_Interrupt   //18
	.long UART0_Interrupt  //19
	.long UART1_Interrupt  //20
	.long SDIO0_Interrupt  //21
	.long SPDIF0_Interrupt //22//or CAN
	.long IR_Interrupt	   //23
	.long I2sInterrupt     //24
	.long SPDIF1_Interrupt //25
	.long Timer7Interrupt  //26
	.long Timer8Interrupt  //27
	.long I2C_Interrupt    //28
	.long DMA_D_Interrupt  //29//stream 0-5 done interrupt
	.long DMA_T_Interrupt  //30//stream 0-5 tint interrupt
	.long DMA_E_Interrupt  //31//stream 0-5 error interrupt

/*
 * exception handlers
 */

/*----------------------------------------------------------------------
   Since N903 doesn't have MMU, make them share the common error handler.
----------------------------------------------------------------------*/
OS_Trap_TLB_Fill:
//1:  b   1b
	EXCEPTION_HANDLER 0x1

OS_Trap_PTE_Not_Present:
//1:  b   1b
	EXCEPTION_HANDLER 0x2

OS_Trap_TLB_Misc:
//1:  b   1b
	EXCEPTION_HANDLER 0x3


OS_Trap_TLB_VLPT_Miss:
//1:  b   1b
	EXCEPTION_HANDLER 0x4

OS_Trap_Machine_Error:
//1:  b   1b
	EXCEPTION_HANDLER 0x5

OS_Trap_Debug_Related:
//1:  b   1b
	EXCEPTION_HANDLER 0x6

OS_Trap_General_Exception:
//1:  b   1b
	EXCEPTION_HANDLER 0x7

OS_Trap_Syscall:
//1:  b   1b
	EXCEPTION_HANDLER 0x8

OS_Trap_Int_Comm:
	ENABLE_IT_DT
	SAVE_ALL_HW
	la  $r1, ISR_TABLE
	lw  $r1, [$r1+$r0<<2]
	jral $r1
	RESTORE_ALL_HW
	iret

! Interrupt wrapper function
OS_Trap_Interrupt_HW0:
	push	$r0
	li	$r0, 0x0
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW1:
	push	$r0
	li	$r0, 0x01
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW2:
	push	$r0
	li	$r0, 0x02
	j	OS_Trap_Int_Comm

/*
OS_Trap_Interrupt_HW3:
	pushm	$r0, $r1
	ENABLE_IT_DT
	j	OS_Trap_Interrupt_SWI
*/
OS_Trap_Interrupt_HW3:
	push	$r0
	li	$r0, 0x03
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW4:
	push	$r0
	li	$r0, 0x04
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW5:
	pushm	$r0, $r1
	ENABLE_IT_DT
	j	OS_Trap_Interrupt_SWI
	;push	$r0
	;li	$r0, 0x05
	;j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW6:
	push	$r0
	li	$r0, 0x06
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW7:
	push	$r0
	li	$r0, 0x07
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW8:
	push	$r0
	li	$r0, 0x08
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW9:
	push	$r0
	li	$r0, 0x09
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW10:
	push	$r0
	li	$r0, 0x0a
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW11:
	push	$r0
	li	$r0, 0x0b
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW12:
	push	$r0
	li	$r0, 0x0c
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW13:
	push	$r0
	li	$r0, 0x0d
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW14:
	push	$r0
	li	$r0, 0x0e
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW15:
	push	$r0
	li	$r0, 0x0f
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW16:
	push	$r0
	li	$r0, 0x10
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW17:
	push	$r0
	li	$r0, 0x11
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW18:
	push	$r0
	li	$r0, 0x12
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW19:
	push	$r0
	li	$r0, 0x13
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW20:
	push	$r0
	li	$r0, 0x14
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW21:
	push	$r0
	li	$r0, 0x15
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW22:
	push	$r0
	li	$r0, 0x16
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW23:
	push	$r0
	li	$r0, 0x17
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW24:
	push	$r0
	li	$r0, 0x18
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW25:
	push	$r0
	li	$r0, 0x19
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW26:
	push	$r0
	li	$r0, 0x1a
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW27:
	push	$r0
	li	$r0, 0x1b
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW28:
	push	$r0
	li	$r0, 0x1c
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW29:
	push	$r0
	li	$r0, 0x1d
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW30:
	push	$r0
	li	$r0, 0x1e
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW31:
	push	$r0
	li	$r0, 0x1f
	j	OS_Trap_Int_Comm

##------------------------------------------------------------------------------
## Startup code

	.section .startup_section
	.global _start
	.weak _call_exit
	.weak _SDA_BASE_
	.func _start
	.type _start, @function

	.align 2
___start:
	/*li $r0,0
	li $r1,0
	li $r2,0
	li $r3,0
	li $r4,0
	li $r5,0
	li $r6,0
	li $r7,0
	li $r8,0
	li $r9,0
	li $r10,0*/
	!************************** Begin of do-not-modify **************************
	! Please don't modify this code
	! Initialize the registers used by the compiler

	nds32_init		! NDS32 startup initial macro in <nds32_init.inc>
	!*************************** End of do-not-modify ***************************
	#bal _nds32_init_mem

	movi55 $r0,#0x0
 	mtsr $r0,$misc_ctl

	bal __init

	bal main
1:  b   1b

	.size _start, .-___start

	.end
