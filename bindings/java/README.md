# Java bindings for rpi-rgb-led-matrix

This folder contains a Gradle project that implements a JNI bridge to the C++ APIs.

# Usage

First, compile the project (see below) and add the resulting api.jar file to your project, e.g. when using Gradle:

```kotlin
implementation(files(File(rootDir, "libs/api.jar")))
```
Then start using it:

```java
RGBMatrix matrix = RGBMatrix.createFromOptions(new RGBMatrix.Options());
matrix.setPixel(10, 10, 255, 255, 255);
```

See [sample](samples/src/main/java) directory for elaborate examples.

# Troubleshooting

If you start your java program with sudo, it will drop permission to
the daemon user by default. This means that the classpath might become
inaccessible to the running program, leading to **class loading errors** when
using other libraries. Explicitly configure a fitting user:

```java
RGBMatrix.RuntimeOptions runtimeOptions = new RGBMatrix.RuntimeOptions();
runtimeOptions.dropPrivileges = 1;
runtimeOptions.dropPrivUser = "someuser";
runtimeOptions.dropPrivGroup = "somegroup";
```

#

# Performance Overhead

The Java bindings are quite efficient. The most important calls are optimized.
On my Raspberry PI Zero 2W `setPixel()` is about eight times slower than direct
native calls. But that is still fast enough to completely fill a 64x64 matrix
about 400 times per second.

# Building

I use cross compiling to build the API on my development machine.

```shell
sudo apt install g++-aarch64-linux-gnu gcc-aarch64-linux-gnu
make CXX=aarch64-linux-gnu-g++ CC=gcc-aarch64-linux-gnu

cd bindings/java

# Build api.jar (placed in api/build/libs)
./gradlew :api:jar

# Or package the samples to try on the PI
./gradlew :samples:distZip

# copy samples/build/distributions/samples.zip to you pi and unzip it
# then, run it with: sudo samples/bin/samples <number>
```

# TODOs

 - not all APIs are exposed to Java right now, some could potentially be useful to add:
   - command flag parsing
   - pixel mappers
   - ...
 - maybe add automated tests, or at least a sample that calls every API to verify
   that none are missing or crashing
 - Set up Maven publishing so people do not have to compile it themselves
