Huffman Encoder/Decoder
=======================
This project is free Software under the terms of the GNU General Public License
Version 2. See COPYING for full license text.

A (yet unoptimized) huffman encoder and decoder.

Usage:
-------
```
Usage:
  Encode: huffman -e input-file -o output-file [-s 1|2|4|8]
  Decode: huffman -d input-file -o output-file

Options:
  --help                        This help message
  -e [ --encode-file ] arg      File to be encoded
  -d [ --decode-file ] arg      File to be decoded
  -s [ --entity-size ] arg (=1) When encoding, interpret input in blocks of 
                                this size in bytes. Input file size must be a 
                                multiple of this size. Possible values: 1, 2, 
                                4, 8
  -o [ --output-file ] arg      Output file. Must not exist.
```


Project structure:
-------------------
- **src/**    
  Huffman source
- **test/src/**    
  Unit tests
- **benchmark/src/**    
  Benchmarks
- **scripts/**    
  Helper scripts

Build:
-------
```
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./huffman --help
```

Build unit tests:
------------------
```
cd test/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./huffman-test
```

Build benchmarks:
------------------
```
cd benchmark/build
cmake .. # defaults to release mode
make
./huffman-benchmark --benchmark_repetitions=10
```

Dependencies:
--------------
cmake >= 2.8.7, boost (`boost::program_options`), [datas-and-algos](https://github.com/thomastrapp/datas-and-algos "Github: datas-and-algos") (`ds::priority_queue`, `util::make_unique`), [googletest](http://code.google.com/p/googletest/ "Google Code: googletest") and [googlebenchmark](https://github.com/google/benchmark "Github: googlebenchmark").  
The dependencies on googletest, googlebenchmark and datas-and-algos are automatically satisfied through cmake (Module `ExternalProject`).

