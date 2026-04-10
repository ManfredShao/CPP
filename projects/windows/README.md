# Windows Benchmark Pack

这个目录用于在 Windows 机器上复现实验，并和 macOS 结果做对比。

## 目录内容

- `dotproduct.c`：C 基准程序
- `Dotproduct.java`：Java 基准程序
- `run_dotproduct_c.ps1`：运行 C（`-O0/-O1/-O2/-O3/-Ofast`）并生成多轮统计
- `run_dotproduct_java.ps1`：运行 Java 并生成多轮统计
- `run_all.ps1`：一键跑 C + Java
- `plot_dotproduct_comparison.py`：生成图和每元素耗时表（含 `C-Ofast`）
- `run_plot.ps1`：Windows 下调用绘图脚本

## 依赖要求

1. PowerShell（Windows 自带）
2. C 编译器（任选其一，需在 `PATH`）
   - LLVM `clang`
   - MinGW-w64 `gcc`
3. JDK 17+（`java`/`javac` 在 `PATH`）
4. Python 3（可选，仅绘图需要）
   - `pip install pandas matplotlib tabulate`

## 建议执行顺序

在 `windows` 目录打开 PowerShell，执行：

```powershell
Set-Location .\windows
Set-ExecutionPolicy -Scope Process Bypass
```

### 1) 一键跑全部（推荐）

```powershell
.\run_all.ps1 -Runs 5
```

### 2) 分开跑 C 和 Java（可选）

```powershell
.\run_dotproduct_c.ps1 -Runs 5
.\run_dotproduct_java.ps1 -Runs 5
```

### 3) 生成图和每元素耗时表（可选）

```powershell
python -m pip install pandas matplotlib tabulate
.\run_plot.ps1
```

## 主要输出文件

### C（每个优化级别都会生成）

- `results_c(O0)_run1.csv` ... `results_c(O0)_run5.csv`
- `results_c(O0)_mean.csv`
- `results_c(O0)_median.csv`
- `results_c(O0).csv`

同理还会生成：`O1`、`O2`、`O3`、`Ofast`。

### Java

- `results_java_run1.csv` ... `results_java_run5.csv`
- `results_java_mean.csv`
- `results_java_median.csv`
- `results_java.csv`

### 图和表（运行绘图后）

- `output_plots/dot_time_compare_median_*.png`
- `output_plots/ns_per_element_mean.csv`
- `output_plots/ns_per_element_median.csv`
- `output_plots/ns_per_element_table.md`
