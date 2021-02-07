#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>


typedef enum ttype {
  UNDEF, LIT, RPAR, LAST_TERM, LPAR, IS_OP, MINUS, ADD, DIVIDE, MULTIPLY, IS_UNRY,
  SQR, REMAIN} ttype;

typedef struct token {
  double value;
  ttype type;
} token;

typedef struct linked_list {
  token t;                    
  bool is_full;
  struct linked_list * next;
} linked_list;

const int precedence[] = {0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 0, 3, 2};
// work with linked_list
linked_list * push_stack(linked_list * tokens_head, token t) {
  if (!tokens_head->is_full) { 
    tokens_head->next = NULL;
  } else {
    linked_list * new_next = malloc(sizeof(linked_list));
    new_next->next = tokens_head->next;
    new_next->t = tokens_head->t;
    new_next->is_full = tokens_head->is_full;

    tokens_head->next = new_next;
  }
  tokens_head->t = t;
  tokens_head->is_full = true;
  return tokens_head;
}

linked_list * queue(linked_list * tokens_head, token t) {
  if (!tokens_head->is_full) {
      tokens_head->t = t;
      tokens_head->is_full = true;
      tokens_head->next = NULL;
      return tokens_head;
  }
  linked_list * curr = tokens_head;
  while (true) {
    if (curr->next == NULL) {
      curr->next = malloc(sizeof(linked_list));
      curr->next->is_full = true;
      curr->next->next = NULL;
      curr->next->t = t;
      break;
    } else {
      curr = curr->next;
    }
  }
  return tokens_head;
}
token * head_pop(linked_list * tokens_head) {
  if (tokens_head->is_full) {
    token * t = malloc(sizeof(token));
    *t = tokens_head->t;
    if (tokens_head->next != NULL) {
      linked_list * temp = tokens_head->next;
      tokens_head->is_full = tokens_head->next->is_full;
      tokens_head->t = tokens_head->next->t;
      tokens_head->next = tokens_head->next->next;
      free(temp); 
    } else {
      tokens_head->is_full = false;
    }
    return t;
  } else {
    return NULL;
  }
}

token add_token(linked_list * tokens_head, token t) {
  queue(tokens_head, t);
  return t;
}
linked_list * new_linked_list(void) {
  linked_list * l = malloc(sizeof(linked_list));
  l->is_full = false;
  l->next = NULL;
  l->t.type = UNDEF;
  l->t.value = 0;
  return l;
}
void free_linked_list(linked_list * thelist) {
  linked_list * next = thelist;
  linked_list * temp = NULL;
  while (next != NULL) {
    temp = next;
    next = next->next;
    free(temp);
  }
}
char * cln(const char * s) {
  char * out;
  while(isspace(*s)) { 
    ++s;
  }
  if(*s == 0) { 
    out = malloc(sizeof(char));
    *out = '\0';
    return out;
  }
  const char * end = s + strlen(s) - 1;
  while(isspace(*end)) { 
    --end;
  }
  size_t len = (end + 2) - s;
  out = malloc(len * sizeof(char));
  memcpy(out, s, len);
  out[len-1] = 0;
  return out;
}

// main functions for processing expressions

double * evaluate_postfix(const linked_list * const post_tokens) {

  ttype type = UNDEF;
  token temp = {LIT, 0};
  token * left = NULL;
  token * right = NULL;
  double temp_answer = 0;
  linked_list * ans_stack = new_linked_list();
  const linked_list * curr= post_tokens;

  while (true) {
    type = curr->t.type;
    if (type >= IS_OP) {
      right = head_pop(ans_stack);
    //   pritnf("%lf", right->value);
      if (right == NULL) {
        puts("missing number!");
        free_linked_list(ans_stack);
        return NULL;
      }
      if (type < IS_UNRY || type == REMAIN) {
        // puts("hereeeeeeeeeee");
        left = head_pop(ans_stack);
        if (left == NULL) {
          puts("missing number");
          free(right); free_linked_list(ans_stack);
          return NULL;
        }
      }

      switch (type) {
        case ADD:
          temp_answer = left->value + right->value;
          break;
        case MINUS:
          temp_answer = left->value - right->value;
          break;
        case MULTIPLY:
          temp_answer = left->value * right->value;
          break;
        case REMAIN:
          if (right->value == 0) {
            puts("divide by zero");
            free_linked_list(ans_stack);
            free(right); free(left);
            return NULL;
          }
        //   printf("%lf", left->value);
        //   puts("kkkkkkkkkkkkkk");
          temp_answer = (double)(fmod(left->value,right->value));
          break;
        case DIVIDE:
          if (right->value == 0) {
            puts("divide by zero");
            free_linked_list(ans_stack);
            free(right); free(left);
            return NULL;
          }
          temp_answer = left->value / right->value;
          break;
        case SQR:
          temp_answer = pow(right->value, 2);
          break;
        default: 
          puts("invalid type");
          free_linked_list(ans_stack);
          free(right); free(left);
          return NULL;
      }

      free(left); free(right);
      left = NULL; right = NULL;

      temp.value = temp_answer;
      push_stack(ans_stack, temp);

    } else {
      push_stack(ans_stack, curr->t);
    }

    if (curr->next == NULL) {
      break; 
    } else {
      curr = curr->next;
    }
  }

  token * final = head_pop(ans_stack);

  if (final == NULL || ans_stack->next != NULL || ans_stack->is_full) {
    puts("many numbers");
    free_linked_list(ans_stack);
    free(final); 
    return NULL;
  }

  double * answer = malloc(sizeof(double));;
  *answer = final->value;
  free(final);
  free_linked_list(ans_stack);
  return answer;
}
linked_list * tokenize(char exp[]) {

  linked_list * tokens_head = new_linked_list();
  bool first = true;
  token previous = {UNDEF, 0};
  token temp = {UNDEF, 0};
  size_t i = 0;
  size_t len = strlen(exp);
  bool car_number = false;

  while (i < len) {
    temp.value = 0;
    switch (exp[i]) {
      case ' ':
        break; 
      case '+':
        if (!first && (previous.type < LAST_TERM || previous.type > IS_UNRY)) {
          temp.type = ADD;
          previous = add_token(tokens_head, temp);
        } else {
          car_number = true;
        }
        break;
      case '-':
        if (!first && (previous.type < LAST_TERM || previous.type > IS_UNRY)) {
          temp.type = MINUS;
          previous = add_token(tokens_head, temp);
        } else {
          car_number = true;
        }
        break;
      case '*':
        temp.type = MULTIPLY;
        previous = add_token(tokens_head, temp);
        break;
      case '/':
        temp.type = DIVIDE;
        previous = add_token(tokens_head, temp);
        break;
      case '%':
        temp.type = REMAIN;
        previous = add_token(tokens_head, temp);
        break;
      case '^':
        temp.type = SQR;
        previous = add_token(tokens_head, temp);
        break;
      case '(':
        temp.type = LPAR;
        previous = add_token(tokens_head, temp);
        break;
      case ')':
        temp.type = RPAR;
        previous = add_token(tokens_head, temp);
        break;
      default:
        if (isalpha(exp[i])) {
            puts("unrecognized variable");
            free_linked_list(tokens_head);
            return NULL;
        } else if (isdigit(exp[i]) || (exp[i] == '.' && isdigit(exp[i+1]))) {
          car_number = true; 
        } else {
          puts("unrecognized character");
          free_linked_list(tokens_head);
          return NULL;
        }
        break;
    }
    if (car_number) { 
      car_number = false;
      double value = 0;
      int l = 0;
      if (sscanf(&exp[i], "%lf%n", &value, &l) != 1) {
        puts("invalid number");
        free_linked_list(tokens_head);
        return NULL;
      }
      temp.type = LIT;
      temp.value = value;
      previous = add_token(tokens_head, temp);
      i = i + (l-1); 
    }
    ++i;
    first = false;
  }
  return tokens_head;
}
linked_list * convert_postfix(const linked_list * const token_list) {

  linked_list * op_stack = new_linked_list();
  linked_list * out_queue = new_linked_list();
  const linked_list * curr = token_list;
  token * temptoken = NULL;
  ttype last_type = UNDEF;
  bool first = true;

  while (true) {
    if (curr->t.type >= IS_OP) {
      if (curr->t.type >= IS_UNRY &&
          ((last_type > LAST_TERM && last_type < IS_UNRY) || first)) {
        puts("missing number");
        free_linked_list(out_queue); free_linked_list(op_stack);
        return NULL;
      }
      while (op_stack->is_full) {
        if (precedence[curr->t.type]<= precedence[op_stack->t.type]) {
          temptoken = head_pop(op_stack);
          queue(out_queue, *temptoken);
          free(temptoken);
        } else {
          break;
        }
      }
      last_type = curr->t.type;
      push_stack(op_stack, curr->t);

    } else if (curr->t.type == LPAR) {
      push_stack(op_stack, curr->t);
      last_type = LPAR;

    } else if (curr->t.type == RPAR) {
      while (true) {
        temptoken = head_pop(op_stack);
        if (temptoken == NULL) {
          puts("missing parentheses");
          return NULL;
        } else if (temptoken->type == LPAR) {
          free(temptoken);
          if (last_type == LPAR) {
            puts("parentheses!");
            return NULL;
          }
          break;
        } else {
          queue(out_queue, *temptoken);
          free(temptoken);
          last_type = IS_OP;
        }
      }
      last_type = RPAR;
    } else {
      if (last_type <= LAST_TERM && !first) {
        puts("missing operator");
        free_linked_list(out_queue); free_linked_list(op_stack);
        return NULL;
      }
      queue(out_queue, curr->t);
      last_type = LIT;
    }
    if (curr->next == NULL) {
      token * t = NULL;
      t = head_pop(op_stack);
      while (t != NULL) {
        // check for mismatched parentheses
        if ((*t).type == LPAR || (*t).type == RPAR) {
          puts("missing parentheses");
          free(t);
          free_linked_list(op_stack);
          free_linked_list(out_queue);
          return NULL;
        }
        queue(out_queue, *t);
        free(t);
        t = head_pop(op_stack);
      }
      break;
    } else {
      curr = curr->next;
      if (first) {
        first = false;
      }
    }
  }
  free_linked_list(op_stack);
  return out_queue;
}

bool expression_pro(char expression[], double * const answer) {

  linked_list * tokens_head = tokenize(expression);
  if (tokens_head == NULL) {
    return false;
  }
  linked_list * post_tokens = convert_postfix(tokens_head);
  free_linked_list(tokens_head);
  if (post_tokens == NULL) {
    return false;
  }
  double * a = evaluate_postfix(post_tokens);
  free_linked_list(post_tokens);
  if (a == NULL) {
    return false;
  }
  *answer = *a;
  free(a);
  return true;
}

int main(void) {
  puts("please provide a mathematical expression.\n");
  char line[2048];
  bool flag = true;
  char * command = NULL;
  double answer = 0;
  while (true) {
    if (!fgets(line, 2048, stdin)) {
      puts("");
      break;
    }
    command = cln(line); 
    if (*command == '\0') {
    } else {
      flag = expression_pro(command, &answer);
      if (flag) {
        printf("answer= %.*f\n", 6, answer);
      }
    }
    free(command);
  }
  return 0;
}
