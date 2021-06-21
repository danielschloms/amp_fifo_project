# How to execute the demo program
Make with:

```bash
$ make
```

Execute with:

```bash
$ OMP_NUM_THREADS=64 bin/Demo
```

If one would want to see a demo of the lock-based queue (single lock):

```bash
$ OMP_NUM_THREADS=64 bin/LockDemo
```
