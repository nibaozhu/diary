# sudo ./message_queue_receive
# sudo ./message_queue_send

.PHONY: all clean

all:
	cc -g -o message_queue_receive message_queue_receive.c
	cc -g -o message_queue_send message_queue_send.c

clean:
	rm message_queue_receive message_queue_send
