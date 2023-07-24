FROM ubuntu:focal

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get clean 
RUN apt-get update
RUN apt-get install -y dialog
RUN apt-get install -y apt-utils
RUN apt-get install -y nano
RUN apt-get install -y build-essential

COPY / $HOME/
EXPOSE 80
CMD ["sleep","infinity"]