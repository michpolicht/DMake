# DMake

Experimental QML frontend to CMake.

## Building

So far the project has been built on Windows with with Qt >= 6.8, MSVC 2022.
Just open main `CMakeLists.txt` with Qt Creator and build it.

## Running the example

In `Qt Creator -> Projects -> Build & Run -> Run` pick `dmake` as
`Run configuration`. Executable should point to `dmake.exe` on Windows.

 In `Command line arguments` type:

```
%{sourceDir}/examples/SampleApp -G DNinja --project-file CMakeLists.qml
```

Trigger `Run` and modified Ninja generator (DNinja) should generate files in
`<build-directory>/out/bin` directory. You can run `ninja` on these files to
build SampleApp.
