#!/usr/bin/env python
# coding=utf-8
import random
import string

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
    print(operations[:-2] + "]")
    print("$")

def DoAdd(message):
    print(message)

def DoAppend(message):
    print(message)

total_ops = random.randint(1, 100)
total_adds = 0
total_append = 0
total_remove = 0
total_message = 0
for i in range(total_ops):
    op = random.randint(0, 4)
    if op == 0:
        total_message += 1
        total_adds += 1
        Add()
    elif op == 1:
        total_adds -= 1
        Remove()
    elif op == 2:
        if total_adds > 0:
            total_append += 1
            total_message += 1
            Append()
        Append()
    elif op == 3:
        total_adds = 0
        total_append = 0
        total_remove = 0
        Archive()
    elif op == 4:
        View()
DoOperations()
for i in range(total_message):
    DoAdd(''.join(random.sample(string.ascii_letters + string.digits, random.randint(1, 40))))
