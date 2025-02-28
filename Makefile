5g:: main.c mac/mac.c rlc/rlc.c pdcp/pdcp.c ipgen/ipgen.c harq/harq.c loopback/loopback.c
	gcc main.c mac/mac.c mac/mac.h rlc/rlc.c rlc/rlc.h pdcp/pdcp.h pdcp/pdcp.c harq/harq.h harq/harq.c ipgen/ipgen.c ipgen/ipgen.h loopback/loopback.h loopback/loopback.c -o 5g 

clean:
	rm -f 5g
