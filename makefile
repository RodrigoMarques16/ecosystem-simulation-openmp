MAKEFLAGS += --silent

# Compile settings
CC        = g++ -DNTHREADS=$(NTHREADS) -funroll-loops -march=native -flto
CFLAGS    = -std=c++14 -Ofast -fno-exceptions
OPENMP    = -fopenmp -mveclibabi=svml
FILES     = *.cpp
TARGET    = ecosystem
NTHREADS  = 4
# Testing variables
SIZE      = 5x5
INPUT     = tests/input$(SIZE)

# Output locations
TESTS_IN  = tests/input
TESTS_OUT = tests/my_output
SEQ_OUT   = output
PAR_OUT   = output_parallel

all:
	$(CC) $(CFLAGS) $(OPENMP) $(FILES) -o $(TARGET)

debug:
	$(CC) $(CFLAGS) -g $(OPENMP) $(FILES) -o $(TARGET)

seq: 
	$(CC) $(CFLAGS) $(FILES) -o $(TARGET)

par: all

run: seq
	./$(TARGET) < $(INPUT)

prun: par
	./$(TARGET) < $(INPUT)

test: seq
	./$(TARGET) < $(INPUT) > $(TESTS_OUT)/output
	$(CC) $(CFLAGS) $(OPENMP) $(FILES) -o $(TARGET)
	./$(TARGET) < $(INPUT) > $(TESTS_OUT)/output_parallel && cmp $(TESTS_OUT)/output $(TESTS_OUT)/output_parallel

tests: seq
	./$(TARGET) < $(TESTS_IN)5x5     > $(TESTS_OUT)/$(SEQ_OUT)5x5
	./$(TARGET) < $(TESTS_IN)10x10   > $(TESTS_OUT)/$(SEQ_OUT)10x10
	./$(TARGET) < $(TESTS_IN)20x20   > $(TESTS_OUT)/$(SEQ_OUT)20x20
	./$(TARGET) < $(TESTS_IN)100x100 > $(TESTS_OUT)/$(SEQ_OUT)100x100
	./$(TARGET) < $(TESTS_IN)200x200 > $(TESTS_OUT)/$(SEQ_OUT)200x200
	$(CC) $(CFLAGS) $(OPENMP) $(FILES) -o $(TARGET)
	./$(TARGET) < $(TESTS_IN)5x5     > $(TESTS_OUT)/output_parallel5x5     && cmp $(TESTS_OUT)/output5x5     $(TESTS_OUT)/output_parallel5x5
	./$(TARGET) < $(TESTS_IN)10x10   > $(TESTS_OUT)/output_parallel10x10   && cmp $(TESTS_OUT)/output10x10   $(TESTS_OUT)/output_parallel10x10
	./$(TARGET) < $(TESTS_IN)20x20   > $(TESTS_OUT)/output_parallel20x20   && cmp $(TESTS_OUT)/output20x20   $(TESTS_OUT)/output_parallel20x20
	./$(TARGET) < $(TESTS_IN)100x100 > $(TESTS_OUT)/output_parallel100x100 && cmp $(TESTS_OUT)/output100x100 $(TESTS_OUT)/output_parallel100x100
	./$(TARGET) < $(TESTS_IN)200x200 > $(TESTS_OUT)/output_parallel200x200 && cmp $(TESTS_OUT)/output200x200 $(TESTS_OUT)/output_parallel200x200

benchmarkseq: seq
	echo "5x5"
	./$(TARGET) < $(TESTS_IN)5x5             > $(TESTS_OUT)/$(SEQ_OUT)5x5
	echo "10x10"
	./$(TARGET) < $(TESTS_IN)10x10           > $(TESTS_OUT)/$(SEQ_OUT)10x10
	echo "20x20"
	./$(TARGET) < $(TESTS_IN)20x20           > $(TESTS_OUT)/$(SEQ_OUT)20x20
	echo "100x100"
	./$(TARGET) < $(TESTS_IN)100x100         > $(TESTS_OUT)/$(SEQ_OUT)100x100
	echo "100x100_unbal01"
	./$(TARGET) < $(TESTS_IN)100x100_unbal01 > $(TESTS_OUT)/$(SEQ_OUT)100x100_unbal01
	echo "100x100_unbal02"
	./$(TARGET) < $(TESTS_IN)100x100_unbal02 > $(TESTS_OUT)/$(SEQ_OUT)100x100_unbal02
	echo "200x200"
	./$(TARGET) < $(TESTS_IN)200x200         > $(TESTS_OUT)/$(SEQ_OUT)200x200

benchmark: par
	$(CC) $(CFLAGS) $(OPENMP) $(FILES) -o $(TARGET)
	echo "5x5"
	./$(TARGET) < $(TESTS_IN)5x5             > $(TESTS_OUT)/$(PAR_OUT)5x5
	echo "10x10"
	./$(TARGET) < $(TESTS_IN)10x10           > $(TESTS_OUT)/$(PAR_OUT)10x10
	echo "20x20"
	./$(TARGET) < $(TESTS_IN)20x20           > $(TESTS_OUT)/$(PAR_OUT)20x20
	echo "100x100"
	./$(TARGET) < $(TESTS_IN)100x100         > $(TESTS_OUT)/$(PAR_OUT)100x100
	echo "100x100_unbal01"
	./$(TARGET) < $(TESTS_IN)100x100_unbal01 > $(TESTS_OUT)/$(PAR_OUT)100x100_unbal01
	echo "100x100_unbal02"
	./$(TARGET) < $(TESTS_IN)100x100_unbal02 > $(TESTS_OUT)/$(PAR_OUT)100x100_unbal02
	echo "200x200"
	./$(TARGET) < $(TESTS_IN)200x200         > $(TESTS_OUT)/$(PAR_OUT)200x200