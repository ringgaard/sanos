#include <os.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

double eval_expr(char **expr);

void skip_space(char **expr) {
  while (**expr == ' ') (*expr)++;
}

void next(char **expr) {
  if (**expr) (*expr)++;
  while (**expr == ' ') (*expr)++;
}

double error(char **expr, char *msg) {
  printf("%s: %s\n", msg, *expr);
  while (**expr) (*expr)++;
  return 0.0;
}

double eval_factor(char **expr) {
  double result;

  if (**expr == '(') {
    next(expr);
    result = eval_expr(expr);
    if (**expr != ')') return error(expr, "')' missing");
    next(expr);
  } else if (**expr == '+') {
    next(expr);
    result = eval_factor(expr);
  } else if (**expr == '-') {
    next(expr);
    result = -eval_factor(expr);
  } else if (**expr == '.' || isdigit(**expr)) {
    char *newpos;
    result = strtod(*expr, &newpos);
    if (!newpos || *expr == newpos) return error(expr, "error in number");
    *expr = newpos;
    skip_space(expr);
  } else if (isalpha(**expr)) {
    double arg1 = 0.0;
    double arg2 = 0.0;
    int n;
    char *func = *expr;
    while (isalnum(**expr)) (*expr)++;
    n = *expr - func;

    if (strncmp(func, "inf", n) == 0) return INFINITY;
    if (strncmp(func, "nan", n) == 0) return NAN;

    skip_space(expr);
    if (**expr != '(') return error(expr, "'(' missing");
    next(expr);
    arg1 = eval_expr(expr);
    if (**expr == ',') {
      next(expr);
      arg2 = eval_expr(expr);
    }
    if (**expr != ')') return error(expr, "')' missing");
    next(expr);

    if (strncmp(func, "asin", n) == 0) {
      result = asin(arg1);
    } else if (strncmp(func, "acos", n) == 0) {
      result = acos(arg1);
    } else if (strncmp(func, "atan", n) == 0) {
      result = atan(arg1);
    } else if (strncmp(func, "atan2", n) == 0) {
      result = atan2(arg1, arg2);
    } else if (strncmp(func, "ceil", n) == 0) {
      result = ceil(arg1);
    } else if (strncmp(func, "cos", n) == 0) {
      result = cos(arg1);
    } else if (strncmp(func, "cosh", n) == 0) {
      result = cosh(arg1);
    } else if (strncmp(func, "exp", n) == 0) {
      result = exp(arg1);
    } else if (strncmp(func, "fabs", n) == 0) {
      result = fabs(arg1);
    } else if (strncmp(func, "floor", n) == 0) {
      result = floor(arg1);
    } else if (strncmp(func, "fmod", n) == 0) {
      result = fmod(arg1, arg2);
    } else if (strncmp(func, "mant", n) == 0) {
      int expo;
      result = frexp(arg1, &expo);
    } else if (strncmp(func, "expo", n) == 0) {
      int expo;
      frexp(arg1, &expo);
      result = expo;
    } else if (strncmp(func, "ldexp", n) == 0) {
      result = fmod(arg1, (int) arg2);
    } else if (strncmp(func, "log", n) == 0) {
      result = log(arg1);
    } else if (strncmp(func, "log10", n) == 0) {
      result = log10(arg1);
    } else if (strncmp(func, "frac", n) == 0) {
      double intg;
      result = modf(arg1, &intg);
    } else if (strncmp(func, "int", n) == 0) {
      modf(arg1, &result);
    } else if (strncmp(func, "pow", n) == 0) {
      result = pow(arg1, arg2);
    } else if (strncmp(func, "sin", n) == 0) {
      result = sin(arg1);
    } else if (strncmp(func, "sinh", n) == 0) {
      result = sinh(arg1);
    } else if (strncmp(func, "sqrt", n) == 0) {
      result = sqrt(arg1);
    } else if (strncmp(func, "tan", n) == 0) {
      result = tan(arg1);
    } else if (strncmp(func, "tanh", n) == 0) {
      result = tanh(arg1);
    } else {
      *expr = func;
      return error(expr, "unknown function");
    }
  } else {
    return error(expr, "syntax error");
  }

  return result;
}

double eval_term(char **expr) {
  double result;

  result = eval_factor(expr);
  while (**expr == '*' || **expr == '/') {
    if (**expr == '*') {
      next(expr);
      result *= eval_factor(expr);
    } else {
      next(expr);
      result /= eval_factor(expr);
    }
  }

  return result;
}

double eval_expr(char **expr) {
  double result;

  result = eval_term(expr);
  while (**expr == '+' || **expr == '-') {
    if (**expr == '+') {
      next(expr);
      result += eval_term(expr);
    } else {
      next(expr);
      result -= eval_term(expr);
    }
  }

  return result;
}

double eval(char **expr) {
  skip_space(expr);
  return eval_expr(expr);
}

int main(int argc, char *argv[]) {
  int i;
  double result;
  char *expr;
  for (i = 1; i < argc; i++) {
    expr = argv[i];
    result = eval(&expr);
    if (*expr) {
      printf("syntax error\n");
    } else {
      printf("%g\n", result);
    }
  }

  return 0;
}
