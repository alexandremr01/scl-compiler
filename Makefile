CC        := gcc
CFLAGS    := -Wextra -Wundef -Wpointer-arith -Wcast-align -std=c11 -D_XOPEN_SOURCE=500 -Werror=vla
LDFLAGS   := -lfl
BUILD     := ./bin

SOURCEDIR := $(CURDIR)/src
GENERATEDDIR := $(SOURCEDIR)/generated

SOURCEFILES    := main.c frontend/semantic_analysis.c \
					datastructures/ast.c datastructures/ir.c datastructures/symbolic_table.c \
					datastructures/datatypes.c \
					backend/codegen.c backend/register_mapping.c backend/linker.c \
					asmWriter/riscv.c asmWriter/binary.c \
					generated/lex.yy.c generated/syntax.tab.c 

GENERATEDFILES := *.yy.c *.tab.* *.output

SRC :=  $(foreach file, $(SOURCEFILES), $(addprefix $(SOURCEDIR)/, $(file)))
GEN :=  $(foreach file, $(GENERATEDFILES), $(addprefix $(SOURCEDIR)/, $(file)))

.PHONY: build-docker run-docker all build test

build-docker:
	docker build -t scl_compiler_docker .

run-docker:
	docker run -it -v $(PWD):/app scl_compiler_docker

all: build

build:
	@mkdir -p $(BUILD) $(GENERATEDDIR)
	flex -o $(GENERATEDDIR)/lex.yy.c $(SOURCEDIR)/frontend/lex.l
	bison --debug -o $(GENERATEDDIR)/syntax.tab.c  -t -v -d -Wcounterexamples $(SOURCEDIR)/frontend/syntax.y
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(BUILD)/sclc
	$(CC) $(CFLAGS) $(SOURCEDIR)/bin2vhdl.c -o $(BUILD)/bin2vhdl

debug: CFLAGS += -g -fsanitize=address
debug: all

clean:
	-@rm -rvf $(BUILD) $(GEN) $(GENERATEDDIR) test/*.out

test:
	sh test/1-test-al.sh