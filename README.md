## hipewSample

hipewを利用してWindows環境でHIP(C++ Heterogeneous-Compute Interface for Portability)にアクセスするサンプルです。

### リポリトジをクローンする際はサブモジュールも同時にクローンしてください。
```bash
git clone --recursive https://github.com/skyt301/hipewSample.git
```
or
```bash
cd Orochi
git submodule update --recursive
```

### HIP API Reference
https://rocmdocs.amd.com/projects/HIP/en/develop/index.html

### 注意点
カーネルコード内での`printf`と`assert`関数はWindows上では動作しないようです。
また、カーネル内で`__host__`修飾した関数もおそらく動作しません。

