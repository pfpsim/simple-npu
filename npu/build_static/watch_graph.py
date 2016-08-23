#!/usr/bin/env python
# -*- coding: utf-8 -*-

import nnpy

def run():
    import nnpy
    # I'm not 100% sure if PUB-SUB is the right model for this.
    # For sure it will work, and it does make the code simpler on
    # The C++ side because it only has to manage one socket.
    s = nnpy.Socket(nnpy.AF_SP, nnpy.SUB)
    # This may seem like a complicated scheme for subscribe topic
    # names, but this makes the C++ side simpler, and should work
    # Well in general. Nanomessage topic subscriptions just match
    # The leading bytes, so if we just convert the ID to a string
    # we'd still need to pad it with zeros, because "PFPDB1"
    # would also match messages for "PFPDB10". This way we still
    # support 65536 traces at the same time, which should be
    # far more than enough.
    #topic = ("PFPDB" +
             #chr(self.id & 0xFF) +
             #chr((self.id >> 8) & 0xff))
    topic = "PFPDB"
    s.connect("ipc:///tmp/pfpdb-trace")
    s.setsockopt(nnpy.SUB, nnpy.SUB_SUBSCRIBE, topic)

    print("Subscribed to god damn topic " + repr(topic))

    # Now set up the matlab plot.
    import matplotlib.pyplot as plt
    plt.figure()
    plt.ion() # non-blocking

    while True:
        msgs = []
        try:
            while True:
                msgs.append(s.recv(nnpy.DONTWAIT))
        except AssertionError:
            if nnpy.nanomsg.nn_errno() != nnpy.EAGAIN:
                error_msg = nnpy.ffi.string(
                        nnpy.nanomsg.nn_strerror(nnpy.nanomsg.nn_errno()))
                raise RuntimeError("Error in nanomsg recv: " + error_msg)

        for msg_str in msgs:
            print("received: " + msg_str)
            msg_str = msg_str[len("PFPDBXX"):] # Chop off topic prefix

            #msg = PFPSimDebugger_pb2.TracingUpdateMsg()
            #msg.ParseFromString(msg_str)

            x,y = map(float, msg_str.split(","))
            plt.scatter(x, y)

            #if msg.HasField("float_value"):
                #plt.scatter(msg.timestamp, msg.float_value)
            #elif msg.HasField("int_value"):
                #plt.scatter(msg.timestamp, msg.int_value)
            #else:
                #print("Something wrong, no float or int value")

        plt.pause(0.0001) # Run plot window event loop and updates

import multiprocessing

p = multiprocessing.Process(target=run)
p.daemon = True

p.start()

p.join()
