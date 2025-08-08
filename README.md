
## What is the SMU?
SMU is a Server Monitoring Utility that transmits over a secure websocket connection various server data such as CPU utilization, RAM and swap file usage, component temperature, fan speed, etc.

## Guides
SMU is represented by three components - [smu-server](server/README.md) (server part), [smu-cli](cli/README.md) (CLI for controlling the server via terminal) and [smu](client/README.md) (client for visualizing and viewing information from the server). The links will take you to the manual for each component

## Installation
Download the 3 packages from the releases - `smu`, `smu-server` and `smu-cli` to install. Install them using the package manager. `smu-server` and `smu-cli` should be installed on one machine, and `smu` (client) should be installed on another machine

## Quick start
Let's assume that you have installed **smu-server** and **smu-cli** on a remote server with IP 1.2.3.4 (replace with your server's IP) and connected via **ssh** as **root**, and you also have **smu** installed on the device from which you will connect to the server. To begin, generate a certificate and private key with the command:
```
smu-cli --keygen 1.2.3.4
```
If everything is successful, you will see the following:
```
Generating certificate and private key...
Saving certificate and private key...
Done!
Certificate saved to /var/lib/smu-server/certificate.crt
Private key saved to /var/lib/smu-server/privkey.key
```
Restart **smu-server** with the command
```
systemctl restart smu-server
```

Next, use **scp** to copy `certificate.crt` to the device you will use to connect to the server:
```
scp root@1.2.3.4:/var/lib/smu-server/certificate.crt .
```
After that, add this certificate to **smu**:
```
sudo smu --loadcert --cert certificate.crt --trust-ip 1.2.3.4
```
You can delete the certificate (a copy will be located in `/var/lib/smu/certs`):
```
rm certificate.crt
```
Then you can connect to the server and enjoy using it:
```
smu 1.2.3.4
```

> For Windows, the instructions are similar, except that a copy of the certificate will be located in `config/certs` next to **smu.exe**, and you do not need to write **sudo** when adding the certificate.


## Building from source
First, you need `libjsoncpp-dev` installed. On Debian-based systems, install this library with the command:

```bash
sudo apt install libjsoncpp-dev
```

You can uninstall it after the build, it is not needed for smu to work.

You also need to install OpenSSL with the command 

```bash
sudo apt install libssl-dev
```

Download the source code from the releases, unzip it somewhere. In the directory with sources create `build` directory and go to it. From now on, all commands will be executed from the build directory. Execute the following commands:

- `cmake ..`

- `cmake --build . --parallel`

- `cmake --build . --target package-all`.

  

In the `package` directory, you will see the built packages that you can install using your package manager

## Building of individual parts
You can change the following flags during the configuration phase:

| Flag | Effect | Default |
|-|-|-|
| -DBUILD_SERVER | Does the server part need to be built | ON |
| -DBUILD_CLIENT | Does the client part need to be built | ON |
| -DBUILD_CLI | Does the CLI part need to be built | ON |

For example:
```bash
cmake .. -DBUILD_SERVER=OFF
```


## License

This project is licensed under the [GPLv3 License](./LICENSE).

It uses third-party components:

- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) — MIT license
- [Drogon](https://github.com/drogonframework/drogon) - MIT license
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp) - MIT license
- [cxxopts](https://github.com/jarro2783/cxxopts) - MIT license
- [IXWebSocket](https://github.com/machinezone/IXWebSocket) - BSD-3-Clause license

See [third party](3dparty) for details
