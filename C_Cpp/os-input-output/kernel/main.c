
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

#define TTY_FIRST       (tty_table)
#define TTY_END         (tty_table + NR_CONSOLES)

PRIVATE COLOR color_schemes[] = { MAKE_COLOR(BLACK, RED), 
    MAKE_COLOR(BLUE, WHITE), MAKE_COLOR(WHITE, RED), 
    MAKE_COLOR(BLACK, BLUE), MAKE_COLOR(GREEN, WHITE), 
    MAKE_COLOR(GREEN, BLACK), MAKE_COLOR(BLUE, RED), 
    MAKE_COLOR(WHITE, GREEN), MAKE_COLOR(WHITE, BLUE), 
    MAKE_COLOR(GREEN, RED), MAKE_COLOR(BLACK, GREEN), 
    MAKE_COLOR(WHITE, RED), MAKE_COLOR(BLACK, WHITE)
};

PRIVATE int color_scheme_no = 13;

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int
kernel_main ()
{
  disp_str ("-----\"kernel_main\" begins-----\n");

  TASK *p_task = task_table;
  PROCESS *p_proc = proc_table;
  char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
  u16 selector_ldt = SELECTOR_LDT_FIRST;
  int i;
  for (i = 0; i < NR_TASKS; i++)
    {
      strcpy (p_proc->p_name, p_task->name);	// name of the process
      p_proc->pid = i;		// pid

      p_proc->ldt_sel = selector_ldt;

      memcpy (&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
	      sizeof (DESCRIPTOR));
      p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
      memcpy (&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
	      sizeof (DESCRIPTOR));
      p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
      p_proc->regs.cs = ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
	| SA_TIL | RPL_TASK;
      p_proc->regs.ds = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
	| SA_TIL | RPL_TASK;
      p_proc->regs.es = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
	| SA_TIL | RPL_TASK;
      p_proc->regs.fs = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
	| SA_TIL | RPL_TASK;
      p_proc->regs.ss = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
	| SA_TIL | RPL_TASK;
      p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;

      p_proc->regs.eip = (u32) p_task->initial_eip;
      p_proc->regs.esp = (u32) p_task_stack;
      p_proc->regs.eflags = 0x1202;	/* IF=1, IOPL=1 */

      p_task_stack -= p_task->stacksize;
      p_proc++;
      p_task++;
      selector_ldt += 1 << 3;
    }

  proc_table[0].ticks = proc_table[0].priority = 9;
  proc_table[1].ticks = proc_table[1].priority = 1;

  k_reenter = 0;
  ticks = 0;

  p_proc_ready = proc_table;

  init_clock ();
  init_keyboard ();

  current_color_scheme = DEFAULT_CHAR_COLOR;

  restart ();

  while (1)
    {
    }
}

/*======================================================================*
                               ClearScreen
 *======================================================================*/
void
ClearScreen ()
{
  while (1)
    {
      milli_delay (40000);

      int n;
      for (n = 0; n < color_scheme_no; n++) {
        if (current_color_scheme == color_schemes[n]) {
            current_color_scheme = color_schemes[(n+1) % color_scheme_no];
            break;
        }
      }

      TTY *p_tty;
      for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
	{
	  clear_screen (p_tty->p_console);
          ticks = 0;
	}
    }
}
