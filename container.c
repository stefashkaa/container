#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];

static int child_fn() {
    printf("PID: %ld\n", (long) getpid());
    pid_t child_pid = fork();
    if (child_pid) {
        printf("Clone fork child PID: %ld\n", (long) child_pid);

        // create and mount new namespace
        system("mount --make-rprivate -o remount /");
        system("mount proc /proc -t proc --make-private");

        // configure new network interface
		printf("\n_________________________________________\n");

		// guest network namespace
        system("ifconfig veth1 10.1.1.2/24 up");
        system("ip link");
		printf("\n_________________________________________\n");

		printf("Loop:\n");
        system("dd if=/dev/zero of=image.fs count=10 bs=1048576");
        system("losetup /dev/loop7 image.fs");
        system("mkfs.ext4 image.fs");
        system("mount -t ext4 /dev/loop7 /home --make-private");
        system("dd if=/dev/zero of=/home/check_file.txt count=1 bs=1024000");

		printf("\n_________________________________________\n");
        printf("Files are located in the new mountspace /home:\n");
        system("ls /home");

		printf("\n_________________________________________\n");
		printf("TEST CONTAINER:\n");
        system("sysbench --test=cpu --cpu-max-prime=20000 run");
		system("sysbench --test=memory --memory-block-size=1M --memory-total-size=10G --num-threads=1 run");
        system("sysbench --test=fileio --file-total-size=40G prepare");
        system("sysbench --test=fileio --file-total-size=40G --file-test-mode=rndrw --init-rng=on --max-time=300 --max-requests=0 run");
        system("sysbench --test=fileio --file-total-size=40G cleanup");
		printf("\n_________________________________________\n");

        system("bash");
    }
    return EXIT_SUCCESS;
}

int main() {
    pid_t child_pid = clone(child_fn, child_stack + STACK_SIZE, CLONE_NEWPID | CLONE_NEWNET | SIGCHLD | CLONE_NEWNS, NULL);
	printf("\n_________________________________________\n");
	printf("clone() = %ld", (long) child_pid);
	printf("\n_________________________________________\n");

	printf("Binding child process with cgroups...Done!");
	char buffer[1024 * sizeof(char)];
    sprintf(buffer, "echo %d > /sys/fs/cgroup/cpu/demo/tasks", child_pid);
    system(buffer);

    // configure network interfaces
    char newnt[1024 * sizeof(char)];
    sprintf(newnt, "ip link add name veth0 type veth peer name veth1 netns %ld", (long) child_pid);
    system(newnt);

	// host network namespace
    system("ifconfig veth0 10.1.1.1/24 up");
    system("ip link");
	printf("\n_________________________________________\n");

    waitpid(child_pid, NULL, 0);

    // check that 'check_file.txt' is not in the host
    printf("Files on the local machine:\n");
    system("ls");
    return EXIT_SUCCESS;
}
