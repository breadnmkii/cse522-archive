1.
Brendan Yang,  Haoxuan Sun, Haoran Qin

2. 
The values reported are: disk space (1k blks), disk usage, remaining disk,
and mount location. These values indicate where the filesystems can be found
as well as how large in capacity they are. This can indicate which filesystems 
are more used by clients.
    (1) home dir:
        fsys:   nfs.seas.wustl.edu:/seaslab/home-lab
        blks:   18473191424   
        used:   11096308736  
        rmnd:   7376882688  
        usge:   61% 
        mntp:   /home/warehouse
    (2) /project/scratch01: 
        fsys:   compute-scratch02.engr.wustl.edu:/export/scratch01
        blks:   1073216512   
        used:   697803776  
        rmnd:   375412736  
        usge:   66% 
        mntp:   /project/scratch01
    (3) cgroup:
        fsys:   tmpfs
        blks:   4087056   
        used:   0  
        rmnd:   4087056  
        usge:   0% 
        mntp:   /sys/fs/cgroup

3.
$ls -i *
36448140 myln           36443801 myln.c         36440688 readme.txt

subdir0:
36443393 myfile

subdir1:
36443393 myfile

subdir2:
36443393 myfile         36443393 testname

They have the same inode number.

4. The files also have the same reuslting inode number
(Note: Are different as we did part 4 on another filesys)

5. 
ls: IN_ACCESS
mv: IN_OPEN, IN_ACCESS, IN_CLOSE
touch: IN_CREATE

6. 
Move the file out of the subdirectory to which the first subdirectory has been bind mounted, then back into it, using the mv command：
mv: cannot move 'myfile' to '../myfile': Device or resource busy

Update the timestamp of one of the hard links to the file with the touch command：
IN_CLOSE wd=1 mask=32 cookie=0 len=16 dir=no
name=myfile
IN_OPEN wd=1 mask=1 cookie=0 len=16 dir=no
name=myfile
IN_ACCESS wd=1 mask=8 cookie=0 len=16 dir=no
name=myfile

7. 
inotify_add_watch success. inotify file descriptor: 3, descriptor for water: 1, file path: subdir0
wd=1 mask=1073741856 cookie=0 len=0 dir=yes
IN_OPEN wd=1 mask=1073741825 cookie=0 len=0 dir=yes
IN_ACCESS wd=1 mask=1073741840 cookie=0 len=0 dir=yes

IN_CLOSE Standard input stream ready.
The string is: 
standard add_watch failed, errno: 2
s
Standard input stream ready.
The string is: s
standard add_watch failed, errno: 2
subdir1
Standard input stream ready.
The string is: subdir1
wd=1 mask=1073741856 cookie=0 len=0 dir=yes
IN_OPEN wd=1 mask=1073741825 cookie=0 len=0 dir=yes
IN_ACCESS wd=1 mask=1073741840 cookie=0 len=0 dir=yes
IN_CLOSE wd=1 mask=512 cookie=0 len=16 dir=no
name=part7file
IN_DELETE wd=2 mask=1073741856 cookie=0 len=0 dir=yes
IN_OPEN wd=2 mask=1073741825 cookie=0 len=0 dir=yes
IN_ACCESS wd=2 mask=1073741840 cookie=0 len=0 dir=yes
IN_CLOSE wd=2 mask=512 cookie=0 len=16 dir=no
name=part7file

Typing the correct directory will add the inotify another path to monite. 

8.
pi@raspberrypi:~/Desktop/cse522/studio4 $ ./inotify subdir0
inotify_add_watch success. inotify file descriptor: 3, descriptor for water: 1, file path: subdir0
subdir1
Standard input stream ready.
The string is: subdir1
standard add_watch failed, errno: 28

The inotify can only add one watch to subdir0, it shown error when it watch exxceed the limit when adding subdir1. 