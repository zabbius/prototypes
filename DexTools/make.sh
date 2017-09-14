#!/bin/sh

jar_file=tools.jar
dex_file=tools.dex

java_dir=java
class_dir=class

android_sdk=/opt/android-sdk/platforms/android-16
android_build_tools=/opt/android-sdk/build-tools/25.0.3

cd `dirname "$0"`

[ -d "$class_dir" ] && rm -r "$class_dir"
[ -f "$jar_file" ] && rm "$jar_file"
[ -f "$dex_file" ] && rm "$dex_file"

mkdir -p "$class_dir"

javac -d class -target 1.7 -source 1.7 -classpath "$android_sdk/android.jar" `find java/ -name "*.java"`

cd "$class_dir"
jar cvf ../tools.jar *
cd ..

"$android_build_tools/dx" --dex --output "$dex_file" "$jar_file"

