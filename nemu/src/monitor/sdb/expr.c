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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"\\(", '('},  // parentheses
  {"\\)", ')'},
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"\\-", '-'},		// minus
  {"\\*", '*'},		// multi
  {"\\/", '/'}, 	// divise
  {"[0-9]+", TK_NUM},	// numbers			 
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

int findop(int p, int q) {
  int i,op=0;
  int islock = 0;
  for(i = p; i <= q; i++) {
    if(tokens[i].type == TK_NUM){continue;}
    if(tokens[i].type == '('){islock = 1;continue;}
    if(tokens[i].type == ')'){islock = 0;continue;}
    if(!islock) {
    	if(tokens[i].type == '+'||tokens[i].type == '-'){return i;}
	else {
	  if(!islock) { op = i; }
	}
    }
  }
  return op;
}

static bool check_parentheses(int p, int q) {
  if(tokens[p].type!='('||tokens[q].type!=')'){ return false; }
  
  char par[32];
  int i,nr_p = 0;
  for(i = p; i<=q; i++){
    if(tokens[i].type=='('||tokens[i].type==')') {
      par[nr_p] = tokens[i].type;
      nr_p ++;
    }
  } 

  if(p+1 > q-1)return false;

  int stack = 0;
  for(i = 1; i < nr_p-1; i++) {
    if(par[i]=='('){ stack++;}
    else{stack--;}
    if(stack < 0){return false;}
  }

  if(stack){ return false;}
  return true;
}

uint32_t eval(int p, int q){
  if(p > q) {
    return -1; 
  }
  else if(p == q) {
    if(tokens[p].type != TK_NUM)return -1;
    else return ((uint32_t)atoi(tokens[p].str));
  }
  else if(check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  }
  else {
    int op = findop(p, q);
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);
    
    printf("%d%c%d\n",val1,tokens[op].type,val2);
    
    switch(tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': 
		assert(val2);
		return val1 / val2;
      default: assert(0);
    }
  }
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
	 * to record the token in the array `tokens'. For certain types
	 * of tokens, some extra actions should be performed.
         */
	
        switch (rules[i].token_type) {
           case TK_NOTYPE: break;
	   case TK_NUM: 
		int j;
		assert(substr_len < 32);
	        for(j = 0; j < substr_len; j++){ tokens[nr_token].str[j] = substr_start[j];}	
	   default: 
		tokens[nr_token].type = rules[i].token_type;
		nr_token++;
	}

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  return eval(0, nr_token-1);
}
