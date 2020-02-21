
CC=gcc

#CFLAGS=-c -g -I /usr/include/mysql
#LDFLAGS=-g

CXXFLAGS = -O2 -I.. -march=native -std=c++11
CFLAGS= $(CXXFLAGS)  -std=gnu99
LDFLAGS=-O2 -lgmp

SOURCES=lyra2re.c lyra2v2.c Lyra2.c lyra2z.c Lyra2-z.c Sponge.c allium.c \
	c11.c x11.c x12.c x13.c hsr14.c sm3.c x14.c x15.c x17.c \
	x22i.c SWIFFTX/SWIFFTX.c \
	blake.c blakecoin.c blake2s.c jha.c keccak.c lbry.c tribus.c exosis.c \
	deep.c fresh.c groestl.c neoscrypt.c nist5.c quark.c qubit.c skein.c skein2.c \
	bitcore.c timetravel.c x11evo.c x16r.c x16s.c xevan.c bastion.c hmq17.c sonoa.c \
	bmw.c luffa.c pentablake.c vitalium.c whirlpool.c whirlpoolx.c zr5.c \
	scrypt.c scryptn.c sha256.c sha256t.c \
	yescrypt.c yescrypt-opt.c sha256_Y.c \
	a5a.c a5amath.c m7m.c magimath.cpp velvet.c \
	hive.c pomelo.c hex.c  \
	phi.c phi2.c polytimos.c rainforest.c skunk.c sib.c veltor.c gost.c aergo.c lbk3.c \
	merkletree/merkle-tree.cpp merkletree/merkle-tree.hpp \
	merkletree/mtp.h merkletree/mtp.cpp \
	argon2ref/argon2.c argon2ref/blake2b.c argon2ref/blake2-impl.h \
	argon2ref/blamka-round-ref.h  argon2ref/core.h  \
	argon2ref/encoding.h  argon2ref/thread.c  \
	argon2ref/argon2.h  argon2ref/blake2.h    \
	argon2ref/blamka-round-opt.h  argon2ref/core.c   \
	argon2ref/encoding.c  argon2ref/ref.c   argon2ref/thread.h 


OBJECTS=$(SOURCES:%.c=%.o) $(SOURCES:%.cpp=%.o)
OUTPUT=libalgos.a

all: $(SOURCES) $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	ar rc $@ $(OBJECTS)
	touch ../stratum.cpp

.cpp.o:
	$(CC) $(CXXFLAGS) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

#	$(CC) $(CFLAGS) -std=gnu99 -Wno-pointer-sign -Wno-pointer-to-int-cast -funroll-loops -fvariable-expansion-in-unroller -fmerge-all-constants -fbranch-target-load-optimize2 -fsched2-use-superblocks -falign-loops=16 -falign-functions=16 -falign-jumps=16 -falign-labels=16 -Ofast -flto -fuse-linker-plugin -ftree-loop-if-convert-stores -DUSE_ASM -pg $<

clean:
	rm -f *.o
	rm -f ar2/*.o
	rm -f blake2/*.o
	rm -f SWIFFTX/*.o
	rm -f argon2ref/*.o
	rm -f merkletree/*.o
