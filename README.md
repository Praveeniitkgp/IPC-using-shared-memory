# IPC-using-shared-memory
Lightweight Custom Discovery Protocol (LCDP): Custom network protocol using raw sockets.
Overview:
Leader process (L) and n followers (F_1, ..., F_n) use shared memory to compute sums of random integers (leader: 1-99, followers: 1-9). Terminates when a sum repeats.
Files:
leader.c: Leader process.
follower.c: Follower processes.
README.md: This file.
Build
gcc -o leader leader.c
gcc -o follower follower.c

Run
Leader:./leader 15
Followers (multiple terminals):./follower 5

Assumptions
Linux, n ≤ 100.
No mutual exclusion for M[1].
Leader runs first; followers fail if no leader.

Sample Output
Leader:
$ ./leader 15
88+4+9+...+6=169
25+9+7+...+9=113

Follower:
$ ./follower
follower 1 joins

