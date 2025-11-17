# Shell Sched

## How to Build
```shell
make release
```

## Commands

### Run:
```shell
./out/release/bin/shell_sched
```

After starting, the available commands are:
### User Scheduler
```shell
> shell_sched: create_create_user_scheduler <Number of Queues>
```

### Execute Process
```shell
> shell_sched: execute_process <Command> <Priority>
```

### List Scheduler
```shell
> shell_sched: list_scheduler
```

### Exit Scheduller
```shell
> shell_sched: exit_scheduler
```


## Tests
```
create_user_scheduler 3
execute_process ./test/small_for 2
execute_process ./test/small_for 1
```
fail: 
- se um processo p1 roda em prioridade x e novo processo p2 entra cm mesma prioridade x, scheduler interrompe p1 p executar p2
```

create_user_scheduler 3
execute_process ./test/40_sec 3
execute_process ./test/small_for 2
execute_process ./test/small_for 1
execute_process ./test/40_sec 1
```