# About
In this project I took upon myself the challenge of building a REDIS Server from Scratch. 

# How to Build and Run the Redis Server
1. First you need to clone the directory.
    > `git clone <repo link> <repo name>`

2. To build the REDIS Server, you can run the following commands by opening a terminal in the project directory:
    > First Make sure that a build does not already exist
    ```
    make clean
    ```
    > Now you can run the `make` command in your terminal
    ```
    make
    ```
    > To run the server:
    ```
    make run
    ```

# Tesing the Server
To test the Redis Server, you require a redis client. You can download a command line redis client from the official Redis Website.  
Use this command to check if the redis client cli has been installed: `redis-cli --version`

# Currently supported Mechanisms
Currenlty this Redis Server Project Support the followinng functionalities of the original Redis:  
1. Concurrency: Multiple client connections at the same time
2. Persistence: Can startup the server and provide an rdb file. The server will setup itself with all the data provided in the RDB file. 
3. Replication & Propagation: Multiple Slave Servers can be connected to the Master Server and any SET commands given to the Master Server will be propagated to the Slave servers automatically.  
4. Handshake: Handshake mechanisms as defined in the RESP Protocol for master and slave servers has been implemented
> Note: While basic RDB files are supported, complex rdb files with complex data structures are not supported currently  
> Note: The SET command is non-blocking for the Master Server, but the propagation to the slave servers is asynchronous and blocking.

# Currently supported Commands:
The following commands are currently supported in the redis server.  
1. **ECHO:** Returns the provided message.
2. **PING:** Tests the connection to the server.
3. **SET:** Assigns a value to a key.
4. **GET:** Retrieves the value of a key.
5. **CONFIG:** Gets or sets server configuration.
6. **KEYS:** Lists keys matching a pattern.
7. **INFO:** Provides server information.
8. **REPLCONF:** Used for replication configuration.
9. **PSYNC:** Synchronizes a replica with a master.

> Since this is an ongoing project, I will keep updating the list as I go on. Feel free to reach out to me or point out any bugs if you find any :D