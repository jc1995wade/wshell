objs = obj/wls.o
objs += obj/gfish.o
objs += obj/wcd.o
objs += obj/cmd.o
objs += obj/wchmod.o
objs += obj/history.o
INC:=include
CC:=gcc

bin/wshell:$(objs)
	$(CC) $^ -I$(INC) -o $@

$(objs):obj/%.o:src/%.c
	$(CC) -c $< -I$(INC) -o $@





.PHONY:clean
clean:
	rm $(objs) 
	rm bin/wshell


