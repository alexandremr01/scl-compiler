CC        := gcc
CFLAGS    := -Wextra -Wundef -Wpointer-arith -Wcast-align -std=c11 -D_XOPEN_SOURCE=500 -Werror=vla
LDFLAGS   := -lfl
BUILD     := ./bin

SOURCEDIR := $(CURDIR)/src

SOURCEFILES    := main.c ast.c ir.c	codegen.c symbolic_table.c \
					semantic_analysis.c datatypes.c \
					register_mapping.c asmWriter/riscv.c \
					lex.yy.c syntax.tab.c 

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
	@mkdir -p $(BUILD)
	flex -o $(SOURCEDIR)/lex.yy.c $(SOURCEDIR)/lex.l
	bison --debug -o $(SOURCEDIR)/syntax.tab.c  -t -v -d -Wcounterexamples $(SOURCEDIR)/syntax.y
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(BUILD)/sclc

debug: CFLAGS += -g -fsanitize=address
debug: all

clean:
	-@rm -rvf $(BUILD) $(GEN) test/*.out

test:
	sh test/1-test-al.sh