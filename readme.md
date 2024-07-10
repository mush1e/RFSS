# Remote File Storage System

## Overview
RFSS (Remote File Storage System) is a robust multithreaded HTTP server written in C++, designed to facilitate secure and efficient storage, retrieval, and management of various file types for users. Leveraging SQLite for data persistence, RFSS aims to provide a scalable platform for file storage with plans to implement advanced features such as file compression, sharing capabilities, user authentication, and role-based access control (RBAC).

## Features (To Be Implemented)
- **Multithreaded HTTP Server**: Utilizes multithreading to handle concurrent HTTP requests efficiently.
- **SQLite Database**:  Supports CRUD operations for storing files and managing user metadata securely.
- **User Authentication**: Implements a secure login system to authenticate and authorize users.
- **Role-Based Access Control (RBAC)**: Defines roles (e.g., owner, client) with granular permissions for file management.
- **File Downloading and Sharing**: Enables users to download their stored files and share them securely.
- **File Compression:**: Implements file compression to optimize storage space and transfer speeds.
- **HTTPS Support**: Enhances security by supporting HTTPS for encrypted data transmission.


## Usage
1. **Building the Server**: Use the provided Makefile to build the server.
2. **Starting the Server**: Run the compiled executable to start the HTTP server.
3. **Accessing the Platform**: Access the server through a web browser or HTTP client at `http://localhost:8080`.


## Building Instructions
- If using the provided Makefile, simply run `make` to compile the server executable.


## Contributing

Contributions to this project are welcome! If you find any issues or have suggestions for improvements, feel free to open an issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

