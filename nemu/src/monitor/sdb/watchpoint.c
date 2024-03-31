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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char *expr;
  uint32_t val;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  if(free_  == NULL){assert(0);}
  else {
    if(head == NULL) {
      head = free_;
      free_ = head->next;
      head->next = NULL;
    }
    else {
      WP *ptr = head;
      head = free_;
      free_ = head->next;
      head->next = ptr;
    }
    return head;
  }
}

void free_wp(WP *wp) {
  WP *ptr = NULL;
  if(head == wp){
    WP *tmp = wp->next;
    wp->next = free_;
    free_ = wp;
    head = tmp;
    return;
  }
  for (ptr = head ; ptr->next != NULL; ptr = ptr->next) {
    if(ptr->next ==  wp){break;}
  } 
  if(ptr->next == NULL) {
    assert(0);
  }
  else {
    WP *tmp = (ptr->next)->next;
    (ptr->next)->next = free_;
    free_ = ptr->next;
    ptr->next = tmp;
  }
  return;
}

void display_wp() {
  WP *ptr = NULL;
  for (ptr = head ; ptr != NULL; ptr = ptr->next) {
    printf("NO:%d VAL:%u\n", ptr->NO, ptr->val);
    printf("EXPR:%s\n",ptr->expr);
  }
  return;
}

void delete_wp(int no) {
  WP *ptr = NULL;
  for (ptr = head ; ptr != NULL; ptr = ptr->next) {
    if(no == ptr->NO)break;
  }
  if(ptr == NULL){ printf("Watch point not found\n");}
  else { free_wp(ptr);}
  return;
}

void set_wp(char *e) {
  WP *ptr = NULL;
  bool success = true;
  ptr = new_wp();
  //printf("%s\n",e);
  ptr->expr = e;
  printf("%s\n", ptr->expr);
  ptr->val = expr(ptr->expr, &success);
  printf("%s\n", ptr->expr);
  return;
}
