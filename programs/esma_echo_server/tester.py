#!/usr/bin/python3
import sys
import time
import random
import string
import socket
import threading


def usage():
    print("use ./teseter.py PORT N_CLIENTS, where PORT is port number and N_CLIENTS number of clients")
    print("for ex. ./tester.py 1122 128")
    sys.exit(1)


def client_func(name):
    print("thread {} has been started.".format(name))
    while True:
        exec_cnt = random.randint(1, 10)
        for i in range(exec_cnt):
            print("Thread {} start working with client".format(name))
            sock = socket.socket()
            sock.connect(('localhost', int(sys.argv[1])))

            #check connect
            time.sleep(0.1)
            hello = "hello\n"

            try:
                sock.send(hello.encode())
                ret = sock.recv(len(hello))
                if ret == hello.encode():
                    print("Thread {} connect establishment succesfully".format(name))
            except:
                print("Thread {} connection again".format(name))
                break

            dg_cnt = random.randint(0, 100)
            print("Thread {} start working with socket. Number of iterations {}".format(name, dg_cnt))
            for j in range(dg_cnt):
                dg_size = random.randint(0, 12000)
                letters = string.ascii_lowercase
                dg = ''.join(random.choice(letters) for i in range(dg_size))
                dg += '\n'

                sock.send(dg.encode())

                ret = sock.recv(len(dg))
                if ret != dg.encode():
                    print("mismatch: send {}; recv {}.".format(dg.encode(), recv))

            sock.close()


if __name__ == "__main__":
    if len(sys.argv) != 3:
        usage()

    for i in range(0, int(sys.argv[2])):
        thread = threading.Thread(target=client_func, args=(i, ))
        thread.start()
