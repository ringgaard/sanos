#ifndef INIFILE_H
#define INIFILE_H

struct property;

struct section {
  char *name;
  struct section *next;
  struct property *properties;
};

struct property {
  char *name;
  char *value;
  struct property *next;
};

struct section *find_section(struct section *sect, char *name);
int get_section_size(struct section *sect);
char *find_property(struct section *sect, char *name);
char *get_property(struct section *sections, char *sectname, char *propname, char *defval);
int get_numeric_property(struct section *sections, char *sectname, char *propname, int defval);
void free_properties(struct section *sect);
struct section *parse_properties(char *props);
struct section *read_properties(char *filename);

#endif
