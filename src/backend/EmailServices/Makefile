TARGETS = smtp pop3 echoserver send_mqueue

all: $(TARGETS)

echoserver: echoserver.cc parse_input.cc
	g++ $^ -lpthread -g -o $@

smtp: smtp.cc parse_input.cc
	g++ $^ -std=c++17 -lpthread -g -o $@

pop3: pop3.cc parse_input.cc
	g++ $^ -std=c++17 -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lcrypto -lpthread -g -o $@

send_mqueue: send_mqueue.cc parse_input.cc
	g++ $^ -std=c++17 -lresolv -g -o $@

pack:
	rm -f submit-hw2.zip
	zip -r submit-hw2.zip *.cc *.hpp *.h README Makefile

clean::
	rm -fv $(TARGETS) *~

realclean:: clean
	rm -fv submit-hw2.zip
