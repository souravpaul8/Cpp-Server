FROM ubuntu:focal

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get clean 
RUN apt-get update
RUN apt-get install -y dialog
RUN apt-get install -y apt-utils
RUN apt-get install -y nano
RUN apt-get install -y build-essential
RUN apt-get install -y libcpprest-dev
RUN apt-get install -y libboost-all-dev
RUN apt-get install -y libssl-dev
RUN apt-get install -y curl

COPY / $HOME/src
EXPOSE 80
CMD ["sleep","infinity"]