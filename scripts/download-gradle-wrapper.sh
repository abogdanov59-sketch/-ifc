#!/usr/bin/env bash
set -euo pipefail

WRAPPER_JAR="gradle/wrapper/gradle-wrapper.jar"
if [[ -f "$WRAPPER_JAR" ]]; then
  echo "Gradle wrapper JAR already present at $WRAPPER_JAR"
  exit 0
fi

GRADLE_VERSION="8.5"
DISTRIBUTION="https://services.gradle.org/distributions/gradle-${GRADLE_VERSION}-bin.zip"
TMP_ZIP="/tmp/gradle-${GRADLE_VERSION}.zip"

echo "Downloading Gradle ${GRADLE_VERSION} distribution..."
curl -fsSL "$DISTRIBUTION" -o "$TMP_ZIP"

unzip -p "$TMP_ZIP" "gradle-${GRADLE_VERSION}/lib/gradle-launcher-${GRADLE_VERSION}.jar" > "$WRAPPER_JAR"

rm -f "$TMP_ZIP"

echo "Wrapper JAR written to $WRAPPER_JAR"
