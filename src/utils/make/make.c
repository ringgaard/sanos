//
// make.c
//
// Make utility
//
// Copyright (C) 2011 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

struct buffer {
  char *start;
  char *end;
  char *limit;
};

struct item {
  char *value;
  struct item *next;
};

struct list {
  struct item *head;
  struct item *tail;
};

#define UNCHECKED   0
#define PENDING     1
#define CLEAN       2
#define DIRTY       3

struct rule {
  char *target;
  struct list dependencies;
  int status;
  time_t timestamp;
  struct list commands;
  struct rule *next;
  struct rule *build_next;
};

struct project {
  struct rule *rules_head;  
  struct rule *rules_tail;

  struct list targets;
  int dry_run;
  int silent;
  int debug;
  int always_build;
  char oldcwd[FILENAME_MAX];

  struct rule *build_first;
  struct rule *build_last;
  struct rule *phony;
  
  struct buffer line;
  struct buffer name;
  struct buffer value;
};

char *strndup(char *str, int size) {
  char *buffer = (char *) malloc(size + 1);
  memcpy(buffer, str, size);
  buffer[size] = 0;
  return buffer;
}

void buffer_init(struct buffer *b) {
  b->start = b->end = b->limit = NULL;
}

void buffer_free(struct buffer *b) {
  if (b->start) free(b->start);
}

void buffer_clear(struct buffer *b) {
  b->end = b->start;
}

void buffer_expand(struct buffer *b, int minfree) {
  char *p;
  int size;
  int minsize;

  if (b->limit - b->end >= minfree) return;
  
  size = b->limit - b->start;
  minsize = b->end - b->start + minfree;
  while (size < minsize) {
    if (size == 0) {
      size = 128;
    } else {
      size *= 2;
    }
  }

  p = (char *) realloc(b->start, size);
  b->end = p + (b->end - b->start);
  b->limit = p + size;
  b->start = p;
}

void buffer_append(struct buffer *b, char *data, int size) {
  buffer_expand(b, size);
  memcpy(b->end, data, size);
  b->end += size;
}

void buffer_add(struct buffer *b, char ch) {
  buffer_expand(b, 1);
  *b->end++ = ch;
}

void buffer_concat(struct buffer *b, char *str) {
  buffer_append(b, str, strlen(str));
}

void buffer_set(struct buffer *b, char *data, int size) {
  buffer_clear(b);
  buffer_append(b, data, size);
}

char *buffer_dup(struct buffer *b) {
  return strndup(b->start, b->end - b->start);
}

void list_init(struct list *l) {
  l->head = l->tail = NULL;
}

void list_append(struct list *l, char *value) {
  struct item *itm = (struct item *) malloc(sizeof(struct item));
  itm->value = value;
  itm->next = NULL;
  if (l->tail) l->tail->next = itm;
  if (!l->head) l->head = itm;
  l->tail = itm;
}

void list_free(struct list *l)  {
  struct item *next;
  struct item *i = l->head;
  while (i != NULL) {
    if (i->value) free(i->value);
    next = i->next;
    free(i);
    i = next;
  }
}

void project_init(struct project *prj) {
  memset(prj, 0, sizeof(struct project));
  getcwd(prj->oldcwd, FILENAME_MAX);
}

void project_free(struct project *prj) {
  struct rule *r;
  
  list_free(&prj->targets);

  r = prj->rules_head;
  while (r != NULL) {
    struct rule *next;
    if (r->target) free(r->target);
    list_free(&r->dependencies);
    list_free(&r->commands);
    next = r->next;
    free(r);
    r = next;
  }

  buffer_free(&prj->line);
  buffer_free(&prj->name);
  buffer_free(&prj->value);
  chdir(prj->oldcwd);
}

struct rule *add_rule(struct project *prj, char *target) {
  struct rule *rule = (struct rule *) malloc(sizeof(struct rule));
  rule->target = target;
  rule->build_next = NULL;
  rule->status = UNCHECKED;
  rule->timestamp = -1;
  list_init(&rule->dependencies);
  list_init(&rule->commands);
        
  rule->next = NULL;
  if (prj->rules_tail) prj->rules_tail->next = rule;
  if (!prj->rules_head) prj->rules_head = rule;
  prj->rules_tail = rule;
  return rule;
}

struct rule *find_rule(struct project *prj, char *name) {
  struct rule *r = prj->rules_head;
  while (r) {
    if (strcmp(r->target, name) == 0) return r;
    r = r->next;
  }
  return NULL;
}

void setup_predefined_variables() {
  setenv("AR", "ar", 0);
  setenv("AS", "as", 0);
  setenv("CC", "cc", 0);
  setenv("MAKE", "make", 0);
}

void replace_pattern(char *p, char *pattern, char *replace, struct buffer *b) {
  int pattern_len = strlen(pattern);
  while (*p) {
    if (strncmp(p, pattern, pattern_len) == 0) {
      buffer_concat(b, replace);
      p += pattern_len;
    } else {
      buffer_add(b, *p++);
    }
  }
}

int expand_macros(char *p, struct project *prj, struct rule *rule, struct buffer *b) {
  buffer_clear(b);
  while (*p) {
    if (*p == '$') {
      // Parse macro
      char *start = ++p;
      char *end = NULL;
      if (*p == '@') {
        // Expand to target name
        if (!rule) {
          fprintf(stderr, "Special macro $@ only allowed in commands\n");
          return -1;
        }
        buffer_concat(b, rule->target);
        p++;
      } else if (*p == '<') {
        // Expand to first name in dependencies
        if (!rule) {
          fprintf(stderr, "Special macro $< only allowed in commands\n");
          return -1;
        }
        if (rule->dependencies.head != NULL) {
          buffer_concat(b, rule->dependencies.head->value);
        }
        p++;
      } else if (*p == '^' || *p == '*' && *(p + 1) == '*') {
        struct item *item;
        int first;

        // Expand to list of dependencies
        if (!rule) {
          fprintf(stderr, "Special macro $^ only allowed in commands\n");
          return -1;
        }
        item = rule->dependencies.head;
        first = 1;
        while (item) {
          if (!first) buffer_add(b, ' ');
          buffer_concat(b, item->value);
          item = item->next;
          first = 0;
        }
        if (*p == '*') p++;
        p++;
        
      } else if (*p == '?') {
        // Expand to list of dependencies more recent than target
        if (!rule) {
          fprintf(stderr, "Special macro $? only allowed in commands\n");
          return -1;
        }
        buffer_concat(b, "#NOTIMPL#");
        p++;
      } else if (*p == '(') {
        start = ++p;
        while (*p && *p != ')') p++;
        if (*p != ')') return -1;
        end = p++;
      } else {
        while (isalnum(*p) || *p == '_') *p++;
        end = p;
      }

      if (end != NULL) {
        // Substitute variable.
        char *pattern = NULL;
        char *replace = NULL;
        char *name = strndup(start, end - start);
        char *value;

        // Check for $(VAR:pattern=replace)
        char *colon = strchr(name, ':');
        if (colon) {
          char *equal = strchr(colon, '=');
          if (equal) {
            *colon = 0;
            pattern = colon + 1;
            *equal = 0;
            replace = equal + 1;
          }
        }

        // Expand variable
        value = getenv(name);
        if (value != NULL) {
          if (pattern != NULL) {
            replace_pattern(value, pattern, replace, b);
          } else {
            buffer_concat(b, value);
          }
        }
        free(name);
      }
    } else {
      // Just add character.
      buffer_add(b, *p++);
    }
  }

  buffer_add(b, 0);
  return 0;
}

void parse_list(char *p, struct list *list) {
  char *start;
  
  while (*p) {
    // Skip whitespace
    if (isspace(*p)) {
      p++;
      continue;
    }
    
    // Parse name
    start = p;
    while (*p && !isspace(*p)) p++;
    list_append(list, strndup(start, p - start));
  }
}

int parse_makefile(struct project *prj, FILE *f) {
  int line_num = 1;
  struct rule *current_rule = NULL;
  struct buffer *line = &prj->line;
  struct buffer *name = &prj->name;
  struct buffer *value = &prj->value;

  while (!feof(f)) {
    int ch;
    char *p;
    int skipws;
    
    buffer_clear(line);
    skipws = 0;
    while ((ch = getc(f)) != EOF)  {
      if (ch == '\r') continue;
      if (ch == '\n') {
        // Increment line counter.
        line_num++;
        
        // Check for line continuation.
        if (line->end > line->start && *(line->end - 1) == '\\') {
          line->end--;
          skipws = 1;
          continue;
        } else {
          break;
        }
      }
      if (skipws) {
        if (isspace(ch)) {
          skipws = 2;
          continue;
        } else {
          if (skipws == 2) buffer_add(line, ' ');
          skipws = 0;
        }
      }
      buffer_add(line, ch);
    }
    buffer_add(line, 0);
    p = line->start;
    
    // Skip empty lines.
    if (!*p) continue;
    
    // Skip comments.
    if (*p == '#') continue;
    
    // Skip directives for now.
    if (*p == '!') continue;
    
    // If the line starts with whitespace it must be a command.
    if (isspace(*p)) {
      // Skip trailing whitespace.
      while (isspace(*p)) p++;
      
      // Is it just a blank line?
      if (!*p) continue;

      if (!current_rule) {
        fprintf(stderr, "line %d: command with no target\n", line_num);
        return -1;
      }
      
      list_append(&current_rule->commands, strdup(p));
    } else {
      char *name_start;
      char *name_end;
      char type;
      
      // Expand macros
      if (expand_macros(line->start, prj, NULL, value) < 0) {
        fprintf(stderr, "line %d: expansion failed\n", line_num);
        return -1;
      }

      // Parse target or variable name
      p = value->start;
      name_start = p;
      while (*p && *p != ':' && *p != '=' && !isspace(*p)) p++;
      name_end = p;
      if (!*p || name_start == name_end) {
        fprintf(stderr, "line %d: syntax errror\n", line_num);
        return -1;
      }
      while (isspace(*p)) p++;
      type = *p++;
      while (isspace(*p)) p++;
      if (type != ':' && type != '=') {
        fprintf(stderr, "line %d: syntax errror, rule or variable expected\n", line_num);
        return -1;
      }
      buffer_set(name, name_start, name_end - name_start);
      buffer_add(name, 0);

      if (type == '=') {
        // Set variable
        setenv(name->start, p, 1);
        if (prj->debug) printf("set %s=%s\n", name->start, p);
      } else {
        // Find existing rule or create a new one
        struct rule *rule = find_rule(prj, name->start);
        if (!rule) rule = add_rule(prj, buffer_dup(name));
        parse_list(p, &rule->dependencies);
        current_rule = rule;
      }
    }
  }

  return 0;
}

time_t get_timestamp(char *filename) {
  struct stat st;
  
  if (stat(filename, &st) < 0) return -1;
  return st.st_mtime;
}

void mark_for_build(struct project *prj, struct rule *rule) {
  if (prj->build_last) prj->build_last->build_next = rule;
  if (!prj->build_first) prj->build_first = rule;
  prj->build_last = rule;
}

int check_rule(struct project *prj, struct rule *rule) {
  struct item *item;
  struct rule *dep;
  int dirty = 0;

  // Check for cyclic dependencies
  if (rule->status == PENDING) {
    fprintf(stderr, "Cyclic dependency on %s\n", rule->target);
    return -1;
  }

  // If this has already been checked we are done.
  if (rule->status != UNCHECKED) return 0;
      
  // Get timestamp for target
  rule->timestamp = get_timestamp(rule->target);
  
  // If this target does not exist we need to build this rule
  if (rule->timestamp == -1) {
    if (prj->debug) printf("build %s because it does not exist\n", rule->target);
    dirty = 1;
  }

  // Mark this target as pending
  rule->status = PENDING;
  
  // Check dependencies.
  item = rule->dependencies.head;
  while (item) {
    dep = find_rule(prj, item->value);
    if (dep) {
      // Check dependent rule
      if (check_rule(prj, dep) < 0) return -1;

      // If dependent rule is dirty we also need to build this rule
      if (dep->status == DIRTY) {
        if (prj->debug) printf("build %s because %s needs to be built\n", rule->target, item->value);
        dirty = 1;
      }
      
      // If this target is older than dependent we need to build this rule
      if (rule->timestamp < dep->timestamp) {
        if (prj->debug) printf("build %s because it is older than %s\n", rule->target, item->value);
        dirty = 1;
      }
    } else {
      // No rule for dependent, just check the timestamp.
      time_t t = get_timestamp(item->value);
      if (rule->timestamp < t) {
        if (prj->debug)  printf("build %s because it is older than %s\n", rule->target, item->value);
        dirty = 1;
      }
    }
    
    item = item->next;
  }
  
  // Always build phony targets
  if (prj->phony) {
    item = prj->phony->dependencies.head;
    while (item) {
      if (strcmp(item->value, rule->target) == 0) {
        if (prj->debug)  printf("build %s because it is a phony target\n", rule->target);
        dirty = 1;
        break;
      }
      item = item->next;
    }
  }
      
  // Mark as dirty if we are in always-build mode
  if (prj->always_build) dirty = 1;

  // Add rule to build list if it is dirty.
  if (dirty) {
    mark_for_build(prj, rule);
    rule->status = DIRTY;
  } else {
    rule->status = CLEAN;
  }

  return 0;
}

int check_targets(struct project *prj) {
  struct item *target = prj->targets.head;
  prj->phony = find_rule(prj, ".PHONY");
  while (target) {
    struct rule *r = find_rule(prj, target->value);
    if (!r) {
      // This is a classic!
      fprintf(stderr, "Don't know how to make %s\n", target->value);
      return -1;
    }
    if (check_rule(prj, r) < 0) return -1;
    target = target->next;
  }
  return 0;
}

int build_targets(struct project *prj) {
  struct buffer *cmd = &prj->value;
  struct rule *r = prj->build_first;
  while (r) {
    // Run commands in rule.
    struct item *command = r->commands.head;
    while (command) {
      // Expand macros in command.
      if (expand_macros(command->value, prj, r, cmd) < 0) return -1;
      if (!prj->silent) printf("%s\n", cmd->start);
      if (!prj->dry_run) {
        if (system(cmd->start) != 0) return -1;
      }

      command = command->next;
    }
    r = r->build_next;
  }
  return 0;
}

void usage() {
  fprintf(stderr, "usage: make [ -f <makefile> ] [ options ] ... [ targets ] ... \n\n");
  fprintf(stderr, "  -B            Unconditionally build all targets.\n");
  fprintf(stderr, "  -C <dir>      Change current directory to <dir> before building.\n");
  fprintf(stderr, "  -d            Output debug messages.\n");
  fprintf(stderr, "  -f <file>     Read <file> as makefle.\n");
  fprintf(stderr, "  -h            Print this message.\n");
  fprintf(stderr, "  -n            Display commands but do not build.\n");
  fprintf(stderr, "  -s            Do not print commands as they are executed.\n");
}

int main(int argc, char *argv[]) {
  int c;
  int i;
  int rc;
  char *makefile = "Makefile";
  char *dir = NULL;
  FILE *mf;
  struct project prj;

  // Initialize project
  setup_predefined_variables(&prj);
  project_init(&prj);
    
  // Parse command line options
  while ((c = getopt(argc, argv, "BC:df:hns")) != EOF) {
    switch (c) {
      case 'B':
        prj.always_build = 1;
        break;

      case 'C':
        dir = optarg;
        break;

      case 'd':
        prj.debug = 1;
        break;

      case 'f':
        makefile = optarg;
        break;
      
      case 'h':
        usage();
        return 1;
        
      case 'n':
        prj.dry_run = 1;
        break;

      case 's':
        prj.silent = 1;
        break;

      default:
        fprintf(stderr, "use -h for help\n");
        return 1;
    }
  }
  
  // Add targets and variables from command line
  for (i = optind; i < argc; ++i) {
    char *eq = strchr(argv[i], '=');
    if (eq) {
      // Set variable
      putenv(argv[i]);
    } else {
      // Add target.
      list_append(&prj.targets, strdup(argv[i]));
    }
  }

  // Change directory if requested
  if (dir) {
    if (chdir(dir) < 0) {
      perror(dir);
      project_free(&prj);
      return 1;
    }
  }

  // Read and parse makefile.
  mf = fopen(makefile, "r");
  if (mf) {
    rc = parse_makefile(&prj, mf);
    fclose(mf);
    if (rc < 0) {
      project_free(&prj);
      return 1;
    }
  }
  
  // Use the first rule if no targets were specified on the command line
  if (!prj.targets.head && prj.rules_head) {
    list_append(&prj.targets, strdup(prj.rules_head->target));
  }

  // Add and expand build targets
  rc = check_targets(&prj);
  if (rc < 0) {
    project_free(&prj);
    return 1;
  }
  
  // Build targets
  rc = build_targets(&prj);
  if (rc < 0) {
    project_free(&prj);
    return 1;
  }

  project_free(&prj);
  return 0;
}
