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
> shell_sched: user_scheduler <Number of Queues>
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

---------------------------------
### Create Example
```shell
make examples
```

### Run Examples
```shell
> shell_sched: execute_process out/test/test01 <priority>
```