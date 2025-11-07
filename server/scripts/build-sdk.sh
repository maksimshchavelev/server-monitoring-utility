#!/usr/bin/env bash
# ==============================================================================
# build-sdk.sh
#
# This script downloads the latest SDK releases from listed repositories,
# separates runtime and dev packages, and combines them into two packages:
#  - smu-server-sdk.deb  (runtime libraries)
#  - smu-server-sdk-dev.deb (development files)
#
# Usage:
#   Called via CMake custom target:
#     add_custom_target(sdk
#       COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build-sdk.sh ${PROJECT_VERSION}
#       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sdk-build
#     )
#
# Requirements:
#   - curl
#   - dpkg-deb
#   - tar
# ==============================================================================
set -eo pipefail

if [[ -z "$1" ]]; then
    echo "Usage: $0 <version>"
    exit 1
fi

VERSION="$1"

# SDK repositories list
REPOS=(
  "https://github.com/maksimshchavelev/smu-server-sdk-c.git"
  # Add your SDK repository here
)

BUILD_DIR="$(pwd)/sdk-build"
RUNTIME_DIR="${BUILD_DIR}/runtime"
DEV_DIR="${BUILD_DIR}/dev"

mkdir -p "${RUNTIME_DIR}" "${DEV_DIR}" "${BUILD_DIR}/tmp"

# Clean previous contents
rm -rf "${RUNTIME_DIR:?}/*" "${DEV_DIR:?}/*" "${BUILD_DIR}/tmp/*"

# Iterate over repositories
for repo_url in "${REPOS[@]}"; do
    echo "Processing repo $repo_url..."

    repo_name=$(basename "$repo_url" .git)
    owner_name=$(basename "$(dirname "$repo_url")")
    api_url="https://api.github.com/repos/$owner_name/$repo_name/releases/latest"

    assets=$(curl -s "$api_url" | grep "browser_download_url" | grep ".deb" | cut -d '"' -f 4 || true)

    if [[ -z "$assets" ]]; then
        echo -e "\033[1;33mWARNING:\033[0m No .deb assets found for $repo_url"
        continue
    fi

    echo "Found assets:"
    echo "$assets"

    while IFS= read -r asset_url; do
        deb_file="${BUILD_DIR}/tmp/$(basename "$asset_url")"

        echo "Downloading $asset_url..."
        curl -L -o "${deb_file}" "$asset_url"

        if [[ "$deb_file" == *"dev"* ]]; then
            echo "Copying $deb_file into DEV_DIR"
            dpkg-deb -x "$deb_file" "$DEV_DIR"
        else
            echo "Copying $deb_file into RUNTIME_DIR"
            dpkg-deb -x "$deb_file" "$RUNTIME_DIR"
        fi

    done <<< "$assets"
done

echo "Building combined SDK packages for version ${VERSION}..."
pushd "$BUILD_DIR" > /dev/null

# Runtime package
mkdir -p "runtime-package/DEBIAN"
cat > runtime-package/DEBIAN/control <<EOF
Package: smu-server-sdk
Version: ${VERSION}
Section: libs
Priority: optional
Architecture: amd64
Maintainer: Maksim Shchavelev <maksimshchavelev@gmail.com>
Description: Combined SDK runtime libraries
EOF

cp -r "${RUNTIME_DIR}/." runtime-package/
find runtime-package/ -type f -name "*.so" -exec chmod 500 {} \;
find runtime-package/ -type f -name "*.so" -exec chown root:root {} \;

dpkg-deb --build runtime-package "smu-server-sdk-${VERSION}.deb"

# Dev package
mkdir -p "dev-package/DEBIAN"
cat > dev-package/DEBIAN/control <<EOF
Package: smu-server-sdk-dev
Version: ${VERSION}
Section: libs
Priority: optional
Architecture: amd64
Maintainer: Maksim Shchavelev <maksimshchavelev@gmail.com>
Description: Combined SDK development files
EOF

cp -r "${DEV_DIR}/." dev-package/

dpkg-deb --build dev-package "smu-server-sdk-dev-${VERSION}.deb"

popd > /dev/null

echo "SDK packages built successfully in $BUILD_DIR/"
