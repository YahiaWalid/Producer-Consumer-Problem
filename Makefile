#Use GNU compiler
CC = g++ -g

all: compilep producer compilec consumer keys cleans

p:  compilep producer
c:  compilec consumer

compilec: consumer.cpp  
	$(CC) -c consumer.cpp
compilep: producer.cpp  
	$(CC) -c producer.cpp
consumer:consumer.o 
	$(CC) -o consumer consumer.o -ll
producer:producer.o 
	$(CC) -o producer producer.o -ll
keys:
	touch sem_e sem_s sem_n	
cleanc:
	rm -f consumer.o consumer
cleanp:
	rm -f producer.o producer	
cleans:
	./cleanmemsem.sh
clean:	
	rm -f consumer.o consumer producer.o producer sem_e sem_s sem_n	
	./cleanmemsem.sh
