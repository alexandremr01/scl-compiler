CC        := gcc
CFLAGS    := -Wextra -Wundef -Wpointer-arith -Wcast-align -std=c11 -D_XOPEN_SOURCE=500 -Werror=vla
LDFLAGS   := -lfl
BUILD     := ./bin

SOURCEDIR := $(CURDIR)/src
GENERATEDDIR := $(SOURCEDIR)/generated

SOURCEFILES    := main.c ast.c ir.c	symbolic_table.c \
					semantic_analysis.c datatypes.c \
					backend/codegen.c backend/register_mapping.c asmWriter/riscv.c \
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
	flex -o $(GENERATEDDIR)/lex.yy.c $(SOURCEDIR)/lex.l
	bison --debug -o $(GENERATEDDIR)/syntax.tab.c  -t -v -d -Wcounterexamples $(SOURCEDIR)/syntax.y
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(BUILD)/sclc

debug: CFLAGS += -g -fsanitize=address
debug: all

clean:
	-@rm -rvf $(BUILD) $(GEN) $(GENERATEDDIR) test/*.out

test:
	sh test/1-test-al.sh