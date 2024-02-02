.PHONY: build-docker run-docker build

build-docker:
	docker build -t scl_compiler_docker .

run-docker:
	docker run -it -v $(PWD):/app scl_compiler_docker

GEN_FOLDER=generated

build:
	mkdir -p $(GEN_FOLDER)
	flex -o $(GEN_FOLDER)/lex.yy.c lex.l
	gcc -o $(GEN_FOLDER)/prog $(GEN_FOLDER)/lex.yy.c -lfl

clean:
	rm -rf $(GEN_FOLDER)