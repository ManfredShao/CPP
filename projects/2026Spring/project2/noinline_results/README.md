# noinline 对照实验结果

该目录保存“去掉 `__attribute__((noinline))`”后的 C 基准结果。

## 目录说明

- `dotproduct.c`：去掉 `noinline` 的源码版本
- `run_dotproduct_c.sh`：复用原脚本，在本目录执行
- `results_c(O0~O3)_run*.csv`：每个优化级 5 次原始结果
- `results_c(O0~O3)_mean.csv`：均值汇总
- `results_c(O0~O3)_median.csv`：中位数汇总（建议用于画图）

## 复现命令

```bash
cd /Users/manfred/Documents/CPP/projects/noinline_results
./run_dotproduct_c.sh 5
```
