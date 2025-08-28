#!/usr/bin/env bash
# ==============================================================================
# build-sdk.sh
#
# Downloads latest SDK .deb releases, separates runtime and dev packages,
# combines them into smu-server-sdk.deb and smu-server-sdk-dev.deb.
#
# Usage:
#   Called via CMake custom target:
#     add_custom_target(sdk
#       COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build-sdk.sh
#       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sdk-build
#     )
#
# Requirements:
#   - curl
#   - dpkg-deb
# ==============================================================================
set -eo pipefail

# SDK repositories list
REPOS=(
  "https://github.com/maksimshchavelev/smu-server-sdk-c.git"
  # Add your SDK repository here
)

BUILD_DIR="$(pwd)/sdk-build"
RUNTIME_DIR="${BUILD_DIR}/runtime"
DEV_DIR="${BUILD_DIR}/dev"

mkdir -p "${RUNTIME_DIR}" "${DEV_DIR}"

# Clean previous contents
rm -rf "${RUNTIME_DIR:?}/*" "${DEV_DIR:?}/*"

info() {
    echo "[INFO] $*"
}

warn() {
    echo "[WARNING] $*"
}

error() {
    echo -e "\033[1;31m[ERROR]\033[0m $*" >&2
}

for repo_url in "${REPOS[@]}"; do
    info "Processing repo $repo_url..."

    repo_name=$(basename "$repo_url" .git)
    owner_name=$(basename "$(dirname "$repo_url")")
    api_url="https://api.github.com/repos/$owner_name/$repo_name/releases/latest"

    # Get download URLs for .deb packages
    assets=$(curl -s "$api_url" | grep "browser_download_url" | grep ".deb" | cut -d '"' -f 4 || true)

    if [[ -z "$assets" ]]; then
        warn "No .deb assets found for $repo_url"
        continue
    fi

    info "Found assets:"
    echo "$assets"

    while IFS= read -r asset_url; do
        info "Downloading $asset_url..."
        if ! curl -L -O "$asset_url"; then
            error "Failed to download $asset_url"
            continue
        fi

        deb_file=$(basename "$asset_url")

        if [[ "$deb_file" == *"dev"* ]]; then
            info "Copying $deb_file into DEV_DIR"
            dpkg-deb -x "$deb_file" "$DEV_DIR"
        else
            info "Copying $deb_file into RUNTIME_DIR"
            dpkg-deb -x "$deb_file" "$RUNTIME_DIR"
        fi
    done <<< "$assets"
done

# Build combined packages
VERSION="1.0.0"  # optional: detect dynamically if needed
PACKAGE_RUNTIME="smu-server-sdk-${VERSION}.deb"
PACKAGE_DEV="smu-server-sdk-dev-${VERSION}.deb"

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

# Set permissions for .so files
find runtime-package/ -type f -name "*.so" -exec chmod 500 {} \;
find runtime-package/ -type f -name "*.so" -exec chown root:root {} \;

dpkg-deb --build runtime-package "$PACKAGE_RUNTIME"

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

dpkg-deb --build dev-package "$PACKAGE_DEV"

popd > /dev/null

info "SDK packages built successfully in $BUILD_DIR/"
