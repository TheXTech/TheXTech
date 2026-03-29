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
import os

# roughly, an enum of header bytes for events (*control channel*)
# HEADER_CLIENT_JOIN = 1
# HEADER_CLIENT_LOSS = 2
# HEADER_TEXT_EVENT = 3
HEADER_FRAME_COMPLETE = 4
# HEADER_YOU_ARE = 5
# HEADER_RAND_SEED = 6
# HEADER_TIME_IS = 7
HEADER_LEFT_ROOM = 8

HEADER_ROOM_KEY = 9
HEADER_ROOM_INFO = 10

HEADER_CREATE_ROOM = 11
HEADER_JOIN_ROOM = 12

HEADER_DATA_CHANNEL = 13
HEADER_ACK = 14

HEADER_PUT_SESSION = 15
HEADER_GET_SESSION = 16

# an enum of header bytes for messages (*data channel*)
MESSAGE_FRAME_BEGIN = 32 # meta-message: the following messages belong to the named frame
MESSAGE_ADD_CLIENT = 33
MESSAGE_DROP_CLIENT = 34
MESSAGE_FRAME_END = 35 # meta-message (TCP only): the previously named frame is now complete
MESSAGE_TRANSMIT_START = 36 # meta-message (UDP only): transmission is only valid if this frame has passed, generally acknowledges receipt of messages up to the named frame

def display_room_key(room_key):
    if len(room_key) != 4:
        return ''

    room_key_int = (room_key[0] << 24) + (room_key[1] << 16) + (room_key[2] << 8) + (room_key[3] << 0)

    if (room_key_int & (192 << 24)):
        return ''

    chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ346789'

    letters = ''.join([chars[(room_key_int >> off) % 32] for off in (25, 20, 15, 10, 5, 0)])
    return letters[:3] + '-' + letters[3:]

class Connection:
    def __init__(self, server, conn, address):
        self.server = server
        self.conn = conn
        self.data_conn = None
        self.address = address

        self.session_key = b'\0\0\0\0'

        # session config state
        self.providing_session = False
        self.receiving_session = False
        self.session_received = 0

        # room state
        self.room = None
        self.id = 0
        self.acked_to = 0
        self.acked_frame = -1
        self.double_acked_frame = -60
        self.warned_frame = 0

        self.tcp_send_start = 0
        self.tcp_send_end = 0
        self.tcp_sent_frame = -1

        self.tcp_frame_in_progress = []
        self.tcp_frame_no = -1

        # UDP state
        self.udp_dest = None
        self.udp_recv = []
        self.udp_send = b''

    def set_room(self, room, id):
        self.providing_session = False
        self.receiving_session = False
        self.session_received = 0

        self.room = room
        self.id = id
        self.acked_to = -1
        self.acked_frame = -1
        self.double_acked_frame = -60
        self.warned_frame = 0

        self.tcp_send_start = 0
        self.tcp_send_end = 0
        self.tcp_sent_frame = -1

    def read_in_room(self):
        # special case for when uploading a session object
        if self.providing_session:
            try:
                self.room.session_config += self.conn.recv(self.room.session_config_length - len(self.room.session_config))
            except BlockingIOError:
                pass

            if len(self.room.session_config) < self.room.session_config_length:
                return

            # acknowledge receipt of session config
            self.conn.sendall(bytes([HEADER_GET_SESSION]))
            self.receiving_session = False

        # conn is the socket object
        try:
            header = self.conn.recv(1)
        except BlockingIOError:
            return

        if len(header) != 1:
            self.room.kill(self)
            return

        if header[0] == HEADER_PUT_SESSION:
            session_config_length = self.conn.recv(4)
            if len(session_config_length) != 4 or self.room.session_config_length != 0:
                self.room.kill(self)
                return

            self.providing_session = True
            self.room.session_config_length = (session_config_length[0] << 24) + (session_config_length[1] << 16) + (session_config_length[2] << 8) + (session_config_length[3] << 0)
            return self.read_in_room()

        if header[0] == HEADER_GET_SESSION:
            self.receiving_session = True
            self.session_received = -1
            return

        try:
            header += self.conn.recv(3)
        except BlockingIOError:
            pass
        if len(header) != 4:
            self.room.kill(self)
            return

        if header == b'\0\0\0\0':
            self.room.unregister(self)

            self.room = None
            self.server.register(self)

            self.conn.sendall(bytes([HEADER_LEFT_ROOM]))
            self.data_conn.close()
            self.data_conn = None

            return

    def read_data(self):
        # conn is the socket object
        while True:
            try:
                message = self.data_conn.recv(4)
            except BlockingIOError:
                break

            if len(message) != 4:
                self.room.kill(self)
                return

            if message[0] == MESSAGE_FRAME_BEGIN or message[0] == MESSAGE_FRAME_END:
                if self.tcp_frame_no > self.acked_frame:
                    self.acked_frame = self.tcp_frame_no

                    for msg in self.tcp_frame_in_progress:
                        self.room.enqueue_text_event(self.tcp_frame_no, self.id, msg)
                        print("Got 1 new event from TCP")

                    self.tcp_frame_in_progress = []

                self.tcp_frame_no = -1

            frame_no = message[1] * 256 * 256 + message[2] * 256 + message[3]
            if message[0] == MESSAGE_FRAME_BEGIN:
                self.tcp_frame_no = frame_no
                if frame_no - 1 > self.acked_frame:
                    self.acked_frame = frame_no - 1
                else:
                    if frame_no > self.acked_frame:
                        self.acked_frame = frame_no
            elif self.tcp_frame_no > self.acked_frame:
                self.tcp_frame_in_progress.append(message)

    def read_data_udp(self, msg):
        if len(msg) % 4 != 0:
            self.room.kill(self)
            return

        udp_frame_index = -1
        for i in range(0, len(msg), 4):
            message = msg[i : i + 4]

            if message[0] == MESSAGE_FRAME_BEGIN or message[0] == MESSAGE_FRAME_END:
                udp_frame_index = message[1] * 256 * 256 + message[2] * 256 + message[3]
                if message[0] == MESSAGE_FRAME_END and udp_frame_index > self.acked_frame:
                    self.acked_frame = udp_frame_index
                elif udp_frame_index - 1 > self.acked_frame:
                    self.acked_frame = udp_frame_index - 1
            else:
                if udp_frame_index > self.acked_frame:
                    self.room.enqueue_text_event(udp_frame_index, self.id, message)
                    print("Got 1 new event from UDP")

    def set_data_conn(self, data_conn):
        if self.data_conn:
            return

        self.data_conn = data_conn
        if self.room:
            self.room.selector.register(self.data_conn, selectors.EVENT_READ, self.read_data)

    def read_in_lobby(self):
        header = self.conn.recv(1)
        if len(header) != 1:
            self.server.kill(self)
            return

        if header[0] == HEADER_ROOM_INFO:
            room_key = self.conn.recv(4)
            if len(room_key) != 4:
                self.server.kill(self)
                return

            room = self.server.load_room(room_key)
            if not room or room.room_key != room_key:
                self.conn.sendall(bytes([HEADER_ROOM_INFO]) + room_key + b'\0\0\0\0\0\0\0\0\0\0\0\0')
                return

            self.conn.sendall(bytes([HEADER_ROOM_INFO]) + room.room_key + room.room_info)
        elif header[0] == HEADER_JOIN_ROOM:
            room_key = self.conn.recv(4)
            if len(room_key) != 4:
                self.server.kill(self)
                return

            room = self.server.load_room(room_key)
            if not room:
                self.conn.sendall(bytes([HEADER_ROOM_KEY]) + b'\0' * 12)
                return

            self.server.unregister(self)
            room.add(self)
        elif header[0] == HEADER_CREATE_ROOM:
            room_info = self.conn.recv(12)

            room = self.server.create_room(room_info)
            if not room:
                self.conn.sendall(bytes([HEADER_ROOM_KEY]) + b'\0' * 12)
                return

            self.server.unregister(self)
            room.add(self)
        elif header[0] == HEADER_DATA_CHANNEL:
            # unregister as a normal client
            self.server.unregister(self)
            self.server.unregister_session(self.session_key)
            self.session_key = b'\0\0\0\0'

            # transfer the conn to the correct client (if it exists)
            session_key = self.conn.recv(4)
            main_client = self.server.find_session(session_key)
            if main_client:
                main_client.set_data_conn(self.conn)

            self.conn = None

    def read(self):
        if self.room is not None:
            self.read_in_room()
            return

        self.read_in_lobby()

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

        self.room_info = b'\0\0\0\0\0\0\0\0\0\0\0\0'

        # room state
        self.frame_no = 0
        self.session_config_length = 0
        self.session_config = bytes()
        self.sent_history = bytes()

        # this is a list of clients added by the main thread
        self.added_clients = []
        self.kill_timer = 0
        self.killed = False
        self.kill_lock = threading.Lock()

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
        if len(event_text) != 4:
            print(f"Warning: invalid event length {len(event_text)}, 4 expected.")
            return
        if event_text[1] != id:
            # figure this out...
            print(f"Warning: invalid event client ID {event_text[1]}, {id} expected.")
            return

        # add to queue
        heapq.heappush(self.event_queue, (frame_no, event_text))

    def unregister(self, client):
        # free client's network state
        try:
            self.selector.unregister(client.conn)
        except KeyError:
            pass
        except ValueError:
            pass

        try:
            self.selector.unregister(client.data_conn)
        except KeyError:
            pass
        except ValueError:
            pass

        # get client's ID
        dead_id = client.id

        # if the client is not in the correct location in the client list, return
        if not self.clients[dead_id] is client:
            return

        # create loss event and add to queue
        serialized_event = bytes([MESSAGE_DROP_CLIENT, 255, 0, dead_id])
        heapq.heappush(self.event_queue, (self.frame_no, serialized_event))

        # remove reference to this client
        self.clients[dead_id] = None

        # mark client as newly dead (can't re-use ID this frame)
        self.newly_freed_clients.add(dead_id)

    def kill(self, client):
        self.unregister(client)
        client.conn.close()
        if client.data_conn:
            client.data_conn.close()

        if client.providing_session and len(self.session_config) < self.session_config_length:
            self.session_config = bytes()
            self.session_config_length = 0

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
        serialized_event = bytes([MESSAGE_ADD_CLIENT, 255, 0, new_id])
        heapq.heappush(self.event_queue, (self.frame_no, serialized_event))

        # tell the client who they are, what the current time is, and the room's key (may throw)
        if client.session_key == b'\0\0\0\0':
            self.server.create_session(client)

        client.conn.sendall(bytes([
            HEADER_ROOM_KEY, self.room_key[0], self.room_key[1], self.room_key[2], self.room_key[3],
            new_id,
            self.frame_no // (256 * 256), (self.frame_no // 256) % 256, self.frame_no % 256,
            client.session_key[0], client.session_key[1], client.session_key[2], client.session_key[3],
            ]))
        client.acked_to = 0
        client.acked_frame = -1
        client.double_acked_frame = -60
        client.warned_frame = self.frame_no
        client.tcp_sent_frame = -1

        # register this socket's events to go to the client
        self.selector.register(client.conn, selectors.EVENT_READ, client.read)
        if client.data_conn:
            self.selector.register(client.data_conn, selectors.EVENT_READ, client.read_data)

    def transmit(self, client):
        # send UDP
        if client.udp_send and client.udp_dest:
            self.server.udp_socket.sendto(client.udp_send, client.udp_dest)
            # print('Sending', list(client.udp_send))

        # send TCP
        if client.data_conn and (client.tcp_send_start < client.tcp_send_end or (not client.udp_dest and client.tcp_send_end == len(self.sent_history))):
            try:
                client.tcp_send_start += client.data_conn.send(self.sent_history[client.tcp_send_start:client.tcp_send_end])
                if client.tcp_send_start == len(self.sent_history):
                    client.data_conn.sendall(bytes([MESSAGE_FRAME_END, self.frame_no // (256 * 256), (self.frame_no // 256) % 256, self.frame_no % 256]))
                    client.tcp_send_end = 0
            except BlockingIOError:
                pass
            except ConnectionResetError:
                pass
            except BrokenPipeError:
                pass

    def process_frame(self):
        # print(f"Debug: starting processing for frame {self.frame_no}")

        # thread-safe way of adopting added clients queue
        added_clients = self.added_clients
        self.added_clients = []

        for client in added_clients:
            try:
                self.add(client)
            except Exception as e:
                print(e)
                self.kill(client)

        # sync all network events
        events = self.selector.select(0)

        for key, mask in events:
            # mask is the event that was triggered (always selectors.EVENT_READ)
            callback = key.data
            try:
                callback()
            except Exception as e:
                print(e)
                self.kill(callback.__self__)

        # sync UDP events
        for client in self.clients:
            if client is None:
                continue

            got = client.udp_recv
            client.udp_recv = []

            for msg in got:
                client.read_data_udp(msg)

        # don't run anything until we have a session_config
        if not self.session_config or len(self.session_config) < self.session_config_length:
            return

        # skip the frame if nobody is connected
        if not self.newly_freed_clients:
            has_any_client = False

            for client in self.clients:
                if client is None:
                    continue

                has_any_client = True
                break

            if not has_any_client:
                return

        # increment the frame number
        self.frame_no += 1

        # combine all events from this frame
        serialized_events = bytes()

        while self.event_queue and self.event_queue[0][0] <= self.frame_no:
            event_to_send = heapq.heappop(self.event_queue)

            event_frame = event_to_send[0]
            serialized_event = event_to_send[1]

            serialized_events += serialized_event

            # debug info
            # if serialized_event[0] == HEADER_CLIENT_JOIN:
            #     print(f"Debug: sending CLIENT_JOIN ({serialized_event[1]})")
            # elif serialized_event[0] == HEADER_CLIENT_LOSS:
            #     print(f"Debug: sending CLIENT_LOSS ({serialized_event[1]})")
            print(f"Debug: sending TEXT_EVENT (from {serialized_event[1]}): {list(serialized_event)}")

            if event_frame != self.frame_no:
                print(f"Warning: sent an event for frame {event_to_send[0]} on frame {self.frame_no}. This is not a normal lag situation.")

        # add the frame-begin event
        if serialized_events:
            serialized_events = bytes([MESSAGE_FRAME_BEGIN, self.frame_no // (256 * 256), (self.frame_no // 256) % 256, self.frame_no % 256]) + serialized_events

        # actually send the frame's events to all client sockets
        self.sent_history += serialized_events

        for client in self.clients:
            if client is None:
                continue

            if client.receiving_session:
                if client.session_received == -1:
                    client.conn.sendall(bytes([HEADER_PUT_SESSION,
                        (self.session_config_length // (256 * 256 * 256)) % 256,
                        (self.session_config_length // (      256 * 256)) % 256,
                        (self.session_config_length //             (256)) % 256,
                        (self.session_config_length                     ) % 256]))
                    client.session_received = 0

                try:
                    client.session_received += client.conn.send(self.session_config[client.session_received:])
                except BlockingIOError:
                    pass

                if client.session_received == self.session_config_length:
                    client.receiving_session = False
                else:
                    continue

            if client.acked_to != -1 and (client.data_conn or client.udp_dest):
                # adjust sent_to based on acked frame
                if client.udp_dest and client.acked_frame >= 0:
                    while client.acked_to + 4 <= len(self.sent_history):
                        if self.sent_history[client.acked_to] == MESSAGE_FRAME_BEGIN and \
                                self.sent_history[client.acked_to + 1 : client.acked_to + 4] >= bytes([client.acked_frame // (256 * 256), (client.acked_frame // 256) % 256, client.acked_frame % 256]):
                            break
                        client.acked_to += 4

                MTU_size = 250
                if client.udp_dest and len(self.sent_history) - client.acked_to <= MTU_size - 8:
                    start = max(client.acked_frame, client.tcp_sent_frame + 1)
                    if start >= 0:
                        ack = bytes([MESSAGE_TRANSMIT_START, start // (256 * 256), (start // 256) % 256, start % 256])
                    else:
                        ack = bytes()
                    client.udp_send = ack + self.sent_history[client.acked_to:] + bytes([MESSAGE_FRAME_END, self.frame_no // (256 * 256), (self.frame_no // 256) % 256, self.frame_no % 256])
                # add the tail of the room history to the TCP send queue
                else:
                    if client.tcp_send_start == client.tcp_send_end:
                        client.tcp_send_start = client.acked_to
                    client.tcp_send_end = len(self.sent_history)
                    client.acked_to = client.tcp_send_end
                    client.tcp_sent_frame = self.frame_no
                    client.udp_send = bytes()

                self.transmit(client)

            # warn clients if they are falling behind (in the future, this will happen based on whether the UDP MTU has been exceeded)
            if self.frame_no - max(client.acked_frame, client.warned_frame) > 20:
                # print('Warning client of catchup', self.frame_no, client.acked_frame, client.warned_frame)
                try:
                    client.conn.sendall(bytes([HEADER_FRAME_COMPLETE, self.frame_no // (256 * 256), (self.frame_no // 256) % 256, self.frame_no % 256]))
                except BlockingIOError:
                    pass
                except ConnectionResetError:
                    pass
                except BrokenPipeError:
                    pass
                client.warned_frame = self.frame_no

            # TCP mode: double-ack clients to let them know their latency
            if client.acked_frame - client.double_acked_frame > 60:
                print('Client latency appears to be', self.frame_no - client.acked_frame)
                try:
                    client.conn.sendall(bytes([HEADER_ACK, client.acked_frame // (256 * 256), (client.acked_frame // 256) % 256, client.acked_frame % 256]))
                except BlockingIOError:
                    pass
                except ConnectionResetError:
                    pass
                except BrokenPipeError:
                    pass
                client.double_acked_frame = client.acked_frame

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
                # for i in range(2):
                #     time.sleep((self.frame_length - proc_time) / 2)

                #     for client in self.clients:
                #         if client is None:
                #             continue
                #         self.transmit(client)


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

        self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp_socket.bind((bind_address, bind_port))
        self.udp_socket.setblocking(False)
        self.selector.register(self.udp_socket, selectors.EVENT_READ, self.read_udp)

        # dictionary of loaded rooms
        self.loaded_rooms = {}
        self.room_insertion_lock = threading.Lock()

        # dictionary of active sessions
        self.sessions = {}
        self.session_kill_list = [] # eventually, add a timeout list for unexpectedly disconnected sessions here
        self.sessions_lock = threading.Lock()

        # dictionary of UDP connections to clients (FIXME: need to timeout and deallocate these when client sessions are killed)
        self.udp_connections = {}

        # directory to store saved rooms
        self.room_directory = '/tmp/py'
        os.makedirs(self.room_directory, exist_ok=True)

        # FIXME: add some rate-limiting system soon

    def insert_room(self, new_room):
        with self.room_insertion_lock:
            if not new_room.room_key in self.loaded_rooms:
                self.loaded_rooms[new_room.room_key] = new_room
                threading.Thread(target=new_room.frame_loop, daemon=True).start()

    def create_room(self, room_info):
        while True:
            room_key = bytes([random.randint(0, 63), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)])
            display_key = display_room_key(room_key)

            if room_key == b'\0\0\0\0':
                continue

            try:
                open(self.room_directory + display_key, 'x').close()
            except FileExistsError:
                pass
            else:
                break

        print("Creating room", display_key)

        new_room = Room(self, room_key)
        new_room.room_info = room_info
        self.insert_room(new_room)

        return new_room

    def create_session(self, client):
        with self.sessions_lock:
            while True:
                session_key = bytes([random.randint(128, 255), random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)])

                if session_key in self.sessions:
                    continue

                break

            self.sessions[session_key] = client
            client.session_key = session_key

    def find_session(self, session_key):
        with self.sessions_lock:
            if session_key in self.sessions:
                return self.sessions[session_key]

        return None

    def unregister_session(self, session_key):
        with self.sessions_lock:
            if session_key in self.sessions:
                del self.sessions[session_key]

    def load_room(self, room_key):
        display_key = display_room_key(room_key)

        try:
            room = self.loaded_rooms[room_key]

            with room.kill_lock:
                if not room.killed:
                    room.kill_timer = 0
                    return room

            room = None
        except KeyError:
            pass

        display_key = display_room_key(room_key)

        if not display_key or not os.path.exists(self.room_directory + display_key) or os.path.getsize(self.room_directory + display_key) == 0:
            return None

        loading_room = Room(self, room_key)
        loading_room.load_from_file()

        self.insert_room(loading_room)
        return self.loaded_rooms[room_key]

    def accept(self):
        # create and set up the new socket
        conn, address = self.socket.accept()
        conn.setblocking(False)
        conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

        # add a client
        new_client = Connection(self, conn, address)

        # wait for events on this client
        self.selector.register(new_client.conn, selectors.EVENT_READ, new_client.read)

    def read_udp(self):
        data, addr = self.udp_socket.recvfrom(256)

        if addr in self.udp_connections and data[0] < 128:
            self.udp_connections[addr].udp_recv.append(data)
        elif len(data) == 4 and not addr in self.udp_connections:
            print(data, addr)
            client = self.find_session(data)
            if client:
                self.udp_connections[addr] = client
                client.udp_dest = addr
                client.udp_recv = []
                client.udp_send = b''

    def unregister(self, client):
        try:
            self.selector.unregister(client.conn)
        except KeyError:
            pass
        except ValueError:
            pass

    def kill(self, client):
        self.unregister(client)
        client.conn.close()

        if client.data_conn:
            client.data_conn.close()

    def process(self):
        # sync all network events
        events = self.selector.select(None)

        for key, mask in events:
            # mask is the event that was triggered (always selectors.EVENT_READ)
            callback = key.data
            try:
                callback()
            except Exception as e:
                print(e)
                # don't kill self on failed accept, just ignore
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
