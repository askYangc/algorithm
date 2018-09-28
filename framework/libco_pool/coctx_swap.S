/*
* Tencent is pleased to support the open source community by making Libco available.

* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

.globl coctx_swap
#if !defined( __APPLE__ ) && !defined( __FreeBSD__ )
.type  coctx_swap, @function
#endif
coctx_swap:

#if defined(__i386__)
	leal 4(%esp), %edx //sp  存储父函数的栈顶指针(刚好参数1的位置)，在这个实例里面，无需存储ecx和eax
	movl 4(%esp), %eax  //取出参数1到eax

	movl %edx, 28(%eax)     //栈顶指针存储
	movl %ebp, 24(%eax)
	movl %esi, 20(%eax)
	movl %edi, 16(%eax)
	movl %ebx, 4(%eax)
	movl 0(%esp), %edx // ret func addr 当前esp是指向的返回地址
	movl %edx, 0(%eax)    
  
  	movl 8(%esp), %eax // param2
	movl 4(%eax), %ebx
	movl 16(%eax), %edi
	movl 20(%eax), %esi
	movl 24(%eax), %ebp
	movl 28(%eax), %esp
    movl 0(%eax), %eax
	jmp *(%eax)         //跳转到*eax存储的值的地址


#elif defined(__x86_64__)
	leaq 8(%rsp),%rax                       /* 参考函数调用原理，当前%rsp是存的返回地址，+8就是父函数的真正的栈顶指针，存入%rax */
	movq %rax, 104(%rdi)                    /* %rdi是当前运行的协程的ctx地址，将各个寄存器的数据存入ctx的reg中,rax存入到reg[13], r15就存到了reg[0]*/
	movq %rbx, 96(%rdi)
	movq %rcx, 88(%rdi)                     /* 函数参数都依次存储在%rdi,%rsi,%rdx,%rcx,%r8,%r9，因此在本调用中后面4个寄存器其实可以不用存 */
	movq %rdx, 80(%rdi)
    
    movq -8(%rax), %rdx
	movq %rdx, 72(%rdi) //ret func addr /* rax当前指向了父函数的真正栈顶指针，因此-8就是子函数完成后返回地址，实际上应该就是co_swap的coctx_swap处 */

	movq %rsi, 64(%rdi)
	movq %rdi, 56(%rdi)
	movq %rbp, 48(%rdi)                     /* %rbp是帧指针，用于返回父函数的，这里好像不一样，反正保存就是 */
	movq %r8,  40(%rdi)
	movq %r9,  32(%rdi)
	movq %r12, 24(%rdi)
	movq %r13, 16(%rdi)
	movq %r14, 8(%rdi)
	movq %r15, 0(%rdi)
	
	movq 0(%rsi),   %r15                         /* 将即将还原的协程的寄存器还原 */
	movq 8(%rsi),   %r14
	movq 16(%rsi),  %r13
	movq 24(%rsi),  %r12
	movq 32(%rsi),  %r9
	movq 40(%rsi),  %r8
	movq 48(%rsi),  %rbp
	movq 56(%rsi),  %rdi
	movq 72(%rsi),  %rax //ret func addr               /* reg[9]存的返回地址，赋值给了rax */
	movq 80(%rsi),  %rdx
	movq 88(%rsi),  %rcx
	movq 96(%rsi),  %rbx
    movq 104(%rsi), %rsp                    /* reg[13]存的是之前保存的rax，即即将运行的协程的父函数的栈顶指针 */
    movq 64(%rsi),  %rsi
    
    jmp *%rax                               //跳转到eax存储的地址
	#pushq %rax                              /* 返回地址入栈 */
	
	#xorl %eax, %eax                         /* 让rax的低32bit清空 */
	#ret                                     /* 切换完成，返回地址出栈并被调用 */   
#endif
