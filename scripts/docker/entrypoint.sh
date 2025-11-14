#!/bin/bash
set -e

# If the first argument is "smu-cli" — run CLI with all following arguments
if [ "$1" = "smu-cli" ]; then
    shift
    exec smu-cli "$@"
else
    # Default: run server
    exec smu-server "$@"
fi

