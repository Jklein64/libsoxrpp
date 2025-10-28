# soxrpp

RAII C++ wrapper for [libsoxr](https://sourceforge.net/projects/soxr/), a popular 1D signal resampling library written in C. This wrapper is also templated, which makes it easier to use in C++ codebases (for example, it supports using lambda functions with captures instead of C-style function pointers for the `set_input_fn` API).

> [!TIP]
> This wrapper library makes the input and output types of a resampler template parameters (as opposed to runtime enum values) to enable compile-time checking of buffer types. The underlying soxr library, however, uses an enum for types and switches on its value at runtime. This difference will generally not present issues, but it does make it more difficult to write code that supports dynamic input and output types, such as [soxr's example 3](https://sourceforge.net/p/soxr/code/ci/master/tree/examples/3-options-input-fn.c).

## Getting Started

Installation for `soxrpp` uses CMake and is compatible with either `find_package` or `FetchContent`. See the [examples](./examples/) for usage. All API methods have documentation in [soxrpp.h](./include/soxrpp.h).

> [!IMPORTANT]
> Builds were only tested on the Docker image specified by the [Dockerfile](./Dockerfile). Builds on other systems, such as MacOS or Windows, might not work as expected.

### Installation for use with `find_package`

Clone the repository and run the following from the project root:

```bash
cmake -B build -D CMAKE_BUILD_TYPE=Release
# Add additional build flags after "--", like "-j 8" for a Makefiles generator
cmake --build build --target install 
```

And then in your project's `CMakeLists.txt`, add:

```cmake
find_package(soxrpp REQUIRED)
# Replace "target_name" with the name of your target
target_link_libraries(target_name PRIVATE soxrpp::soxrpp)
```

### Installation with `FetchContent`

Add the following to your `CMakeLists.txt` file:

```cmake
include(FetchContent)
# Replace "main" with a git tag if you want to pin it
FetchContent_Declare(soxrpp GIT_REPOSITORY https://github.com/Jklein64/soxrpp.git GIT_TAG main)
FetchContent_MakeAvailable(soxrpp)
# Replace "target_name" with the name of your target
target_link_libraries(target_name PRIVATE soxrpp::soxrpp)
```

## Why?

I'm working on a physics simulator that generates audio, ideally in real-time, which naturally requires significant resampling. A typical timestep for physics simulations is around `1e-6`, which corresponds to a 1 MHz sample rate. That's much bigger than the 44.1 kHz or 48 kHz that are typical for high-quality audio. Lots of existing C++ libraries only support integer ratios, which would struggle to downsample 1 MHz to 48 kHz (requiring 480x upsampling before decimation). I opted to wrap [libsoxr](https://github.com/chirlu/soxr?tab=readme-ov-file), which is what's used by [librosa](https://librosa.org/doc/0.11.0/generated/librosa.resample.html#librosa-resample), for example.

Could I have just used raw pointers and `void*` everywhere and used the soxr library as-is? Yes. Could I have just used the [r8brain](https://github.com/avaneev/r8brain-free-src) resampler instead? Yes. But I didn't know that one existed at the time, and I learned a lot about template metaprogramming and configuring CMake to work with FetchContent for header-only libraries. Ask me how I got capturing lambdas to play nicely with a function pointer-based API :)