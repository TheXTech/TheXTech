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
# HEADER_ROOM_KEY = 8

class Connection:
    def __init__(self, server, conn, address):
        self.server = server
        self.conn = conn
        self.address = address

        # room state
        self.room = None
        self.id = id
        self.sent_to = -1

    def set_room(self, room, id):
        self.room = room
        self.id = id
        self.sent_to = -1

    def read_in_room(self):
        # conn is the socket object
        header = self.conn.recv(4)
        if len(header) != 4:
            self.room.kill(self)
            return

        frame_no = header[0] * 256 * 256 + header[1] * 256 + header[2]
        event_length = header[3]

        if event_length > 0:
            event_text = self.conn.recv(event_length)

            if len(event_text) != event_length:
                self.room.kill(self)
                return
        else:
            event_text = bytes()

        self.room.enqueue_text_event(frame_no, self.id, event_text)

    def read(self):
        if self.room is not None:
            self.read_in_room()
            return

class Room:
    def __init__(self, server, room_key):
        # reference to the main server
        self.server = server
        self.frame_length = self.server.frame_length

        # this object will tell us when any network events have occurred for this room's clients
        self.selector = selectors.DefaultSelector()

        # room information
        self.room_key = room_key
        assert(len(room_key) == 4)

        self.engine_hash = b'\0\0\0\0'
        self.asset_hash = b'\0\0\0\0'
        self.content_hash = b'\0\0\0\0'

        # room state
        self.frame_no = 0
        self.sent_history = bytes([HEADER_RAND_SEED, random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)])

        # this is a list of clients added by the main thread
        self.added_clients = []

        # this is a list of clients
        self.clients = []
        self.freed_clients = set()
        self.newly_freed_clients = set()

        # this is the current frame, and a queue of events
        self.event_queue = []

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

    def unregister(self, client):
        # free client's network state
        try:
            self.selector.unregister(client.conn)
        except KeyError:
            pass

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

    def kill(self, client):
        self.unregister(client)
        client.conn.close()

    def add(self, client):
        # find an ID and slot for the client
        if self.freed_clients:
            new_id = min(self.freed_clients)
            self.freed_clients.discard(new_id)
        else:
            new_id = len(self.clients)
            self.clients.append(None)

        # add the client to the room
        client.set_room(self, new_id)
        self.clients[new_id] = client

        # create join event and add to queue
        serialized_event = bytes([HEADER_CLIENT_JOIN, new_id])
        heapq.heappush(self.event_queue, (self.frame_no, serialized_event))

        # tell the client who they are, what the current time is, and the room's key (may throw)
        client.conn.sendall(bytes([HEADER_YOU_ARE, new_id,
            HEADER_TIME_IS, self.frame_no // (256 * 256), (self.frame_no // 256) % 256, self.frame_no % 256,
            # HEADER_ROOM_KEY]) + self.room_key)
            ]))
        client.sent_to = 0

        # register this socket's events to go to the client
        self.selector.register(client.conn, selectors.EVENT_READ, client.read)

    def process_frame(self):
        # print(f"Debug: starting processing for frame {self.frame_no}")

        # thread-safe way of adopting added clients queue
        added_clients = self.added_clients
        self.added_clients = []

        for client in added_clients:
            try:
                self.add(client)
            except Exception:
                self.kill(client)

        # sync all network events
        events = self.selector.select(0)

        for key, mask in events:
            # mask is the event that was triggered (always selectors.EVENT_READ)
            callback = key.data
            try:
                callback()
            except Exception:
                self.kill(callback.__self__)

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
                except ConnectionResetError:
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


class Server:
    def __init__(self, bind_address, bind_port, frame_length):
        self.frame_length = frame_length

        # this object will tell us when any network events have occurred
        self.selector = selectors.DefaultSelector()

        # this is the TCP server socket
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((bind_address, bind_port))
        self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self.socket.listen(100)
        self.socket.setblocking(False)
        self.selector.register(self.socket, selectors.EVENT_READ, self.accept)

        # list of loaded rooms
        self.loaded_rooms = {}

    def create_room(self):
        self.loaded_rooms['\0\0\0\0'] = Room(self, '\0\0\0\0')

        # daemon = True for now, eventually we'll do something nice with "shutdown"
        threading.Thread(target=self.loaded_rooms['\0\0\0\0'].frame_loop, daemon=True).start()

    def accept(self):
        # create and set up the new socket
        conn, address = self.socket.accept()
        conn.setblocking(False)
        conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

        # add a client
        new_client = Connection(self, conn, address)

        # wait for events on this client
        # self.selector.register(new_client.conn, selectors.EVENT_READ, new_client.read)

        # temporary half-measure
        if not self.loaded_rooms:
            self.create_room()

        self.loaded_rooms['\0\0\0\0'].added_clients.append(new_client)

    def unregister(self, client):
        try:
            self.selector.unregister(client.conn)
        except KeyError:
            pass

    def kill(self, client):
        self.unregister(client)
        client.conn.close()

    def process(self):
        # sync all network events
        events = self.selector.select(None)

        for key, mask in events:
            # mask is the event that was triggered (always selectors.EVENT_READ)
            callback = key.data
            try:
                callback()
            except Exception:
                # don't kill self on failed except, just ignore
                if callback.__self__ is not self:
                    self.kill(callback.__self__)

    def main_loop(self):
        while True:
            self.process()


# this is what happens when you run the script directly
if __name__ == '__main__':
    import sys

    server = Server(sys.argv[1] if len(sys.argv) >= 2 else '0.0.0.0', 4305, 0.0156)
    server.main_loop()
