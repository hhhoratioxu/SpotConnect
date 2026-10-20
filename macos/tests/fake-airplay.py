#!/usr/bin/env python3
# Copyright (C) 2026 hhhoratioxu. MIT. Local pipe fixture; never contacts a device.
import os
import select
import sys
import time

args = sys.argv[1:]
fifo = args[args.index('--cmdpipe') + 1]
fd = os.open(fifo, os.O_RDWR | os.O_NONBLOCK)
os.set_blocking(0, False)
directory = os.environ['HORI_FAKE_CAPTURE']
mode = os.environ.get('HORI_FAKE_MODE', 'normal')
print('[STATUS] connected', flush=True)
print('Private fixture text must never be forwarded by the adapter', flush=True)
commands = b''
buffered = False
deadline = time.monotonic() + 12
with open(directory + '/pcm', 'ab', buffering=0) as pcm, open(directory + '/commands', 'ab', buffering=0) as control:
    while time.monotonic() < deadline:
        ready, _, _ = select.select([0, fd], [], [], .02)
        if 0 in ready:
            data = os.read(0, 16384)
            if not data:
                break
            pcm.write(data)
            if mode == 'error':
                print('[STATUS] error code=fixture_failure', flush=True)
                break
            if not buffered:
                print('[STATUS] audio buffered_ms=92', flush=True)
                buffered = True
        if fd in ready:
            commands += os.read(fd, 4096)
            while b'\n' in commands:
                command, commands = commands.split(b'\n', 1)
                control.write(command + b'\n')
                if command == b'ACTION=START':
                    print('[STATUS] started requested_unix_ms=0 at_unix_ms=0', flush=True)
                elif command == b'ACTION=FLUSH':
                    while select.select([0], [], [], 0)[0]:
                        if not os.read(0, 16384): break
                    buffered = False
                    print('[STATUS] flushed', flush=True)
                elif command == b'ACTION=DISCONNECT':
                    os.close(fd)
                    os.unlink(fifo)  # The real CLI also owns/removes this FIFO.
                    sys.exit(0)
os.close(fd)
