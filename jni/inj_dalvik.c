/*
 * ����ļ���������ע����ںͷ��ű��滻�߼���
 * 1���ڸú����м���libhook.soͨ�����е�do_hook��������ԭ����open��close��ַ�Լ�Ҫ�滻���µ�open��close������ַ
 * 2��Ȼ��̬��libnativehelper��̬�⣬��ȡ��ṹ�����ڱ��ҵ�ȫ�ַ��ű�GOT�����ñ�洢���ⲿ�������ŵĵ�ַ
 * 3������GOT���ҵ�ԭ�ȵ�open������close������ַ���ֱ��滻Ϊ�µ�open�������µ�close��������
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <asm/ptrace.h>
#include <asm/user.h>
#include <asm/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <jni.h>
#include <elf.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "utils.h"
#include <signal.h>
#include <sys/types.h>
#ifdef ANDROID
//#include <linker.h>
#endif
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <jni.h>

/*
 * �˴�����Ҫ���滻���ű��so�б�
 */
char *sos[] = { "libnativehelper.so", };

/*
 *�˴��򿪶�̬���ӿ⣬ͨ��do_hook�����õ��¾ɵ�ַ
 */
int hook_entry() {
	unsigned long old_open_addr;
	unsigned long new_open_addr;
	unsigned long old_close_addr;
	unsigned long new_close_addr;

	LOGD("hello ARM! pid:%d\n", getpid());
	void *handle;
	/**
	 * ����do_hook ����
	 */
	int (*fcn)(unsigned long *param, unsigned long *param1,
			unsigned long *param2, unsigned long *param3);
	int target_pid = getpid();

	handle = dlopen("/dev/libhook.so", RTLD_NOW);
	LOGD("The Handle of libhook: %x\n", handle);

	if (handle == NULL) {
		LOGD("Failed to load libhook.so: %s\n", dlerror());
		return 1;
	}

	/* ��̬��do_hook����*/
	LOGD("find do_hook pre %x\n", fcn);
	fcn = dlsym(handle, "do_hook");
	if (fcn != NULL)
		LOGD("find do_hook %x\n", fcn);
	else {
		LOGD("failed to find do_hook\n");
		return 0;
	}
	fcn(&old_open_addr, &new_open_addr, &old_close_addr, &new_close_addr);
	//ȡold_open_addr��ַ
	LOGD("[+] Get old address global  %x\n", old_open_addr);
	//ȡnew_open_addr��ַ
	LOGD("[+] Get new address global  %x\n", new_open_addr);
	LOGD("[+] Get old address global  %x\n", old_close_addr);
	LOGD("[+] Get new address global  %x\n", new_close_addr);
	return 0;
}

int main(int argc, char *argv[]) {
	int pid = 0;

	void *handle = NULL;
	long proc = 0;

	/*�˴�����Ҫע��Ľ���*/
	char *process = "com.speedsoftware.rootexplorer";
	//char *process="in.wptraffcianalyzer.filereadwritedemo";

	pid = find_pid_of(process);
	ptrace_attach(pid);
	ptrace_find_dlinfo(pid);

	handle = ptrace_dlopen(pid, "/dev/libhook.so", 2);
	printf("ptrace_dlopen handle %p\n", handle);
	hook_entry();

	/*�����滻open�����ķ��Ž�*/
	proc = (long) ptrace_dlsym(pid, handle, "new_open");
	printf("new_open = %lx\n", proc);
	LOGD("new_open = %lx\n", proc);
	replace_all_rels(pid, "open", proc, sos);

	/*�����滻close�����ķ��Ž�*/
	proc = (long) ptrace_dlsym(pid, handle, "new_close");
	printf("new_close = %lx\n", proc);
	LOGD("new_close = %lx\n", proc);
	replace_all_rels(pid, "close", proc, sos);

	ptrace_detach(pid);
}

