#include <os.h>

typedef void *FILE;

#include <jni.h>

// osexec jexec.exe org.apache.tomcat.startup.Tomcat

#define CLASSPATH "c:\\classes\\;c:\\jre\\lib\\tools.jar;c:\\tomcat\\lib\\webserver.jar;c:\\tomcat\\lib\\servlet.jar;c:\\tomcat\\lib\\parser.jar;c:\\tomcat\\lib\\jaxp.jar;c:\\tomcat\\lib\\jasper.jar"
#define JRE_PATH "c:\\jre"

//#define JVM      "classic"
#define JVM      "hotspot"

JavaVM *vm = NULL;
JNIEnv *env = NULL;

JavaVMOption options[] =
{
  {"-Djava.class.path=" CLASSPATH, NULL},
  {"-Dtomcat.home=c:\\tomcat", NULL},
  //{"-verbose", NULL},
  //{"-verbose:Xclassdep", NULL},
  //{"-verbose:class", NULL},
  //{"-verbose:jni", NULL},
};

#define NJOPTS 2

JavaVMInitArgs args;

hmodule_t hjvm;

char *argv[] = {"jexec", "1", "2", "3"};
int argc = 1;

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

jobjectArray new_string_array(JNIEnv *env, char **strv, int strc)
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

int __stdcall main(hmodule_t hmod, char *cmdline, int reserved)
{
  jint r;
  jclass mainclass;
  jmethodID mainid;
  jobjectArray mainargs;

  hjvm = load_module(JRE_PATH "\\bin\\" JVM "\\jvm.dll");
  if (hjvm == NULL) panic("error loading jvm.dll\n");

  memset(&args, 0, sizeof(args));
  args.version  = JNI_VERSION_1_2;
  args.nOptions = NJOPTS;
  args.options  = options;
  args.ignoreUnrecognized = JNI_FALSE;

  CreateJavaVM = (jint (JNICALL *)(JavaVM **pvm, void **env, void *args)) get_proc_address(hjvm, "JNI_CreateJavaVM");
  if (!CreateJavaVM) panic("unable to find CreateJavaVM\n");

  syslog(LOG_DEBUG, "create vm\n");
  r = CreateJavaVM(&vm, (void **) &env, &args);
  if (r != JNI_OK) panic("error creating java vm\n");

  syslog(LOG_DEBUG, "load main class\n");
  mainclass = load_class(env, cmdline);
  if (mainclass == NULL) panic("unable to load main class\n");

  syslog(LOG_DEBUG, "find main method\n");
  mainid = (*env)->GetStaticMethodID(env, mainclass, "main", "([Ljava/lang/String;)V");
  if (mainid == NULL) panic("unable to find main method\n");

  syslog(LOG_DEBUG, "create args\n");
  mainargs = new_string_array(env, argv + 1, argc - 1);
  if (mainargs == NULL) panic("error creating command arguments\n");

  syslog(LOG_DEBUG, "invoke main\n");
  (*env)->CallStaticVoidMethod(env, mainclass, mainid, mainargs);
  syslog(LOG_DEBUG, "return from main\n");
  if ((*env)->ExceptionOccurred(env)) 
  {
    (*env)->ExceptionDescribe(env);
    goto leave;
  }

  if ((*vm)->DetachCurrentThread(vm) != 0) panic("could not detach main thread\n");
  
leave:
  (*vm)->DestroyJavaVM(vm);
  syslog(LOG_DEBUG, "finito\n");
  return 0;
}
