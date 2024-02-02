build-docker:
	docker build -t scl_compiler_docker .

run-docker:
	docker run -it -v $(PWD):/app scl_compiler_docker