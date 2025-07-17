## What is smu-cli?
smu-cli is a terminal-based server management tool that allows you to enable and disable modules, configure behavior, change configs, and more using commands — without worrying about breaking something. This document provides a guide to smu-cli.

## Installation
To install, download the release package and install it via your package manager. After that, the tool will be accessible through the `smu-cli` command.

> Note that smu-server must also be installed and running, otherwise attempting to run a command will result in a `connection refused` error.

## Building from Source
To build, download the source code of the desired release, navigate to the `cli` folder, create a `build` directory, and navigate into it. The following commands will be executed from the `build` directory:

 - Run `cmake ..`. Wait for configuration to complete.
 - Run `cmake --build . --parallel`
 - Run `cmake --build . --target package` to build the package
 - A built package will appear in the `build` directory. Install it using your package manager.

## Command Reference
smu-cli supports several commands, here are some of them:
- `--version` displays the version of smu-cli
- `--help` or `-h` shows help
- `--list commands` prints the list of commands. Note that these commands are provided by the server.
- `--list modules` displays the list of modules, their statuses (running / stopped), and descriptions. Here's an example of what it might look like:
```

NAME		STATUS		DESCRIPTION

RAM			<span style="color:green">RUNNING</span>		A module that allows you to get information about RAM

```
- `--run <modules>` starts the specified module(s). Just list the modules in place of `<modules>`, for example: `--run RAM` or `--run RAM TEMP`.

> Note that if the server does not find a module by name, it will immediately return an error. In that case, modules listed before the nonexistent one will be turned on (or remain on if they already were), and the state of the modules listed after it will not be changed.

- `--stop <modules>` works the same way as `--run <modules>`, but stops the modules instead.

## Conclusion
In this guide, you learned how to install, build, and use smu-cli to work with smu-server.

