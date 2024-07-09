	.stabs	"/Users/LukeIannini/PdCheckout/DesireData/",100,0,2,Ltext0
	.stabs	"src/m_fifo.c",100,0,2,Ltext0
	.text
Ltext0:
	.stabs	"",102,0,0,0
	.stabs	"gcc2_compiled.",60,0,0,0
	.stabs	":t(0,1)=(0,1)",128,0,0,0
_lifo_pop:
	.stabd	46,0,0
	.stabd	68,0,281
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	subl	$20, %esp
	.stabd	68,0,282
	movl	$0, -12(%ebp)
	.stabd	68,0,283
	movl	8(%ebp), %esi
	addl	$4, %esi
	# LFPOP 					
	pushl	%ebx				
	pushl	%ecx				
	movl 	4(%esi), %edx		
	movl  	(%esi), %eax		
	testl	%eax, %eax		
	jz		20f					
10:	movl 	(%eax), %ebx		
	movl	%edx, %ecx		
	incl	%ecx				
	cmpxchg8b (%esi)		
	jz		20f					
	testl	%eax, %eax		
	jnz	10b					
20:	popl	%ecx				
	popl	%ebx				
	
	movl	%eax, -12(%ebp)
	.stabd	68,0,305
	movl	-12(%ebp), %eax
	.stabd	68,0,306
	addl	$20, %esp
	popl	%esi
	popl	%ebp
	ret
	.stabs	"lifo_pop:f(0,2)",36,0,281,_lifo_pop
	.stabs	"lifo:p(0,3)",160,0,280,8
	.stabs	"data:(0,2)",128,0,282,-12
	.stabs	"void:t(0,1)",128,0,0,0
	.stabs	":t(0,2)=*(0,1)",128,0,0,0
	.stabs	":t(0,3)=*(0,4)",128,0,0,0
	.stabs	"t_lifo:t(0,4)=(0,5)",128,0,0,0
	.stabs	"_lifo:T(0,5)=s12ic:(0,6),0,32;top:(0,7),32,32;oc:(0,6),64,32;;",128,0,0,0
	.stabs	"long unsigned int:t(0,6)=r(0,6);0;037777777777;",128,0,0,0
	.stabs	":t(0,7)=*(0,8)",128,0,0,0
	.stabs	"t_fifocell:t(0,8)=(0,9)",128,0,0,0
	.stabs	"_fifocell:T(0,9)=s8next:(0,10),0,32;data:(0,2),32,32;;",128,0,0,0
	.stabs	":t(0,10)=*(0,9)",128,0,0,0
	.stabn	192,0,0,_lifo_pop
	.stabn	224,0,0,Lscope0
Lscope0:
	.stabs	"",36,0,0,Lscope0-_lifo_pop
	.stabd	78,0,0
_lifo_push:
	.stabd	46,0,0
	.stabd	68,0,309
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	subl	$4, %esp
	.stabd	68,0,310
	movl	8(%ebp), %esi
	movl	12(%ebp), %ecx
	# LFPUSH					
	pushl	%ebx				
	pushl	%ecx				
	movl 0(%esi), %eax		
	movl 4(%esi), %edx		
1:	movl %eax, %ebx			
	incl %ebx					
	movl %edx, (%ecx)		
	cmpxchg8b (%esi)		
	jnz	1b					
	popl	%ecx				
	popl	%ebx				
	
	.stabd	68,0,327
	addl	$4, %esp
	popl	%esi
	popl	%ebp
	ret
	.stabs	"lifo_push:f(0,1)",36,0,309,_lifo_push
	.stabs	"lifo:p(0,3)",160,0,308,8
	.stabs	"data:p(0,2)",160,0,308,12
Lscope1:
	.stabs	"",36,0,0,Lscope1-_lifo_push
	.stabd	78,0,0
_lifo_init:
	.stabd	46,0,0
	.stabd	68,0,449
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	.stabd	68,0,450
	movl	8(%ebp), %eax
	movl	$0, (%eax)
	.stabd	68,0,451
	movl	8(%ebp), %eax
	movl	$0, 4(%eax)
	.stabd	68,0,452
	movl	8(%ebp), %eax
	movl	$0, 8(%eax)
	.stabd	68,0,453
	leave
	ret
	.stabs	"lifo_init:f(0,1)",36,0,449,_lifo_init
	.stabs	"lifo:p(0,3)",160,0,448,8
Lscope2:
	.stabs	"",36,0,0,Lscope2-_lifo_init
	.stabd	78,0,0
.globl _fifo_init
_fifo_init:
	.stabd	46,0,0
	.stabd	68,0,456
	pushl	%ebp
	movl	%esp, %ebp
	subl	$40, %esp
	.stabd	68,0,457
	movl	$24, (%esp)
	call	L_getbytes$stub
	movl	%eax, -12(%ebp)
	.stabd	68,0,459
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	_lifo_init
	.stabd	68,0,460
	movl	-12(%ebp), %eax
	addl	$12, %eax
	movl	%eax, (%esp)
	call	_lifo_init
	.stabd	68,0,462
	movl	-12(%ebp), %eax
	.stabd	68,0,463
	leave
	ret
	.stabs	"fifo_init:F(0,11)",36,0,456,_fifo_init
	.stabs	"ret:(0,11)",128,0,457,-12
	.stabs	"_fifo:T(0,12)=s24in:(0,4),0,96;out:(0,4),96,96;;",128,0,0,0
	.stabs	":t(0,11)=*(0,12)",128,0,0,0
	.stabn	192,0,0,_fifo_init
	.stabn	224,0,0,Lscope3
Lscope3:
	.stabs	"",36,0,0,Lscope3-_fifo_init
	.stabd	78,0,0
.globl _fifo_destroy
_fifo_destroy:
	.stabd	46,0,0
	.stabd	68,0,467
	pushl	%ebp
	movl	%esp, %ebp
	subl	$40, %esp
L10:
	.stabd	68,0,471
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	L_fifo_get$stub
	movl	%eax, -12(%ebp)
	.stabd	68,0,473
	cmpl	$0, -12(%ebp)
	jne	L10
	.stabd	68,0,475
	movl	$24, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	L_freebytes$stub
	.stabd	68,0,477
	leave
	ret
	.stabs	"fifo_destroy:F(0,1)",36,0,467,_fifo_destroy
	.stabs	"fifo:p(0,11)",160,0,466,8
	.stabs	"data:(0,2)",128,0,468,-12
	.stabn	192,0,0,_fifo_destroy
	.stabn	224,0,0,Lscope4
Lscope4:
	.stabs	"",36,0,0,Lscope4-_fifo_destroy
	.stabd	78,0,0
.globl _fifo_put
_fifo_put:
	.stabd	46,0,0
	.stabd	68,0,480
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	.stabd	68,0,481
	movl	8(%ebp), %edx
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	_lifo_push
	.stabd	68,0,482
	leave
	ret
	.stabs	"fifo_put:F(0,1)",36,0,480,_fifo_put
	.stabs	"fifo:p(0,11)",160,0,479,8
	.stabs	"data:p(0,2)",160,0,479,12
Lscope5:
	.stabs	"",36,0,0,Lscope5-_fifo_put
	.stabd	78,0,0
.globl _fifo_get
_fifo_get:
	.stabd	46,0,0
	.stabd	68,0,485
	pushl	%ebp
	movl	%esp, %ebp
	subl	$40, %esp
	.stabd	68,0,487
	movl	8(%ebp), %eax
	addl	$12, %eax
	movl	%eax, -20(%ebp)
	.stabd	68,0,489
	movl	-20(%ebp), %eax
	movl	%eax, (%esp)
	call	_lifo_pop
	movl	%eax, -24(%ebp)
	.stabd	68,0,491
	cmpl	$0, -24(%ebp)
	jne	L16
LBB2:
	.stabd	68,0,494
	movl	8(%ebp), %eax
	movl	%eax, -12(%ebp)
	.stabd	68,0,495
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	_lifo_pop
	movl	%eax, -24(%ebp)
	.stabd	68,0,497
	cmpl	$0, -24(%ebp)
	jne	L18
	jmp	L16
L19:
	.stabd	68,0,501
	movl	-24(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	-20(%ebp), %eax
	movl	%eax, (%esp)
	call	_lifo_push
	.stabd	68,0,502
	movl	-16(%ebp), %eax
	movl	%eax, -24(%ebp)
L18:
	.stabd	68,0,499
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	_lifo_pop
	movl	%eax, -16(%ebp)
	cmpl	$0, -16(%ebp)
	jne	L19
L16:
LBE2:
	.stabd	68,0,507
	movl	-24(%ebp), %eax
	.stabd	68,0,508
	leave
	ret
	.stabs	"fifo_get:F(0,2)",36,0,485,_fifo_get
	.stabs	"fifo:p(0,11)",160,0,484,8
	.stabs	"data:(0,2)",128,0,486,-24
	.stabs	"out:(0,3)",128,0,487,-20
	.stabn	192,0,0,_fifo_get
	.stabs	"tmp:(0,2)",128,0,493,-16
	.stabs	"in:(0,3)",128,0,494,-12
	.stabn	192,0,0,LBB2
	.stabn	224,0,0,LBE2
	.stabn	224,0,0,Lscope6
Lscope6:
	.stabs	"",36,0,0,Lscope6-_fifo_get
	.stabd	78,0,0
	.stabs	"",100,0,0,Letext0
Letext0:
	.section __IMPORT,__jump_table,symbol_stubs,self_modifying_code+pure_instructions,5
L_freebytes$stub:
	.indirect_symbol _freebytes
	hlt ; hlt ; hlt ; hlt ; hlt
L_getbytes$stub:
	.indirect_symbol _getbytes
	hlt ; hlt ; hlt ; hlt ; hlt
L_fifo_get$stub:
	.indirect_symbol _fifo_get
	hlt ; hlt ; hlt ; hlt ; hlt
	.subsections_via_symbols
