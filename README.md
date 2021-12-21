# Fuzz a byteview

 byteview 是 bytectf2021 final 中出现的一道题，对于这样的题目，我觉得可以用 fuzz 的方式来获取 poc。

具体的请参考[我的博客](https://cjovi.icu/fuzzing/1589.html)

使用 AFLplusplus（只是为了用他的 qemu 和 qasan）和 libprotobuf 生成结构化的输入进行 fuzz。**并没有用到任何 AFL 的变异，只是为了方便用他执行 qemu 和 qasan**

## usage
准备环境
```
source ./build_all_and_setup_env.sh
```
开始 fuzz
```
AFLplusplus/afl-fuzz -i input -o output -Q -- ./byteview
```

fuzz 前需要设置环境变量，每次新开终端是都需要通过 `source ./setup_env.sh` 来重新设置。

input 中随便放个文件就行了，实际上并不需要用到，只是为了让 AFL 能跑起来。

目录下的 fuzz.sh 是 *“另一种方法”*，具体请参考博客

### reference

基于以下两个项目实现

> [afl-libprotobuf-mutator](https://github.com/thebabush/afl-libprotobuf-mutator)
>
> [protobuf_ctf_fuzz](https://github.com/Kiprey/protobuf_ctf_fuzz)
