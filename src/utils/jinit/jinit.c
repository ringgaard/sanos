//
// jinit.h
//
// Java VM launcher
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
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

#include <os.h>
#include <java/jni.h>
#include <string.h>
#include <inifile.h>
#include <stdlib.h>

struct section *cfg;
char *cfgname;
JavaVM *vm = NULL;
JNIEnv *env = NULL;

JavaVMInitArgs args;
hmodule_t hjvm;

jint (JNICALL *CreateJavaVM)(JavaVM **pvm, void **env, void *args);

jclass load_class(JNIEnv *env, char *name) {
  jclass cls;
  char clsname[128];
  char *s;
  char *t;

  s = name;
  t = clsname;
  while (*s) {
    *t++ = *s == '.' ? '/' : *s;
    s++;
  }
  *t = 0;

  cls = (*env)->FindClass(env, clsname);

  return cls;
}

jstring new_string(JNIEnv *env, char *s) {
  int len = strlen(s);
  jclass cls;
  jmethodID mid;
  jbyteArray ary;

  cls = (*env)->FindClass(env, "java/lang/String");
  mid = (*env)->GetMethodID(env, cls, "<init>", "([B)V");
  ary = (*env)->NewByteArray(env, len);
  if (ary != 0) {
    jstring str = 0;
    (*env)->SetByteArrayRegion(env, ary, 0, len, (jbyte *) s);
    
    if (!(*env)->ExceptionOccurred(env)) {
      str = (*env)->NewObject(env, cls, mid, ary);
    }

    (*env)->DeleteLocalRef(env, ary);
    
    return str;
  }
  
  return 0;
}

jobjectArray new_string_array(JNIEnv *env, int strc, char **strv) {
  jarray cls;
  jarray ary;
  int i;

  cls = (*env)->FindClass(env, "java/lang/String");
  ary = (*env)->NewObjectArray(env, strc, cls, 0);
  for (i = 0; i < strc; i++) {
    jstring str = new_string(env, *strv++);
    (*env)->SetObjectArrayElement(env, ary, i, str);
    (*env)->DeleteLocalRef(env, str);
  }

  return ary;
}

void init_jvm_args() {
  JavaVMOption *options;
  struct section *optsect;
  struct section *propsect;
  struct section *cpsect;
  int nopts;
  int n;
  struct property *prop;
  char *buf;
  char *p;
  int len;
  int first;

  cpsect = find_section(cfg, get_property(cfg, cfgname, "classpaths", "java.classpaths"));
  optsect = find_section(cfg, get_property(cfg, cfgname, "options", "java.options"));
  propsect = find_section(cfg, get_property(cfg, cfgname, "properties", "java.properties"));

  nopts = get_section_size(optsect) + get_section_size(propsect) + (cpsect ? 1 : 0);

  options = (JavaVMOption *) malloc(nopts * sizeof(JavaVMOption));
  memset(options, 0, nopts * sizeof(JavaVMOption));

  n = 0;

  if (cpsect) {
    len = strlen("-Djava.class.path=") + 1;
    prop = cpsect->properties;
    while (prop) {
      if (prop->name) len += strlen(prop->name) + 1;
      if (prop->value) len += strlen(prop->value) + 1;
      prop = prop->next;
    }

    buf = (char *) malloc(len);
    strcpy(buf, "-Djava.class.path=");
    p = buf + strlen(buf);
    first = 1;
    prop = cpsect->properties;
    while (prop) {
      if (!first) *p++ = ';';
      first = 0;

      if (prop->name) {
        len = strlen(prop->name);
        memcpy(p, prop->name, len + 1);
        p += len;
      }

      if (prop->value) {
        *p++ = ':';
        len = strlen(prop->value);
        memcpy(p, prop->value, len + 1);
        p += len;
      }

      prop = prop->next;
    }

    options[n++].optionString = buf;
  }

  if (optsect) {
    prop = optsect->properties;
    while (prop) {
      if (prop->value) {
        len = strlen(prop->name) + 1 + strlen(prop->value);
        buf = (char *) malloc(len + 1);
        strcpy(buf, prop->name);
        strcpy(buf + strlen(buf), ":");
        strcpy(buf + strlen(buf), prop->value);
      } else {
        len = strlen(prop->name);
        buf = (char *) malloc(len + 1);
        strcpy(buf, prop->name);
      }

      options[n++].optionString = buf;

      prop = prop->next;
    }
  }

  if (propsect) {
    prop = propsect->properties;
    while (prop) {
      if (prop->value) {
        len = 2 + strlen(prop->name) + 1 + strlen(prop->value);
      } else {
        len = 2 + strlen(prop->name);
      }

      buf = (char *) malloc(len + 1);
      strcpy(buf, "-D");
      strcpy(buf + strlen(buf), prop->name);

      if (prop->value) {
        strcpy(buf + strlen(buf), "=");
        strcpy(buf + strlen(buf), prop->value);
      }

      options[n++].optionString = buf;

      prop = prop->next;
    }
  }

  memset(&args, 0, sizeof(args));
  args.version  = JNI_VERSION_1_2;
  args.nOptions = nopts;
  args.options  = options;
  args.ignoreUnrecognized = JNI_FALSE;
}

int init_jvm() {
  jint rc;

  // Get JVM options from os.ini
  init_jvm_args();

  // Load VM
  if (hjvm == NULL) {
    char *jvmname = get_property(cfg, cfgname, "jvm", "jvm.dll");

    hjvm = dlopen(jvmname, 0);
    if (hjvm == NULL) {
      syslog(LOG_ERR, "Error loading JVM %s", jvmname);
      return -1;
    }
  }

  if (!CreateJavaVM) {
    CreateJavaVM = (jint (JNICALL *)(JavaVM **pvm, void **env, void *args)) dlsym(hjvm, "JNI_CreateJavaVM");
    if (!CreateJavaVM) {
      syslog(LOG_ERR, "Unable to find CreateJavaVM");
      return -1;
    }
  }

  // Create VM instance
  if (!vm) {
    rc = CreateJavaVM(&vm, (void **) &env, &args);
    if (rc != JNI_OK) {
      syslog(LOG_ERR, "Error %d creating java vm", rc);
      return -1;
    }
  } else {
    rc = (*vm)->AttachCurrentThread(vm, (void **) &env, &args);
    if (rc != JNI_OK) {
      syslog(LOG_ERR, "Error %d attaching to vm", rc);
      return -1;
    }
  }

  return 0;
}

int parse_args(char *args, char **argv) {
  char *p;
  int argc;
  char *start;
  char *end;
  char *buf;
  int delim;

  p = args;
  argc = 0;
  while (*p) {
    while (*p == ' ') p++;
    if (!*p) break;

    if (*p == '"' || *p == '\'') {
      delim = *p++;
      start = p;
      while (*p && *p != delim) p++;
      end = p;
      if (*p == delim) p++;
    } else {
      start = p;
      while (*p && *p != ' ') p++;
      end = p;
    }

    if (argv) {
      buf = (char *) malloc(end - start + 1);
      if (!buf) break;
      memcpy(buf, start, end - start);
      buf[end - start] = 0;
      argv[argc] = buf;
    }
    
    argc++;
  }

  return argc;
}

static void free_args(int argc, char **argv) {
  int i;

  for (i = 0; i < argc; i++) free(argv[i]);
  if (argv) free(argv);
}

int execute_main_method(char *mainclsname, char *mainclsargs) {
  int argc;
  char **argv;
  jclass mainclass;
  jmethodID mainid;
  jobjectArray mainargs;

  // Load main class
  mainclass = load_class(env, mainclsname);
  if (mainclass == NULL) {
    syslog(LOG_ERR, "Unable to find main class %s", mainclsname);
    return -1;
  }

  // Find main method
  mainid = (*env)->GetStaticMethodID(env, mainclass, "main", "([Ljava/lang/String;)V");
  if (mainid == NULL) {
    syslog(LOG_ERR, "Class %s does not have a main method", mainclsname);
    return -1;
  }

  // Create argument array
  argc = parse_args(mainclsargs, NULL);
  if (argc) {
    argv = (char **) malloc(argc * sizeof(char *));
    parse_args(mainclsargs, argv);
  } else {
    argv = NULL;
  }

  mainargs = new_string_array(env, argc, argv);
  if (mainargs == NULL) {
    syslog(LOG_ERR, "Error creating command arguments");
    return -1;
  }

  // Invoke main method
  (*env)->CallStaticVoidMethod(env, mainclass, mainid, mainargs);
  if ((*env)->ExceptionOccurred(env)) {
    (*env)->ExceptionDescribe(env);
    return -1;
  }

  free_args(argc, argv);
  return 0;
}

int main(int argc, char *argv[]) {
  char *mainclsname;
  char *mainclsargs;

  // Determine configuration
  if (argc > 1) {
    cfgname = argv[1];
  } else {
    cfgname = "java";
  }

  if (argc > 2) {
    cfg = read_properties(argv[2]);
    if (cfg == NULL) {
      syslog(LOG_ERR, "Unable to read JVM configuration from %s", argv[2]);
      return 1;
    }
  } else {
    cfg = osconfig();
  }

  // Initialize Java VM
  if (init_jvm() != 0) return 1;

  // Get main class and arguments
  mainclsname = get_property(cfg, cfgname, "mainclass", "sanos.os.Shell");
  mainclsargs = get_property(cfg, cfgname, "mainargs", "");

  // Call main method
  execute_main_method(mainclsname, mainclsargs);

  // Detach main thread from jvm
  if ((*vm)->DetachCurrentThread(vm) != 0) {
    syslog(LOG_ERR, "Could not detach main thread");
    return 1;
  }

  (*vm)->DestroyJavaVM(vm);
  return 0;
}
