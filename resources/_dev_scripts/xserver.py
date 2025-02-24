#!/usr/bin/python3

# Copyright (C) 2025 ds-sloth
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import selectors
import socket
import time
import heapq
import threading
import random

# roughly, an enum of header bytes for events
HEADER_CLIENT_JOIN = 1
HEADER_CLIENT_LOSS = 2
HEADER_TEXT_EVENT = 3
HEADER_FRAME_COMPLETE = 4
HEADER_YOU_ARE = 5
HEADER_RAND_SEED = 6
HEADER_TIME_IS = 7

class Connection:
    def __init__(self, parent, id, conn, address):
        self.parent = parent
        self.id = id
        self.conn = conn
        self.address = address

        self.sent_to = -1

    def read(self, conn):
        # conn is the socket object
        header = conn.recv(4)
        if len(header) != 4:
            self.parent.kill(self, conn)
            return

        frame_no = header[0] * 256 * 256 + header[1] * 256 + header[2]
        event_length = header[3]

        if event_length > 0:
            event_text = conn.recv(event_length)

            if len(event_text) != event_length:
                self.parent.kill(self, conn)
                return
        else:
            event_text = bytes()

        self.parent.enqueue_text_event(frame_no, self.id, event_text)

class Server:
    def __init__(self, bind_address, bind_port, frame_length):
        # this object will tell us when any network events have occurred
        self.selector = selectors.DefaultSelector()

        # this is the TCP server socket
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((bind_address, bind_port))
        self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self.socket.listen(100)
        self.socket.setblocking(False)
        self.selector.register(self.socket, selectors.EVENT_READ, self.accept)

        # this is a list of clients
        self.clients = []
        self.freed_clients = set()
        self.newly_freed_clients = set()

        # this is the current frame, and a queue of events
        self.frame_length = frame_length
        self.frame_no = 0
        self.event_queue = []

        # this is everything that has been sent, for fast-forwarding newly-connected clients
        self.sent_history = bytes([HEADER_RAND_SEED, random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)])

    def enqueue_text_event(self, frame_no, id, event_text):
        # force event to happen in future (if client has lag)
        if frame_no < self.frame_no:
            frame_no = self.frame_no

        # encode event
        if len(event_text) > 255:
            print(f"Warning: truncating text of length {len(event_text)} to 255 characters.")
            event_text = event_text[:255]

        header = bytes([HEADER_TEXT_EVENT, id, len(event_text)])

        serialized_event = header + event_text

        # add to queue
        heapq.heappush(self.event_queue, (frame_no, serialized_event))

    def kill(self, client, conn):
        # free client's network state
        self.selector.unregister(conn)
        conn.close()

        # get client's ID
        dead_id = client.id

        # if the client is not in the correct location in the client list, return
        if not self.clients[dead_id] is client:
            return

        # create loss event and add to queue
        serialized_event = bytes([HEADER_CLIENT_LOSS, dead_id])
        heapq.heappush(self.event_queue, (self.frame_no, serialized_event))

        # remove reference to this client
        self.clients[dead_id] = None

        # mark client as newly dead (can't re-use ID this frame)
        self.newly_freed_clients.add(dead_id)

    def accept(self, sock):
        # create and set up the new socket
        conn, address = sock.accept()
        conn.setblocking(False)
        conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

        # find an ID and slot for the client
        if self.freed_clients:
            new_id = min(self.freed_clients)
            self.freed_clients.discard(new_id)
        else:
            new_id = len(self.clients)
            self.clients.append(None)

        # create the client object
        new_client = Connection(self, new_id, conn, address)
        self.clients[new_id] = new_client

        print(f"Debug: creating new Connection with ID {new_id}")

        # send the client the current history (this should not be done on the main thread in general)
        new_client.conn.sendall(bytes([HEADER_YOU_ARE, new_id,
            HEADER_TIME_IS, self.frame_no // (256 * 256), (self.frame_no // 256) % 256, self.frame_no % 256]))
        new_client.sent_to = 0

        # register this socket's events to go to the client
        self.selector.register(conn, selectors.EVENT_READ, new_client.read)

        # create join event and add to queue
        serialized_event = bytes([HEADER_CLIENT_JOIN, new_id])
        heapq.heappush(self.event_queue, (self.frame_no, serialized_event))

    def process_frame(self):
        # print(f"Debug: starting processing for frame {self.frame_no}")

        # sync all network events
        events = self.selector.select(0)

        for key, mask in events:
            # mask is the event that was triggered (always selectors.EVENT_READ)
            callback = key.data
            try:
                callback(key.fileobj)
            except Exception:
                self.kill(callback.__self__, key.fileobj)

        # combine all events from this frame
        serialized_events = bytes()

        while self.event_queue and self.event_queue[0][0] <= self.frame_no:
            event_to_send = heapq.heappop(self.event_queue)

            event_frame = event_to_send[0]
            serialized_event = event_to_send[1]

            serialized_events += serialized_event

            # debug info
            if serialized_event[0] == HEADER_CLIENT_JOIN:
                print(f"Debug: sending CLIENT_JOIN ({serialized_event[1]})")
            elif serialized_event[0] == HEADER_CLIENT_LOSS:
                print(f"Debug: sending CLIENT_LOSS ({serialized_event[1]})")
            elif serialized_event[0] == HEADER_TEXT_EVENT:
                print(f"Debug: sending TEXT_EVENT ({serialized_event[1]}): {serialized_event[3:]}")

            if event_frame != self.frame_no:
                print(f"Warning: sent an event for frame {event_to_send[0]} on frame {self.frame_no}. This is not a normal lag situation.")

        # skip the frame if nobody is connected
        if not serialized_events and not self.newly_freed_clients:
            has_any_client = False

            for client in self.clients:
                if client is None:
                    continue

                has_any_client = True
                break

            if not has_any_client:
                return

        # add the frame-end event
        serialized_events += bytes([HEADER_FRAME_COMPLETE, self.frame_no // (256 * 256), (self.frame_no // 256) % 256, self.frame_no % 256])
        # print(f"Debug: sending FRAME_COMPLETE ({self.frame_no})")

        # actually send the frame's events to all client sockets
        self.sent_history += serialized_events

        for client in self.clients:
            if client is None:
                continue

            if client.sent_to != -1:
                try:
                    client.sent_to += client.conn.send(self.sent_history[client.sent_to:])
                except BlockingIOError:
                    pass

        # increment the frame number
        self.frame_no += 1

        # update the free client ID list
        self.freed_clients.update(self.newly_freed_clients)
        self.newly_freed_clients = set()

    def frame_loop(self):
        while True:
            start_time = time.time()
            self.process_frame()
            end_time = time.time()

            proc_time = end_time - start_time

            if(proc_time > self.frame_length):
                print(f'Warning: overlong frame {self.frame_no - 1} ({proc_time}s > {self.frame_length}s)')
            else:
                time.sleep(self.frame_length - proc_time)


class Client:
    def __init__(self, connect_address, connect_port):
        # this is the TCP client socket
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((connect_address, connect_port))
        self.socket.setblocking(True)

        # this is the current frame
        self.frame_no = 0

        # this is a list of text events to be sent for the next frame
        self.text_queue = []

        # this tracks whether CTRL-C has been pressed yet
        self.quit = False

    def process_frame(self):
        # send any enqueued events
        for event_text in self.text_queue:
            if len(event_text) > 255:
                print(f"Warning: truncating text of length {len(event_text)} to 255 characters.")
                event_text = event_text[:255]

            serialized_text = bytes([self.frame_no // (256 * 256), (self.frame_no // 256) % 256, self.frame_no % 256, len(event_text)]) + event_text
            self.socket.sendall(serialized_text)

        self.text_queue = []

        # read events until we hit a frame end
        while True:
            header_byte = self.socket.recv(1)

            if not header_byte:
                self.socket.close()
                self.quit = True
                print(f'Warning: socket closed')
                return

            if header_byte[0] == HEADER_FRAME_COMPLETE:
                frame = self.socket.recv(3)
                frame_no = frame[0] * 256 * 256 + frame[1] * 256 + frame[2]
                break
            elif header_byte[0] == HEADER_CLIENT_JOIN:
                client_no = self.socket.recv(1)[0]
                print(f'Debug: client {client_no} joined on frame {self.frame_no}')
            elif header_byte[0] == HEADER_CLIENT_LOSS:
                client_no = self.socket.recv(1)[0]
                print(f'Debug: client {client_no} left on frame {self.frame_no}')
            elif header_byte[0] == HEADER_TEXT_EVENT:
                client_no, length = self.socket.recv(2)
                event_text = self.socket.recv(length)
                print(f'Info: got event {event_text} from {client_no} on frame {self.frame_no}')

        if frame_no != self.frame_no:
            print(f'Warning: expected frame_no {self.frame_no}, got {frame_no}')
            self.frame_no = frame_no

        self.frame_no += 1

    def ui_thread(self):
        while not self.quit:
            try:
                text = input().strip().encode()
            except KeyboardInterrupt:
                self.quit = True
                break

            if not text:
                continue

            self.text_queue.append(text)


    def frame_loop(self):
        ui_thread = threading.Thread(target=self.ui_thread, daemon=True)
        ui_thread.start()

        try:
            while not self.quit:
                self.process_frame()
        except KeyboardInterrupt:
            self.quit = True
        finally:
            self.socket.close()

# this is what happens when you run the script directly
if __name__ == '__main__':
    import sys

    if len(sys.argv) >= 2 and sys.argv[1] == 'server':
        server = Server(sys.argv[2] if len(sys.argv) >= 3 else 'localhost', 4305, 0.015)
        server.frame_loop()
    else:
        client = Client(sys.argv[1] if len(sys.argv) >= 2 else 'localhost', 4305)
        client.frame_loop()
        print('Program terminated')
