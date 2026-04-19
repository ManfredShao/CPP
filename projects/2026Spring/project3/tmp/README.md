# Project 3 实验框架：C 语言矩阵乘法性能对比

这个框架已覆盖除报告外的实现任务：

- `matmul_plain()`：朴素三重循环基线。
- `matmul_improved()`：转置 + 分块 + OpenMP + SIMD（AVX2/NEON）优化。
- `matmul_openblas()`：可选 OpenBLAS 对比路径。
- 基准测试：支持 `16, 128, 1024, 8192, 65536`，自动按内存上限跳过不可执行规模。
- 正确性检查：与 `plain` 和 `OpenBLAS` 比较最大绝对误差与相对误差。
- 输出：终端摘要 + 可选 CSV。

## 文件说明

- `matrix.h` / `matrix.c`：矩阵结构、分配、随机初始化、误差计算。
- `matmul.h` / `matmul.c`：三种矩阵乘法实现。
- `benchmark.h` / `benchmark.c`：性能测试与结果统计。
- `main.c`：命令行参数解析与实验入口。
- `CMakeLists.txt` / `Makefile`：构建脚本。

## 构建

```bash
cd /Users/manfred/Documents/CPP/projects/2026Spring/project3
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_OPENBLAS=ON
cmake --build build -j
```

> 若系统未安装 OpenBLAS/BLAS，CMake 会提示并自动关闭 OpenBLAS 路径，仍可运行 improved/plain 对比。

## 运行示例

```bash
./build/project3 --sizes 16,128,1024,8192,65536 --runs 3 --plain-max 1024 --mem-gib 8 --csv result.csv
```

常用参数：

- `--sizes`：逗号分隔的测试规模。
- `--runs`：improved/OpenBLAS 计时重复次数，取最优值。
- `--plain-max`：仅在 `N <= plain-max` 时执行 plain（避免超慢）。
- `--mem-gib`：单个 case 的总内存上限（按 4 个 `N×N` 浮点矩阵估算）。
- `--no-openblas`：禁用 OpenBLAS 对比。
- `--csv`：导出 CSV 数据。

## 结果解读建议

- 报告中建议给出：
  - 各规模下 improved 与 plain/OpenBLAS 的耗时与 GFLOPS。
  - speedup（`plain_time / improved_time`，`openblas_time / improved_time`）。
  - 误差（max abs / max rel）。
- 对 `64K×64K`，若内存不足被自动跳过，可在报告中说明硬件限制与估算内存需求。
