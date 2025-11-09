
## What is the SMU?
SMU is a Server Monitoring Utility that transmits over a secure websocket connection various server data such as CPU utilization, RAM and swap file usage, component temperature, fan speed, etc.

## Guides
SMU is represented by three components - [smu-server](server/README.md) (server part), [smu-cli](cli/README.md) (CLI for controlling the server via terminal) and [smu](client/README.md) (client for visualizing and viewing information from the server). The links will take you to the manual for each component

## Documentation
Documentation for regular users and developers wishing to support the project:
- [smu-server documentation](https://maksimshchavelev.github.io/server-monitoring-utility/server/docs/html/index.html)
- [smu-cli documentation](https://maksimshchavelev.github.io/server-monitoring-utility/cli/docs/html/index.html)
- [smu (client) documentation](https://maksimshchavelev.github.io/server-monitoring-utility/client/docs/html/index.html)

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

> If you want to install **external modules** for the server, install the `smu-server-linux-amd64-external-modules.deb` and `smu-server-sdk.deb` packages from the page of the release you need. A list of external modules is provided in the [server documentation](https://maksimshchavelev.github.io/server-monitoring-utility/server/docs/html/index.html)

> A list of built-in modules is provided in the [server documentation](https://maksimshchavelev.github.io/server-monitoring-utility/server/docs/html/index.html)


## Building from source on Linux
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

- `../configure`

- `cmake --build . --parallel`

- `cmake --build . --target package-all`.

 
In the `package` directory, you will see the built packages that you can install using your package manager


## Building of individual parts
You can use the `components` parameter when calling `../configure`. For example, the following command will configure the `client` and `server` components:

```{.bash}
../configure --components=client,server
```

By default (if `components` is not specified), all components are configured. Here is a list of components:

| Component | Effect |
|-|-|
| server | The `server` component will be built  |
| cli | The `cli` component will be built  |
| client | The `client` component will be built  |


## Building SDK

If you want to use external modules, you need to build the SDK. You can do this by running the following command *after building the server part* (everything is built by default, which is also fine):
```
cmake --build . --target sdk
```
After executing the command, look in `build/server-build/sdk-build/` (the detailed path will be specified after executing the command), where you will see two packages (the version will be specified):

- `smu-server-sdk.deb`
- `smu-server-sdk-dev.deb`

> In fact, no SDK build in the usual sense takes place. The compiled artifacts are downloaded from the SDK repositories and recompiled into two packages.



## Cross-platform build for Windows under Linux

Only the `client` component can be built on Windows. You will need to install some dependencies using the command:

```{.bash}
sudo apt install -y build-essential zlib1g-dev wget curl mingw-w64 gcc perl
```

Then download the source code for the release you need, navigate to the root of the source code directory, create a `build` directory, and navigate to it. Run the following commands:

```
../configure --platform=x86-64-windows --components=client 
```

Next is the command for building:

```
cmake --build . --parallel
```

If the build was successful, go to `client`. You'll find `smu.exe` and the necessary dynamic libraries there. You can start using it.


## Configure script

You can get help with configuration by running:
```{.bash}
../configure --help
```


## License

This project is licensed under the [GPLv3 License](./LICENSE).

It uses third-party components:

- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) — MIT license
- [Drogon](https://github.com/drogonframework/drogon) - MIT license
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp) - MIT license
- [cxxopts](https://github.com/jarro2783/cxxopts) - MIT license
- [IXWebSocket](https://github.com/machinezone/IXWebSocket) - BSD-3-Clause license
- [OpenSSL](https://openssl-library.org)

See [third party](3dparty) for details
