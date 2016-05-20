# Simple NPU [![Build Status](https://travis-ci.org/pfpsim/simple-npu.svg?branch=master)](https://travis-ci.org/pfpsim/simple-npu)

This repository is an example of a generic Network Processing Unit, which illustrates the methodology and approach towards system level design using the [PFPSim Framework](https://github.com/pfpsim/PFPSim).

# Getting Started:

## Prerequisites:

To start exploring this example, the PFPSim Framework is required.

- [PFPSim Library and the PFPSim P4 fork](https://github.com/pfpsim/PFPSim)
- [`pfpgen`](https://github.com/pfpsim/pfpgen)
- [`pfpdb`](https://github.com/pfpsim/pfpdb)

## Downloading the example

To complete the exercise you will need to clone the repository:

```bash
git clone https://github.com/pfpsim/simple-npu.git
```

## Building the Example:

### PFPSim Workflow:

The PFPSim workflow is described in more detail [here](https://github.com/pfpsim/pfpgen#pfpsim-methodology--workflow), but the following is a quick summary.

![WorkflowPFPSim](https://cloud.githubusercontent.com/assets/943241/15303861/0a2b6a34-1b88-11e6-8e65-37a079c0adc9.PNG)

 1. Write structural code in a FAD file.
 2. Run `pfpgen` on your FAD design, which creates:
    - SystemC code for the structural part of the model.
    - Skeleton code for behavioural description of components of the model.
    - Project setup using CMake for build management.
 3. Write behavioural code in C++ for components in the design.
 4. Simulate.

``` sh
cd simple-npu
# Run pfpgen to generate structural code
pfpgen npu.fad
cd npu/build
cmake ../src
make
```

#### A quick overview of what we just did - Project Setup.

- `npu.fad` is the blueprint of how things are built and connected to each other.
- `pfpgen` converts your structural description into human readable and compilable SystemC code -
  it literally does 50% of the work for you.
- You (the user) fill in the behavioural skeletons by providing the logic of the modules described in the FAD file in C++.
- Use the generated [CMake](https://cmake.org/) configuration to build the simulation model.
  - The build directory is generated for [out of source builds](https://cmake.org/Wiki/CMake_FAQ#Out-of-source_build_trees).
  - The Directory Layout looks something like this:

```
.
└── npu
    ├── build             ---------------- Your build directory
    |
    ├── build_static      ---------------- Runtime Configuration Files
    │   ├── Configs
    │   └── validation
    └── src               ---------------- Source Code
        ├── behavioural   ---------------- Behavioural Source Files
        │   └── common
        ├── structural    ---------------- Structural generated code
        └── tries         ---------------- PFPSIM Trie Library
            └── src
```

Behavioural code for all components is provided in this example and can be found under `npu/src/behavioural/`. A complete description of the model is coming soon on our wiki. (`#TODO`)

## Running the Example:

First link all of the config files from `npu/build_static/` into your build directory. You can also just copy them if you
want to modify them without affecting the originals.

``` sh
# From the build dir
ln -s ../build_static/* ./
```

### Simulating:
We provide a convenient wrapper script to simplify the process of running the model:

``` sh
# Run the Model
./runme.sh minimal
```

Putting it all together, the whole process should look like this:

![Process](https://cloud.githubusercontent.com/assets/2020096/15334262/701260b0-1c3b-11e6-8de9-de247c3d237f.gif)


####Inputs required by the Model:
The simulation model takes many command line arguments to specify its configuration (hence the wrapper script).

- `-c <config directory>` path to module configuration files
- `-X*` User Arguments that the user has to specify.
  - `-Xp4 <program.json>` Compiled P4 application in JSON format. Produced by
    [the P4 compiler](https://github.com/p4lang/p4c-bm).
  - `-Xtpop <table-population.txt>` Match Action Table population file. See [table.txt](/npu/build_static/table.txt) for
    the format of this file.
  - `-Xin <input.pcap>` Input pcap file that is used to generate test traffic for the model.
  - `-Xvalidation-out <output.pcap>` Pcap file that logs the output of the model. This can be used for functional
    validation against our [P4 validation model](https://github.com/pfpsim/p4-validation-model).
- `-v <level>`  Verbosity Level, valid verbosity Levels:
  - `normal`
  - `minimal`
  - `p4profile`
  - `profile`
  - `debugger`
  - `debug`

### Debugging using `pfpdb`:

To debug the NPU model with [`pfpdb`](https://github.com/pfpsim/pfpdb), run
``` sh
#Debug the Model
pfpdb npu-sim --args "-Xp4 simple_router.json -Xtpop table.txt -v minimal -Xin Configs/input.pcap -Xvalidation-out output.pcap" -v
```

``` sh
pfpdb <executable name> --args "<args given to the executable >" -v
```

### Building with Debug Symbols:
This will add [debug symbols](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html) to your compiled binary ; available for debugging in a debugger like gdb .
``` sh
cd npu/build
cmake -DCMAKE_BUILD_TYPE=DEBUG ../src
```
### Building without `pfpdb`
If you don't plan on using `pfpdb`. Components required by `pfpdb` can be compiled out, by specifying the `PFPSIMDEBUGGER` flag to cmake as
``` sh
cd npu/build
cmake -DPFPSIMDEBUGGER=ON|OFF
```
by default this flag is set to `ON`

Its status can be checked as:
``` sh
 $ cd npu/build
 $ cmake ../src
-- Searching for SystemC
...
...
-- PFPSIM DEBUGGER FLAG IS ON
...
...
```
## Built With

* A whole lot of Coffee and Ice tea :wink:.

## Contributing
If you'd like to contribute code back to `simple-npu`, please fork this github repository and send us a pull request!
Please make sure that your contribution passes our [continuous integration build](https://travis-ci.org/pfpsim/simple-npu).

Any contribution to the C++ core code must respect the coding guidelines as stated on the wiki. We rely heavily on the
Google C++ Style Guide, with some differences listed in this repository's wiki. Our Travis builds include running
[`cpplint`](https://github.com/google/styleguide/tree/gh-pages/cpplint) to ensure correct style and formatting.

## Support
If you need help using this project, please
[send us an email at `pfpsim.help@gmail.com`](mailto:pfpsim.help@gmail.com) - we'd be happy to hear from you!

If you think you've found a bug, or would like to request a new feature,
[please open an issue using github](https://github.com/pfpsim/simple-npu/issues) - we're always trying to improve PFPSim!


## License

This project is licensed under the GPLv2 - see [LICENSE](LICENSE) for details

## Authors
Copyright (C) 2016 Concordia Univ., Montreal
 - Samar Abdi
 - Umair Aftab
 - Gordon Bailey
 - Faras Dewal
 - Shafigh Parsazad
 - Eric Tremblay

Copyright (C) 2016 Ericsson
- Bochra Boughzala
