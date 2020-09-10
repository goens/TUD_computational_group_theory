#!/bin/bash

set -e
set -x

if [ -z "$DEPLOY" ]; then
  exit 0
fi

AUTHOR="Timo Nicolai"
AUTHOR_EMAIL="timonicolai@arcor.de"

USER_NAME="Time0o"
REPO_NAME="mpsym"
REPO_URL="github.com/$USER_NAME/$REPO_NAME.git"

PYPI_DOCKER_IMAGE=quay.io/pypa/manylinux1_x86_64
PYPI_DOCKER_SRC_DIR=/mpsym
PYPI_DOCKER_WHEELS_DIR="$PYPI_DOCKER_SRC_DIR/wheelhouse"

# upload coverage data
echo "Uploading coverage data..."
mkdir gcov
cd gcov
gcov ../source/CMakeFiles/**/*
bash <(curl -s https://codecov.io/bash)
cd ..

# release build
echo "Preparing release build..."
rm -rf *
cmake -DCMAKE_BUILD_TYPE=Release -DLUA_NO_ROCK=ON -DBUILD_DOC=ON ..

# deploy documentation
echo "Deploying documentation..."
make doc

echo "Cloning gh-pages branch..."
git clone -b gh-pages "https://$REPO_URL"

cd "$REPO_NAME"

echo "Configuring git..."
git config push.default simple
git config user.name "$AUTHOR"
git config user.name "$AUTHOR_EMAIL"

if [ -d ../doxygen/html ] && [ -f ../doxygen/html/index.html ]; then
  echo "Pushing documentation..."
  cp -r ../doxygen/html/* .
  git add .
  git commit --amend -m "Deploy documentation, current commit is $TRAVIS_COMMIT"
  git push --force "https://$REPO_TOKEN@$REPO_URL" > /dev/null 2>&1
else
  echo "Failed to find generated documentation" >&2
  exit 1
fi

cd ..

# publish PyPi package
echo "Publishing PyPi package"

docker pull "$PYPI_DOCKER_IMAGE"

docker run --rm -v "$(dirname "$0")":"$PYPI_DOCKER_SRC_DIR" \
  -e SRC_DIR="$PYPI_DOCKER_SRC_DIR" \
  -e WHEELS_DIR="$PYPI_DOCKER_WHEELS_DIR" \
  -e TWINE_PYPI_USER_NAME="$USER_NAME" \
  -e TWINE_TESTPYPI_USER_NAME="$USER_NAME" \
  -e TWINE_PYPI_PASSWORD="$PYPI_PASSWORD" \
  -e TWINE_TESTPYPI_PASSWORD="$TESTPYPI_PASSWORD" \
  "$PYPI_DOCKER_IMAGE" "$PYPI_DOCKER_SRC_DIR/buildwheels.sh"
