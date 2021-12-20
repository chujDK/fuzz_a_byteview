# Fuzz a byteview

byteview 是 bytectf2021 final 中出现的一道题，对于这样的题目，我觉得可以用 fuzz 的方式来获取 poc。
具体的等我睡个觉再来写 :)

使用 AFLplusplus（只是为了用他的 qemu 和 qasan）和 libprotobuf 生成结构化的输入进行 fuzz。
**并没有用到任何 AFL 的变异，只是用他执行 qemu 和 qasan**

## usage
准备环境
```
./build_all_and_setup_env.sh
source ./setup_env.sh
```
开始 fuzz
```
AFLplusplus/afl-fuzz -i input -o output -Q -- ./byteview
```

input 中随便放个文件就行了，实际上并不需要用到，只是为了通过 AFL 的检测。

### reference

基于以下两个项目实现

> [afl-libprotobuf-mutator](https://github.com/thebabush/afl-libprotobuf-mutator)
>
> [protobuf_ctf_fuzz](https://github.com/Kiprey/protobuf_ctf_fuzz)