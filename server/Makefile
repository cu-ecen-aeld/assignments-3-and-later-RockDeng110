

CC=$(CROSS_COMPILE)gcc
#CFLAGS= -g -Wall -Werror -std=gnu17
CFLAGS= -g -Wall -std=gnu17
#CFLAGS=
OBJS_SERVER=aesdsocket.o
OBJS_CLIENT=socket_client.o
APP_SERVER=aesdsocket
APP_CLIENT=socket_client
APPS=$(APP_SERVER) $(APP_CLIENT)
OBJS=$(OBJS_SERVER) $(OBJS_CLIENT)


all: $(APPS)


$(APP_SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(OBJS_SERVER) -o $(APP_SERVER) 

$(APP_CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) $(OBJS_CLIENT) -o $(APP_CLIENT) 

clean:
	rm $(APP_CLIENT) $(APP_SERVER) $(OBJS_CLIENT) $(OBJS_SERVER)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

default: $(APP_SERVER)
server: $(APP_SERVER)
client: $(APP_CLIENT)
