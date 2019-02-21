# 499-kunpeng
USC CS499 Project  
Name: Kun Peng  
Email: kunpeng@usc.edu  

## Setup Steps
Setup gRPC

	$ sudo apt-get install build-essential autoconf libtool pkg-config
	$ sudo apt-get install libgflags-dev libgtest-dev
	$ sudo apt-get install clang libc++-dev
	$ git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
	$ cd grpc
	$ git submodule update --init
	$ sudo make
	$ sudo make install
	$ cd grpc/third_party/protobuf
	$ sudo make install

Clone 499-kunpeng repo

    $ git clone https://github.com/Rompue/499-kunpeng.git
	$ cd 499-kunpeng
	$ git checkout phaseone

## Compilation instructions
	$ sudo make

## Basic example usage
Run Key-Value storage server and Service Layer server first:  

	$ ./storage_server
	$ ./service

All operations should be ran by a logined user, except `read`  

To **register a user** with `username`: `$ ./chirp --register username`   

To login as `username` and **post a chirp** with `text`: `$ ./chirp --user username --chirp "text"`  

To login as `username` and **post a chirp** with `text` as a **reply** to a chirp with `id`: `$ ./chirp --user username --chirp "text" --reply id`

To login as `username` and **follow another user** with `username2`: `$ ./chirp --user username --follow username2`   

To login as `username` and start **monitoring** following users: `$ ./chirp --user username --monitor`  
To stop monitoring, press Ctrl + C (terminating process in bash/terminal)

To **read a chirp** with `chirpid` without login: `$ ./chirp --read chirpid`   

#### All other use of the command line tool may result in error or even undefined behavior


