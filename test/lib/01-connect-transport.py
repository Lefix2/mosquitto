#!/usr/bin/env python3

# Test whether a client can connect over a socket provided by the application
# with mosquitto_connect_transport().

# The client should open a TCP connection to port 1888 itself and hand it over
# in its transport open callback, then connect with keepalive=60, clean session
# set, and client id 01-connect-transport.
# The test will send a CONNACK message to the client with rc=0. Upon receiving
# the CONNACK and verifying that rc=0, the client should send a DISCONNECT
# message. If rc!=0, the client should exit with an error.

from mosq_test_helper import *

def do_test(conn, data):
    connect_packet = mqtt_packets.gen_connect("01-connect-transport")
    connack_packet = mqtt_packets.gen_connack(rc=0)
    disconnect_packet = mqtt_packets.gen_disconnect()

    mosq_test.do_receive_send(conn, connect_packet, connack_packet, "connect")
    mosq_test.expect_packet(conn, "disconnect", disconnect_packet)

mosq_test.client_test(Path("c", mosq_test.get_build_type(), "01-connect-transport.exe"), [], do_test, None)
