
all:
	gcc -o worker worker.c
	gcc -o server server.c
	gcc -o client client.c
	chmod +x script.sh
