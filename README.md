Libudpr
=======

Libudpr implements a protocol designed to provide reliable communication on top of upd (without congestion control). It aims to be lightweight and to provide a good latency (to be measured!). It is also an occasion to experiments ideas on designing software using finite state machine as a building unit.

On the use of finite state machine
----------------------------------

Inspired by this article by Lamport (http://research.microsoft.com/en-us/um/people/lamport/pubs/spec-book-chap.pdf), I am trying to find a way to organize software using state machine as a building unit. This would allow specifying a software's architecture using a language providing abstraction and allowing to think about a subtract of the problem (e.g. TLA+) and to generate code by deriving from this specification. Obviously this goal needs a lot more work but it is funny to discover about it.

Aside that I believe that using finite state machine is a good way to avoid spaghetti code as the program state is encoded in the fsm state instead of being handled in the code, with "if".

Protocol overview
-----------------

Since this protocol will be used in a known context (dieKurve Mock), it does not uses acknoledgement in order to verify that packet has been sent, but rely on a timestamp. If the receiver catch a jump in the messages timestamp, it knows that a message is missing so it ask to resend it. If no message are received for a period of time, the reciever ask the remote to send it's timestamp in order to make sure they are synced.

Program organization
--------------------

Libudpr rely on two to interfaces: one from the protocol to the udp/ip stack, one to the application layer.
	Lib => network iface:
		+ send_data();
		+ register_data_arrived(&on_data_arrived);
	Lib => application:
		+ udpr_register_callbacks(packet_arrived_cb, out_of_sync_cb);
		+ udpr_send();

The two main component of libudpr are the receiver and the sender. The receiver is a fsm handling the incoming packet, parsing them and forwarding them to the proper destination. If it is a control packet, it is sent to the sender, if it is a data packet it is sent to the application. The receiver is executed in a sequential context, one message at the time. The synchronization is made through a lockless queue I wrote. It needs further investigation in order to find if it is completely correct and if the lockless part worth it because it uses (too?) many atomic operations.

The receiver will have three states: synced, missing and no_message. It starts in the synced state. When receiving a message, if the timestamp is correct, it stays here, if not, it moves to the missing state and save the message in a min heap. In missing, when data are received, if the timestamp is ok, we go to synced, else we stay in missing. When the timestamp goes down, we go to no_message, asking the sender to send its current timestamp. Then we move to synced or missing depending on the timestamp we received.

The sender stores a certain amount of packet. It also contains the code handling the control packet since it is responsible of resending the packets it has stored.

