/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void display_wp();
void delete_wp(int);
void new_wp();
void set_wp(char*);
word_t vaddr_read(vaddr_t, int);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_si(char *args) {
  /* extract the number */
  char *arg = strtok(NULL, " ");
  int num;

  if(arg == NULL) {
    num = 1;
  }
  else{
    num = atoi(arg);
  }
  cpu_exec(num);
  
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_info(char *args) {
  char *arg = strtok(args, " ");
  if(arg == NULL) {
    printf("Use info r or info w");
  }
  else if(!strcmp(arg, "r")) {
    isa_reg_display(NULL);  
  }
  else if(!strcmp(arg, "w")) {
    display_wp();
  }
  else {}
  return 0;
}

static int cmd_x(char *args) {
  char *num, *nexpr;
  num = strtok(args, " ");
  nexpr = strtok(NULL, " ");
  int nums;
  int exprs;
  if((num == NULL)||(nexpr == NULL) ){
    printf("Missing arguments");
  } 
  else {
    nums = atoi(num);	
    exprs = strtol(nexpr, NULL, 16);
    int i;
    for(i = 0; i < nums; i++) {
      word_t vread = vaddr_read(exprs+4*i, 4);
      printf("%x\n",vread);  
    }
  }
  return 0;
}

static int cmd_p(char *args) {
  bool *success = NULL;
  printf("%u\n",expr(args, success));
  return 0;
}

static int cmd_w(char *args) {
  set_wp(args);
  //printf("%s\n",args);
  return 0;
}

static int cmd_d(char *args) {
  int no = atoi(args);
  delete_wp(no);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute single instruction", cmd_si },
  { "info", "Display the information", cmd_info },
  { "x", "Scan the memory", cmd_x },
  { "p", "Calculate the expr", cmd_p },
  { "w", "Set watchpoint", cmd_w },
  { "d", "Delete watchpoint", cmd_d },

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}


void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {nemu_state.state = NEMU_QUIT;return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
