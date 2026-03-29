# jsplit

Recreation of the Linux/GNU util 'split' used for splitting files.

Recreated by Jan Teskeredzic for learning purposes.


# Build

To build gcc is required:

```bash
sudo apt install gcc
```

```bash
gcc -Wall jsplit.c -o jsplit
```


# Usage

Execution flags:

-b (byte splitting size)
-kb (kilobyte splitting size)
-n (end each file with \n)
-o sampleBatch (provide output batching name, default is 'autobatch+index')
-l 1024 (limit amount of bytes written in total)
-lf 4 (limit amount of files created in total)
-y or -Y (yes flag, skips confirmation to execute)


```bash
./jsplit -b 2048 -n -o filebatch
```
