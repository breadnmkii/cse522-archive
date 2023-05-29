1. Brendan Yang   Haoxuan Sun   Haoran Qin

2.
pi@pi:~ $ mount | grep cgroup
cgroup2 on /sys/fs/cgroup type cgroup2 (rw,nosuid,nodev,noexec,relatime,nsdelegate,memory_recursiveprot)

Explain: This command shows the cgroup filesystem that are currently mounted. 'mount' command lists all the mounted file systems, and 'grep cgroup' filters the output to list only the results that contain the string "cgroup". 

pi@pi:~ $ cat /proc/cgroups
#subsys_name	hierarchy	num_cgroups	enabled
cpuset	0	91	1
cpu	0	91	1
cpuacct	0	91	1
blkio	0	91	1
memory	0	91	1
devices	0	91	1
freezer	0	91	1
net_cls	0	91	1
perf_event	0	91	1
net_prio	0	91	1
pids	0	91	1

Explain: In this example, the system has access to a total of 10 cgroup subsystems, all of which belong to hierarchy 0. All subsystems listed here have been enabled.


3.
    (1)
        root@pi:/sys/fs/cgroup# cat cgroup.controllers 
        cpuset cpu io memory pids

        root@pi:/sys/fs/cgroup# cat cgroup.subtree_control 
        memory pids

    (2) 
        root@pi:/sys/fs/cgroup/s8test# cat cgroup.controllers 
        memory pids
        root@pi:/sys/fs/cgroup/s8test# cat cgroup.subtree_control 
        (empty)
    
    (3)
        root@pi:/sys/fs/cgroup/s8test# ls
        cgroup.controllers	cgroup.type	     memory.oom.group
        cgroup.events		cpu.stat	     memory.stat
        cgroup.freeze		memory.current	     memory.swap.current
        cgroup.max.depth	memory.events	     memory.swap.events
        cgroup.max.descendants	memory.events.local  memory.swap.high
        cgroup.procs		memory.high	     memory.swap.max
        cgroup.stat		memory.low	     pids.current
        cgroup.subtree_control	memory.max	     pids.events
        cgroup.threads		memory.min	     pids.max

    (4)
        root@pi:/sys/fs/cgroup# echo "-pids" > cgroup.subtree_control
        bash: echo: write error: Device or resource busy

    (5)
        root@pi:/sys/fs/cgroup/s8test# echo "-memory" > cgroup.controllers 
        bash: echo: write error: Invalid argument
        Because the memory constraint is inherited from parent cgroup, which is a read-only file from child's directory and therefore couldn't be modified.


4.
pi@pi:~ $ echo $$
1448
pi@pi:~ $ cat /proc/self/cgroup 
0::/s8test
    
    Before forkbomb:
        root@pi:/sys/fs/cgroup/s8test# cat cgroup.procs 
        1448

        root@pi:/sys/fs/cgroup/s8test# cat memory.current 
        94208

        root@pi:/sys/fs/cgroup/s8test# cat memory.stat
        anon 0
        file 0
        kernel_stack 0
        percpu 0
        sock 0
        shmem 0
        file_mapped 0
        file_dirty 0
        file_writeback 0
        inactive_anon 0
        active_anon 0
        inactive_file 0
        active_file 0
        unevictable 0
        slab_reclaimable 0
        slab_unreclaimable 0
        slab 0
        workingset_refault_anon 0
        workingset_refault_file 0
        workingset_activate_anon 0
        workingset_activate_file 0
        workingset_restore_anon 0
        workingset_restore_file 0
        workingset_nodereclaim 0
        pgfault 132
        pgmajfault 0
        pgrefill 0
        pgscan 0
        pgsteal 0
        pgactivate 0
        pgdeactivate 0
        pglazyfree 0
        pglazyfreed 0

    During forkbomb:
        root@pi:/sys/fs/cgroup/s8test# cat cgroup.procs 
        1448
        1506
        1507
        1508
        1509

        root@pi:/sys/fs/cgroup/s8test# cat memory.current 
        954368

        root@pi:/sys/fs/cgroup/s8test# cat memory.stat
        anon 405504
        file 0
        kernel_stack 40960
        percpu 0
        sock 0
        shmem 0
        file_mapped 0
        file_dirty 0
        file_writeback 0
        inactive_anon 405504
        active_anon 0
        inactive_file 0
        active_file 0
        unevictable 0
        slab_reclaimable 0
        slab_unreclaimable 0
        slab 0
        workingset_refault_anon 0
        workingset_refault_file 0
        workingset_activate_anon 0
        workingset_activate_file 0
        workingset_restore_anon 0
        workingset_restore_file 0
        workingset_nodereclaim 0
        pgfault 2343
        pgmajfault 0
        pgrefill 0
        pgscan 0
        pgsteal 0
        pgactivate 0
        pgdeactivate 0
        pglazyfree 0
        pglazyfreed 0

    After forkbomb: 
        root@pi:/sys/fs/cgroup/s8test# cat cgroup.procs 
        1448    

        root@pi:/sys/fs/cgroup/s8test# cat memory.current 
        204800

        root@pi:/sys/fs/cgroup/s8test# cat memory.stat
        anon 135168
        file 0
        kernel_stack 0
        percpu 0
        sock 0
        shmem 0
        file_mapped 0
        file_dirty 0
        file_writeback 0
        inactive_anon 135168
        active_anon 0
        inactive_file 0
        active_file 0
        unevictable 0
        slab_reclaimable 0
        slab_unreclaimable 0
        slab 0
        workingset_refault_anon 0
        workingset_refault_file 0
        workingset_activate_anon 0
        workingset_activate_file 0
        workingset_restore_anon 0
        workingset_restore_file 0
        workingset_nodereclaim 0
        pgfault 2706
        pgmajfault 0
        pgrefill 0
        pgscan 0
        pgsteal 0
        pgactivate 0
        pgdeactivate 0
        pglazyfree 0
        pglazyfreed 0
        
    cgroup.procs records all forked processes during the forkbomb. memory.current shows current memory usage, which increases dramatically during the forkbomb.

    the 'pgfault' in memory.stat is the counted number of page fault that occuried among the cgroup. When forkbomb is triggered, the malloc() function will use mmap syscall to alloc new pages, which will generate page faults. That is why we saw a pgfault increase.

5.
root@pi:/sys/fs/cgroup/s8test# echo "200000" > memory.max
root@pi:/sys/fs/cgroup/s8test# cat memory.max
196608

These values differed because the memory.max is aligned to pages

pi@pi:~/522s/CSE522-SP23/playground $ ./forkbomb
Connection to pi.local closed.

I lost ssh connection to my pi after executing the forkbomb. That is because the cgroup used too much memory and the cgroup killed the parent shell process running the forkbomb

root@pi:/sys/fs/cgroup/s8test# cat memory.events
low 0
high 0
max 4878
oom 4
oom_kill 2

According to kernel docs: (https://docs.kernel.org/admin-guide/cgroup-v2.html)

low
The number of times the cgroup is reclaimed due to high memory pressure even though its usage is under the low boundary. This usually indicates that the low boundary is over-committed.

high
The number of times processes of the cgroup are throttled and routed to perform direct memory reclaim because the high memory boundary was exceeded. For a cgroup whose memory usage is capped by the high limit rather than global memory pressure, this event’s occurrences are expected.

max
The number of times the cgroup’s memory usage was about to go over the max boundary. If direct reclaim fails to bring it down, the cgroup goes to OOM state.

oom
The number of time the cgroup’s memory usage was reached the limit and allocation was about to fail.

This event is not raised if the OOM killer is not considered as an option, e.g. for failed high-order allocations or if caller asked to not retry attempts.

oom_kill
The number of processes belonging to this cgroup killed by any kind of OOM killer.

6.
root@pi:/sys/fs/cgroup/s8test# cat memory.current 
311296
root@pi:/sys/fs/cgroup/s8test# cat memory.max
954368
root@pi:/sys/fs/cgroup/s8test# echo "454368" > memory.high
root@pi:/sys/fs/cgroup/s8test# cat memory.high
450560

pi@pi:~/522s/CSE522-SP23/playground $ ./monitor /sys/fs/cgroup/s8test/memory.events /sys/fs/cgroup/s8test/cgroup.events 
wd=1
high: 203
wd=1
high: 206
wd=1
high: 206
wd=1
high: 209
wd=1
high: 212
...
wd=1
high: 293
wd=2
populated: 0

The forkbomb will cause the memory usage to exceeds the "high" threshold. After the termination of (1) shell, the monitor outputs the populated status accordingly (populated: 0).

7.

== container1 cgroup.procs output ==
External:
11487
11488

Internal:
0
1
3

Explain: We see differing processes based on the view of the container from within versus outside.
Within the container, the user sees the processes that it runs as direct children from the root
init process, mimicking the virtualized environment that spawned as the only process.
Outside the container, we can actually see that the namespace is running as another process under
the real system, thus its PID is a much larger number.


== cgroup file output ==
External:
0::/pi_containers/container1

Internal:
0::/

Explain: The internal container sees only its hierarchy and downwards, thus its cgroup appears to it
as the root group. Outside the container, the real system can see the container actually belongs to
a subtree of cgroups.


== writing process output ==
External:
-bash: echo: write error: Permission denied

Internal:
Word expansion failed

Explain: They both fail as expected, the isolation ensured that the cloned namespace would not be able to expand its permissions outside its specified cgroup container.

