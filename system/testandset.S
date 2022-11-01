/* testandset.S - test and set (for x86) */

		.text
		.globl	test_and_set

/*------------------------------------------------------------------------
 * test_and_set -  X86 context switch; the call is test_and_set(uint32 *ptr, uint32 new_value)
 *------------------------------------------------------------------------
 */
test_and_set:
		pushl	%ebp		        /* Push ebp onto stack		                */
		movl	%esp,%ebp	        /* Record current SP in ebp	                */
		pushfl			            /* Push flags onto the stack	            */
        pushl   %edi                /* Push used reg onto stack (just edi)      */


		movl	8(%ebp),%edi	    /* Get mem location of ptr	                */
		movl	12(%ebp),%eax	    /* Get new value                            */
        xchg    (%edi),%eax         /* atomic exchange                          */

		
		pop     %edi                /* Restore edi                              */
        movl	4(%esp),%ebp	    /* Pick up ebp before restoring	interrupts  */
		popfl			            /* Restore interrupt mask	                */
		add	$4,%esp		            /* Skip saved value of ebp	                */
		ret			                /* Return to new process	                */
