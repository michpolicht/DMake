# DMake

Experimental QML frontend to CMake.

The project aims at answering the question whether it would be  possible to
define CMake project in a declarative fashion utilizing a full power of QML
language:

```CMakeLists.qml
import DMakeLib

Project {
    name: "SampleApp"

    AddExecutable {
        name: "SampleApp"

        sources: [
            "main.cpp"
        ]
    }
}
```

## Project status

This is currently a toy project. I wanted to know whether it would be
practically possible to integrate QML with CMake.

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

## Motivation

In the past Qt projects have been using qmake as a main build automation
tool. Nokia initiated the works on [Qbs](https://doc.qt.io/qbs/index.html) to
replace qmake, however, after many years of development, the tool never reached
its goal. In 2018 it became deprecated and CMake was chosen as a standard build
automation tool since Qt 6.

Main flaw of Qbs is that it doesn't rely on standard QML engine, but it comes
with its own QML dialect. I've made an attempt to port it to Qt QML, but it came
out that Qbs is implemented in such way that it's a very cumbersome task.

Hence this project, which is investigating an orthogonal scenario "what if we
would port CMake to QML". Although CMake codebase also isn't quite prepared for
such attempts, it seems to be doable!
