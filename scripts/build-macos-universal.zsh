#!/usr/bin/env zsh

set -e

# Navigate to repository root
basedir="$(cd "$(dirname "$0")/.." && pwd)"
cd "$basedir"

# Setup environment variables
. scripts/setenv

# Directory to stage per-arch app bundles
stage_dir="macos_universal"
rm -rf "$stage_dir"
mkdir -p "$stage_dir"/x64 "$stage_dir"/arm64

# Build x64 bundle
echo "Building OpenRCT2 (x64)..."
. scripts/setenv -q
scripts/build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=on -DARCH="x86_64"
mv bin/OpenRCT2.app "$stage_dir"/x64/
rm -rf bin

# Build arm64 bundle
echo "Building OpenRCT2 (arm64)..."
. scripts/setenv -q
scripts/build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=on -DARCH="arm64" -DWITH_TESTS=on
mv bin/OpenRCT2.app "$stage_dir"/arm64/
rm -rf bin

# Create universal app bundle
cd "$stage_dir"
"$basedir/scripts/create-macos-universal"
cd "$basedir"

# Package universal bundle
mkdir -p artifacts
mv "$stage_dir"/OpenRCT2-universal.app artifacts/OpenRCT2.app
echo -e "\033[0;36mCompressing OpenRCT2.app...\033[0m"
cd artifacts
zip -rqy OpenRCT2-macos-universal.zip OpenRCT2.app
cd "$basedir"


