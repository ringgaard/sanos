//
// ctohtml.c
//
// Convert C source files to HTML
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
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>

#define STRSIZE   1024
#define NUMFIELDS 10

struct tag {
  char *name;
  char *file;
  int line;
  int local;
};

char *source_dir = NULL;
char *output_dir = ".";
char *include_dir = "include";
char *ctags_file = NULL;
char *title = "source";
int source_index = 0;
size_t source_prefix_len = 0;

struct tag **tags = NULL;
int tags_size = 0;
int num_tags = 0;

char *keywords[] = {
  "#include", "#define", "#if", "#ifdef", "#ifndef", "#elif", "#else", "#endif", "#pragma",
  "int", "void", "char", "short", "long", "__int64", "float", "double", 
  "struct", "union", "enum", "typedef", "sizeof",
  "if", "else", "while", "break", "return", "for", "goto", "do", 
  "continue", "switch", "case", "default",
  "extern", "static", "unsigned", 
  "const", "__const", "volatile", "__volatile", 
  "register", "signed", "auto", "inline", "__inline", "__declspec",
  "asm", "__asm",
  NULL,
};

int is_keyword(char *str) {
  char **keyword = keywords;
  while (*keyword) {
    if (strcmp(str, *keyword) == 0) return 1;
    keyword++;
  }
  return 0;
}

void output_html(FILE *out, char *str, char *end) {
  while (str < end) {
    if (*str == '<') {
      fputs("&lt;", out);
    } else if (*str == '>') {
      fputs("&gt;", out);
    } else if (*str == '&') {
      fputs("&amp;", out);
    } else {
      fputc(*str, out);
    }

    str++;
  }
}

int split_string(char *str, char **fields, int max_fields) {
  int n = 0;
  fields[n++] = str;
  while (*str) {
    if (n == max_fields) break;
    if (*str == '\t' || *str == '\r' || *str == '\n') {
      *str = 0;
      fields[n++] = str + 1;
    }
    str++;
  }
  return n;
}

void relative_url(char *source, char *target, char *url) {
  char *s = source;
  char *t = target;
  while (*s && *t && *s == *t) {
    int sep = *s == '/';
    s++;
    t++;
    if (sep) {
      source = s;
      target = t;
    }
  }
  
  url[0] = 0;
  while (*source) {
    if (*source == '/') strcat(url, "../");
    source++;
  }
  strcat(url, target);
}

void read_ctags(char *ctags_file)  {
  char line[STRSIZE];
  char *field[NUMFIELDS];
  struct tag *t;
  int i, n;
  FILE *f;
  char *s;
   
  f = fopen(ctags_file, "r");
  if (!f) {
    perror(ctags_file);
    return;
  }
  
  while (fgets(line, sizeof line, f)) {
    if (*line == '!') continue;
    for (i = 0; i < NUMFIELDS; ++i) field[i] = NULL;
    n = split_string(line, field, NUMFIELDS);
    if (n < 4) continue;
    if (strcmp(field[3], "m") == 0) continue;

    if (num_tags == tags_size) {
      tags_size = tags_size ? tags_size * 2 : 64;
      tags = (struct tag **) realloc(tags, sizeof(struct tag *) * tags_size);
    }
    t = tags[num_tags++] = (struct tag *) malloc(sizeof(struct tag));
    t->name = strdup(field[0]);
    t->file = strdup(field[1]);
    s = t->file;
    while (*s) {
      if (*s == '\\') *s = '/';
      s++;
    }
    t->line = atoi(field[2]);
    t->local = 0;
    for (i = 4; i < n; ++i) {
      if (strcmp(field[i], "file:") == 0) t->local = 1;
    }
  }
  fclose(f);
}

struct tag *find_tag(char *name, char *file) {
  int lo = 0;
  int hi = num_tags - 1;
  while (lo <= hi) {
    int mid = lo + (hi - lo) / 2;
    int compare = strcmp(tags[mid]->name, name);
    if (compare == 0) {
      int i = mid;
      while (i > 0 && strcmp(tags[i - 1]->name, name) == 0) i--;
      while (i < num_tags && strcmp(tags[i]->name, name) == 0) {
        if (!tags[i]->local || strcmp(tags[i]->file, file) == 0) return tags[i];
        i++;
      }
      return NULL;
    } else if (compare < 0) {
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
  }
  return NULL;
}

void convert_file(char *source_filename) {
  char html_filename[NAME_MAX + 1];
  char *relative_filename;
  char *base_filename;
  char *ext, *p;
  int is_c = 0;
  int is_asm = 0;
  FILE *src;
  FILE *out;
  int line_num;
  char line[STRSIZE];
  char filename[STRSIZE];
  char token[STRSIZE];
  char url[STRSIZE];
  int in_comment;
  
  relative_filename = source_filename + source_prefix_len;
  ext = "";
  base_filename = relative_filename;
  p = relative_filename;
  while (*p) {
    if (*p == '/') {
      base_filename = p + 1;
      ext = "";
    }
    if (*p == '.') ext = p;
    p++;
  }
  
  strcpy(html_filename, output_dir);
  strcat(html_filename, "/");
  strcat(html_filename, relative_filename);
  strcat(html_filename, ".html");

  if (strcmp(ext, ".c") == 0) is_c = 1;
  if (strcmp(ext, ".h") == 0) is_c = 1;
  if (strcmp(ext, ".cpp") == 0) is_c = 1;
  if (strcmp(ext, ".asm") == 0) is_asm = 1;
  if (strcmp(ext, ".s") == 0) is_asm = 1;

  if (!is_c && !is_asm) return;
  
  src = fopen(source_filename, "rt");
  if (!src) {
    perror(source_filename);
    return;
  }
  
  out = fopen(html_filename, "wb");
  if (!out) {
    perror(html_filename);
    return;
  }

  fprintf(out, "<html>\r\n");
  fprintf(out, "<head>\r\n");
  fprintf(out, "<title>%s - %s</title>\r\n", base_filename, title);
  fprintf(out, "<style type='text/css'>\r\n");
  fprintf(out, "a:link {text-decoration: none; color:inherit}\r\n");
  fprintf(out, "a:visited {text-decoration: none; color:inherit}\r\n");
  fprintf(out, "a:active {text-decoration: none; color:inherit}\r\n");
  fprintf(out, "</style>\r\n");
  fprintf(out, "</head>\r\n");
  fprintf(out, "<body>\r\n");
  if (source_index) {
    fprintf(out, "<p style='float: right'><a href='/sanos/source/index.html'>Goto sanos source index</a></p>");
  }
  fprintf(out, "<pre>\r\n");
  
  line_num = 1;
  in_comment = 0;
  while (fgets(line, sizeof line, src)) {
    char *p = line;
    char *end = line;
    while (*end && *end != '\r' && *end != '\n') end++;
    *end = 0;
    
    if (in_comment) {
      char *comment_start = p;
      fprintf(out, "<font color='green'>");
      while (p < end) {
        if (p[0] == '*' && p[1] == '/') {
          p += 2;
          output_html(out, comment_start, p);
          in_comment = 0;
          fprintf(out, "</font>");
          break;
        }
        p++;
      }
    }

    fprintf(out, "<a name=':%d'></a>", line_num);
    
    if (!is_c) {
      output_html(out, p, end);
      p = end;
    }

    while (p < end) {
      if (p[0] == '/' && p[1] == '/') {
        fprintf(out, "<font color='green'>");
        output_html(out, p, end);
        fprintf(out, "</font>");
        p = end;
      } else if (p[0] == '/' && p[1] == '*') {
        char *comment_start = p;
        fprintf(out, "<font color='green'>");
        while (p < end) {
          if (p[0] == '*' && p[1] == '/') {
            p += 2;
            output_html(out, comment_start, p);
            in_comment = 0;
            fprintf(out, "</font>");
            break;
          }
          p++;
        }
      } else if (*p == '\'' || *p == '"') {
        char *start = p++;
        while (*p && *p != *start) {
          if (*p == '\\' && *(p + 1)) p++;
          p++;
        }
        if (*p) p++;
        fprintf(out, "<font color='brown'>");
        output_html(out, start, p);
        fprintf(out, "</font>");
      } else if (*p == '#' || *p == '_' || isalpha(*p)) {
        char *start = p++;
        while (*p && (*p == '_' || isalnum(*p))) p++;
        memcpy(token, start, p - start);
        token[p - start] = 0;
        if (is_keyword(token)) {
          fprintf(out, "<font color='blue'>");
          output_html(out, start, p);
          fprintf(out, "</font>");
          
          if (strncmp(start, "#include", 8) == 0) {
            start = p;
            while (isspace(*p)) p++;
            output_html(out, start, p);
            
            if (*p == '"' || *p == '<') {
              int stdincl = *p == '<';
              output_html(out, p, p + 1);
              start = ++p;
              while (*p && *p != '>' && *p != '"') p++;
              if (stdincl) {
                char *base;
                
                strcpy(filename, include_dir);
                strcat(filename, "/");
                base = filename + strlen(filename);
                memcpy(base, start, p - start);
                base[p - start] = 0;
              } else {
                int pathlen = base_filename - relative_filename;
                int fnlen = p - start;
                memcpy(filename, relative_filename, pathlen);
                memcpy(filename + pathlen, start, fnlen);
                filename[pathlen + fnlen] = 0;
              }
              relative_url(relative_filename, filename, url);
              fprintf(out, "<a href='%s.html'>", url);
              output_html(out, start, p);
              fprintf(out, "</a>");
              if (*p) {
                output_html(out, p, p + 1);
                p++;
              }
            }
          }
        } else {
          struct tag *tag = find_tag(token, relative_filename);
          if (tag) {
            int self;
            
            relative_url(relative_filename, tag->file, url);
            self = strcmp(url, base_filename) == 0 && tag->line == line_num;
            if (!self) fprintf(out, "<a href='%s.html#:%d'>", url, tag->line);
            output_html(out, start, p);
            if (!self) fprintf(out, "</a>");
          } else {
            output_html(out, start, p);
          }
        }
      } else {
        output_html(out, p, p + 1);
        p++;
      }
    }
    if (in_comment) fprintf(out, "</font>");
    fprintf(out, "\r\n", line);
    line_num++;
  }
  
  fprintf(out, "</pre>\r\n");
  fprintf(out, "</body>\r\n");
  fprintf(out, "</html>\r\n");

  fclose(src);
  fclose(out);
}

void convert_directory(char *path) {
  struct dirent *dp;
  DIR *dirp;
  char filename[NAME_MAX + 1];
  char dirname[NAME_MAX + 1];
  struct stat st;

  strcpy(dirname, output_dir);
  if (strlen(path) > source_prefix_len) {
    strcat(dirname, "/");
    strcat(dirname, path + source_prefix_len);
  }
  mkdir(dirname, 0644);

  dirp = opendir(path);
  if (!dirp) return;
  while ((dp = readdir(dirp))) {
    if (strcmp(dp->d_name, ".") == 0) continue;
    if (strcmp(dp->d_name, "..") == 0) continue;
    if (strcmp(dp->d_name, "CVS") == 0) continue;
    
    strcpy(filename, path);
    strcat(filename, "/");
    strcat(filename, dp->d_name);
    if (stat(filename, &st) < 0) continue;
    if (S_ISDIR(st.st_mode)) convert_directory(filename);
    if (S_ISREG(st.st_mode)) convert_file(filename);
  }
  closedir(dirp);
}

void usage() {
  fprintf(stderr, "usage: ctohtml [options]\n\n");
  fprintf(stderr, "  -h            Print this message.\n");
  fprintf(stderr, "  -d <dir>      Input directory for source files.\n");
  fprintf(stderr, "  -c <file>     CTAGS file for cross references.\n");
  fprintf(stderr, "  -o <dir>      Output directory for html files.\n");
  fprintf(stderr, "  -i <dir>      Include file directory relative to source.\n");
  fprintf(stderr, "  -l            Add link to sanos source index.\n");
  fprintf(stderr, "  -t <title>    Title for HTML pages.\n");
}

int main(int argc, char *argv[]) {
  int c, i;

  // Parse command line options
  while ((c = getopt(argc, argv, "d:c:i:o:hlt:")) != EOF) {
    switch (c) {
      case 'd':
        source_dir = optarg;
        break;

      case 'c':
        ctags_file = optarg;
        break;

      case 'o':
        output_dir = optarg;
        break;

      case 'i':
        title = include_dir;
        break;
        
      case 'l':
        source_index = 1;
        break;

      case 't':
        title = optarg;
        break;

      case 'h':
      default:
        usage();
        return 1;
    }
  }
  if (!source_dir) {
    usage();
    return 1;
  }

  // Read CTAGS file
  if (ctags_file != NULL) read_ctags(ctags_file);
  
  // Convert all source files
  source_prefix_len = strlen(source_dir) + 1;
  convert_directory(source_dir);
  
  // Deallocate tags
  for (i = 0; i < num_tags; ++i) {
    free(tags[i]->name);
    free(tags[i]->file);
    free(tags[i]);
  }
  if (tags) free(tags);
  
  return 0;
}
