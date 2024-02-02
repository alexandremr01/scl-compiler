CC        := gcc
CFLAGS    := -Wextra -Wundef -Wpointer-arith -Wcast-align -pedantic -std=c11 -D_XOPEN_SOURCE=500 -Werror=vla
LDFLAGS   := -lfl
BUILD=./bin

SOURCEDIR := $(CURDIR)/src

SOURCEFILES  := main.c 

SRC :=  $(foreach file, $(SOURCEFILES), $(addprefix $(SOURCEDIR)/, $(file)))

.PHONY: build-docker run-docker all build test

build-docker:
	docker build -t scl_compiler_docker .

run-docker:
	docker run -it -v $(PWD):/app scl_compiler_docker

all: build

build:
	@mkdir -p $(BUILD)
	flex -o $(SOURCEDIR)/lex.yy.c $(SOURCEDIR)/lex.l
	bison -o $(SOURCEDIR)/syntax.tab.c  -t -v -d -Wcounterexamples $(SOURCEDIR)/syntax.y
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(BUILD)/scl

clean:
	-@rm -rvf $(BUILD) **/**.yy.c test/**.out src/**.tab.* src/**.output

test:
	sh test/1-test-al.sh