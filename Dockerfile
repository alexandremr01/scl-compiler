FROM ubuntu:22.04

RUN apt-get update && apt-get install -y build-essential flex bison

WORKDIR /app

ENTRYPOINT bash