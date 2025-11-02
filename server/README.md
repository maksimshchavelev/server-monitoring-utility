# smu-server

This component is one of the three parts of **smu** — the server monitoring system. You can install it by downloading the `.deb` package from the releases or by building it manually (explained below).

Check the usage guide [here](#Usage). If you'd like to **contribute to the development** of smu-server, head over [here](for-developers/beginning.md).

## List of external modules

Modules allow you to monitor a specific group of metrics. For example, the CPU module monitors processor parameters. A list of **external** modules is provided below.

| Module | Purpose |
|--|--|
| UPTIME | Monitoring system uptime |


## List of built-in modules

| Module | Purpose |
|--|--|
| RAM | RAM usage monitoring |
| DMI | Information about the motherboard |

## How to build smu-server?

First, clone the main repository and navigate to the `server` folder. For convenience, we will build it in a separate `build` directory (you'll need to create it — all commands below should be executed from the `build` directory).

> Be sure to install `libjsoncpp-dev` and `libssl-dev`! Also, the cmake version must be **3.28** or **higher**.

Configure `smu-server` with:

```bash
cmake ..
```

You can set the following options:

| Option | Values | Description | Default Value |
|--|--|--|--|
| `-DCMAKE_BUILD_TYPE` | `Release` / `Debug` | Specifies the build type: Release or Debug. In Debug mode, the build will include sanitizers. | `Release` |
| `-DCMAKE_INSTALL_PREFIX` | Any path | Sets the installation directory for the executable. For example, `/usr/local` will install `smu-server` to `/usr/local/bin` | `/usr/` |

**Example**:

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
```

Once configuration is complete, build `smu-server` with:

```bash
cmake --build . --parallel
```

To install it, build the package after building `smu-server`:

```bash
cmake --build . --target package
```

> ⚠️ It is **not recommended** to run `sudo cmake --install .`. Instead, build the package and install it using your package manager. This way, the package can be cleanly removed later.

Now just install the resulting package.  
(**Don't forget to open the firewall port — 5050 by default!**)

> If installed via a package manager, a systemd service script will be installed, systemd will reload, and `smu-server` will start immediately. The service is called `smu-server` (you can manage it via `systemd`).


## Building SDK


If you want to use external modules, you need to build the SDK. You can do this by running the following command *after building the server part* (everything is built by default, which is also fine):
```
cmake --build . --target sdk
```
After executing the command, look in `build/sdk-build/` (the detailed path will be specified after executing the command), where you will see two packages (the version will be specified):

- `smu-server-sdk.deb`
- `smu-server-sdk-dev.deb`

> In fact, no SDK build in the usual sense takes place. The compiled artifacts are downloaded from the SDK repositories and recompiled into two packages.


## Usage

> `smu-server` comes with a CLI tool that allows you to configure the server without directly editing config files. This method is preferred (see the CLI documentation for details). Below is a basic overview of the config structure, which is also worth reading.

After installation, you can configure `smu-server` however you like. But before making any changes to the configs, stop the server:

```bash
sudo systemctl stop smu-server
```

Then go to `/var/lib/smu-server`. You'll see a `config.json` file (which holds server settings) and a `modules.d` folder, which contains config files for each module. Here's an example of the `config.json` file (as of version 0.1.0):

```json
{
  "port": 5050,
  "send_interval_ms": 1000
}
```

It's pretty straightforward:  
- `"port"` defines the port that the server listens on. You can change it to any free port.  
- `"send_interval_ms"` sets how often information is sent, in milliseconds. In this case — every 1000 ms (i.e., every second).

Now let’s look at an example module configuration for the `RAM` module (which provides information about memory usage, total memory, etc.). Its config is located at `modules.d/RAM.json` and looks like this:

```json
{
  "enabled": true,
  "poll_ratio": 1
}
```

As you can see, there's a single `"enabled"` field that determines whether the module is active. If disabled, the module won't send any data. Set it to `false` to disable the module.

> Each module has its own specific settings, but the `enabled` field is always present. You can enable or disable modules individually.

`poll_ratio` is responsible for the data update rate. For example, in the server configuration, `send_interval_ms` is equal to `1000`, and in the module configuration, `poll_ratio` is equal to `1`. Then the module data will be updated every second. If `poll_ratio` is equal to `5`, then every `5` seconds. If `0`, then the first response from the module will be cached.

> ⚠️ You should not normally change `poll_ratio`, especially if it is set to `0`!


Once you're done configuring, start `smu-server` with:

```bash
sudo systemctl start smu-server
```

You can also check whether the server started correctly (and see logs if errors occurred) with:

```bash
sudo systemctl status smu-server
```

## Problems after updating
If you encounter a **module registration error** after updating, for example:
```
[30.07.25 10:00:59] [MODULE RAM] The 'poll_ratio' field is missing. Can't continue
[30.07.25 10:00:59] [MODULE RAM] Registration failed! Cause: The 'poll_ratio' field is missing
```

Then you need to delete the module configuration and restart the server. Usually, the module configuration is located in the `/var/lib/smu-server/modules.d` directory. In the example above, you need to delete `/var/lib/smu-server/modules.d/RAM.json`

## Conclusion

We’ve covered installation, building from source, and a basic configuration example for `smu-server`. Enjoy using it!
