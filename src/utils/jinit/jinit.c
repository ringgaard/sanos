//
// jinit.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Java VM launcher
//

#include <os.h>
#include <java/jni.h>
#include <string.h>
#include <inifile.h>
#include <stdlib.h>

JavaVM *vm = NULL;
JNIEnv *env = NULL;

JavaVMInitArgs args;
hmodule_t hjvm;

jint (JNICALL *CreateJavaVM)(JavaVM **pvm, void **env, void *args);

jclass load_class(JNIEnv *env, char *name)
{
  jclass cls;
  char clsname[128];
  char *s;
  char *t;

  s = name;
  t = clsname;
  while (*s)
  {
    *t++ = *s == '.' ? '/' : *s;
    s++;
  }
  *t = 0;

  cls = (*env)->FindClass(env, clsname);

  return cls;
}

jstring new_string(JNIEnv *env, char *s)
{
  int len = strlen(s);
  jclass cls;
  jmethodID mid;
  jbyteArray ary;

  cls = (*env)->FindClass(env, "java/lang/String");
  mid = (*env)->GetMethodID(env, cls, "<init>", "([B)V");
  ary = (*env)->NewByteArray(env, len);
  if (ary != 0) 
  {
    jstring str = 0;
    (*env)->SetByteArrayRegion(env, ary, 0, len, (jbyte *) s);
    
    if (!(*env)->ExceptionOccurred(env)) 
    {
      str = (*env)->NewObject(env, cls, mid, ary);
    }

    (*env)->DeleteLocalRef(env, ary);
    
    return str;
  }
  
  return 0;
}

jobjectArray new_string_array(JNIEnv *env, int strc, char **strv)
{
  jarray cls;
  jarray ary;
  int i;

  cls = (*env)->FindClass(env, "java/lang/String");
  ary = (*env)->NewObjectArray(env, strc, cls, 0);
  for (i = 0; i < strc; i++) 
  {
    jstring str = new_string(env, *strv++);
    (*env)->SetObjectArrayElement(env, ary, i, str);
    (*env)->DeleteLocalRef(env, str);
  }

  return ary;
}

void init_jvm_args()
{
  JavaVMOption *options;
  struct section *optsect;
  struct section *propsect;
  int nopts;
  int n;
  struct property *prop;
  char *buf;
  int len;

  optsect = find_section(config, "java.options");
  propsect = find_section(config, "java.properties");
  nopts = get_section_size(optsect) + get_section_size(propsect);

  options = (JavaVMOption *) malloc(nopts * sizeof(JavaVMOption));
  memset(options, 0, nopts * sizeof(JavaVMOption));

  n = 0;
  if (optsect)
  {
    prop = optsect->properties;
    while (prop)
    {
      if (prop->value)
      {
	len = strlen(prop->name) + 1 + strlen(prop->value);
	buf = (char *) malloc(len + 1);
	strcpy(buf, prop->name);
	strcpy(buf + strlen(buf), ":");
	strcpy(buf + strlen(buf), prop->value);
      }
      else
      {
	len = strlen(prop->name);
	buf = (char *) malloc(len + 1);
	strcpy(buf, prop->name);
      }

      options[n++].optionString = buf;

      prop = prop->next;
    }
  }

  if (propsect)
  {
    prop = propsect->properties;
    while (prop)
    {
      if (prop->value)
	len = 2 + strlen(prop->name) + 1 + strlen(prop->value);
      else
	len = 2 + strlen(prop->name);

      buf = (char *) malloc(len + 1);
      strcpy(buf, "-D");
      strcpy(buf + strlen(buf), prop->name);

      if (prop->value)
      {
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

int init_jvm()
{
  jint rc;

  // Get JVM options from os.ini
  init_jvm_args();

  // Load VM
  if (hjvm == NULL)
  {
    char *jvmname = get_property(config, "java", "jvm", "jvm.dll");

    hjvm = load(jvmname);
    if (hjvm == NULL) 
    {
      syslog(LOG_ERROR, "Error loading JVM %s\n", jvmname);
      return 1;
    }
  }

  if (!CreateJavaVM)
  {
    CreateJavaVM = (jint (JNICALL *)(JavaVM **pvm, void **env, void *args)) resolve(hjvm, "JNI_CreateJavaVM");
    if (!CreateJavaVM) 
    {
      syslog(LOG_ERROR, "Unable to find CreateJavaVM\n");
      return 1;
    }
  }

  // Create VM instance
  if (!vm)
  {
    rc = CreateJavaVM(&vm, (void **) &env, &args);
    if (rc != JNI_OK) 
    {
      syslog(LOG_ERROR, "Error %d creating java vm\n", rc);
      return 1;
    }
  }
  else
  {
#if 0
    rc = (*vm)->AttachCurrentThread(vm); 
    if (rc != JNI_OK) 
    {
      syslog(LOG_ERROR, "Error %d attaching to vm\n", rc);
      return 1;
    }
#endif
  }

  return 0;
}

int __stdcall main(hmodule_t hmod, char *cmdline, int reserved)
{
  char *mainclsname;
  char *mainclsargs;
  int argc;
  char **argv;

  jclass mainclass;
  jmethodID mainid;
  jobjectArray mainargs;

  // Initialize Java VM
  if (init_jvm() != 0) return 1;

  // Get main class and arguments
  if (cmdline && *cmdline)
  {
    mainclsname = cmdline;
    while (*cmdline && *cmdline != ' ') cmdline++;
    if (*cmdline)
    {
      *cmdline++ = 0;
      while (*cmdline == ' ') cmdline++;
      mainclsargs = cmdline;
    }
    else
      mainclsargs = "";
  }
  else
  {
    mainclsname = get_property(config, "java", "mainclass", "sanos.os.Shell");
    mainclsargs = get_property(config, "java", "mainargs", "");
  }

  // Load main class
  //syslog(LOG_DEBUG, "load main class %s\n", mainclsname);
  mainclass = load_class(env, mainclsname);
  if (mainclass == NULL) 
  {
    syslog(LOG_ERROR, "Unable to find main class %s\n", mainclsname);
    return 1;
  }

  // Find main method
  //syslog(LOG_DEBUG, "find main method\n");
  mainid = (*env)->GetStaticMethodID(env, mainclass, "main", "([Ljava/lang/String;)V");
  if (mainid == NULL) 
  {
    syslog(LOG_ERROR, "Class %s does not have a main method\n", mainclsname);
    return 1;
  }

  // Create argument array
  argc = parse_args(mainclsargs, NULL);
  if (argc)
  {
    argv = (char **) malloc(argc * sizeof(char *));
    parse_args(mainclsargs, argv);
  }
  else
    argv = NULL;

  mainargs = new_string_array(env, argc, argv);
  if (mainargs == NULL)
  {
    syslog(LOG_ERROR, "Error creating command arguments\n");
    return 1;
  }

  // Invoke main method
  //syslog(LOG_DEBUG, "invoke main\n");
  (*env)->CallStaticVoidMethod(env, mainclass, mainid, mainargs);
  //syslog(LOG_DEBUG, "return from main\n");
  if ((*env)->ExceptionOccurred(env)) (*env)->ExceptionDescribe(env);

#if 0
  if ((*vm)->DetachCurrentThread(vm) != 0) 
  {
    syslog(LOG_ERROR, "Could not detach main thread\n");
    return 1;
  }
#endif

  //(*vm)->DestroyJavaVM(vm);
  free_args(argc, argv);
  return 0;
}
