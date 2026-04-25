##用前需读：下载压缩包解压后双击integral_gui.exe即可运行
同时推广自己的公众号：格致谈思录，欢迎来看我胡思乱想~




# 定积分计算器 (Numerical Integral Calculator)

一个基于 C++17 的数值定积分计算工具，提供 **命令行 (CLI)** 和 **Windows 图形界面 (GUI)** 两种交互方式。

核心功能：输入任意数学表达式（如 `sin(x^2) * exp(-x)`），指定积分区间，即可快速计算定积分的数值近似值。

---

## 功能特性

- **三种数值积分方法**：梯形法、Simpson 1/3 法则、自适应 Simpson 法
- **强大的表达式解析**：基于 [exprtk](https://github.com/ArashPartow/exprtk) 头文件库，支持所有标准数学函数（三角函数、指数、对数、双曲函数等）
- **双重交互方式**：CLI 适合脚本集成，GUI 适合交互式使用
- **Windows 原生 GUI**：使用 Win32 API 构建，Unicode 支持，中文显示无乱码
- **自动边界处理**：支持上下限互换、零区间等边界情况
- **误差控制**：自适应方法提供可配置的误差容限

---

## 项目结构

```
定积分工具/
├── Makefile                 # MinGW 构建 (mingw32-make)
├── CMakeLists.txt           # CMake 构建 (VS 2022)
├── README.md                # 本文件
├── include/
│   ├── integrator.h         # 数值积分接口
│   └── parser.h             # 数学表达式解析封装 (基于 exprtk)
├── src/
│   ├── main.cpp             # CLI 入口
│   ├── gui.cpp              # Win32 GUI 入口
│   └── integrator.cpp       # 积分算法实现
├── external/exprtk/
│   └── exprtk.hpp           # 表达式解析库 (header-only, ~1.6 MB)
├── build/                   # 编译中间产物
├── integral.exe             # CLI 可执行文件
└── integral_gui.exe         # GUI 可执行文件
```

---

## 快速开始

### 环境要求

- **编译器**：MinGW-w64 g++ 15.2.0+ 或 Visual Studio 2022
- **标准**：C++17
- **系统**：Windows 10/11

### 使用 MinGW 构建

```bash
# 编译 CLI + GUI
mingw32-make all

# 仅编译 CLI
mingw32-make integral

# 仅编译 GUI
mingw32-make integral_gui

# 编译并运行 CLI
mingw32-make run

# 编译并运行 GUI
mingw32-make run-gui

# 清理构建产物
mingw32-make clean
```

### 使用 CMake 构建

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

或在 VS 2022 中直接打开 `CMakeLists.txt`。

### 注意

- MinGW 的 `make` 命令命名为 `mingw32-make`，使用 `make` 可能会报错
- 编译时需添加 `-Wa,-mbig-obj` 标志，因为 exprtk.hpp 模板展开会产生大量 COFF section，超出默认限制

---

## CLI 使用方法

```
integral <表达式> <下限> <上限> [选项]
```

### 选项

| 参数 | 说明 |
|------|------|
| `--method <trapezoidal\|simpson\|adaptive>` | 选择积分方法（默认: adaptive） |
| `--n <int>` | 梯形法 / Simpson 法的分段数（默认: 1000） |
| `--tol <float>` | 自适应法的误差容限（默认: 1e-6） |
| `--help` | 显示帮助信息 |
| `--methods` | 列出支持的数学函数和常数 |

### 使用示例

```bash
# 自适应 Simpson 法计算 sin(x) 在 [0, π] 上的积分
integral "sin(x)" 0 3.141592653589793

# Simpson 法，100 等分
integral "x^2" 0 1 --method simpson --n 100

# 梯形法
integral "1/x" 1 2 --method trapezoidal --n 10000

# 高精度自适应
integral "exp(-x^2)" -5 5 --tol 1e-10
```

### 支持的表达式

自变量为 `x`，支持完整数学函数集，包括但不限于：

**基本运算**：`+`, `-`, `*`, `/`, `^`（乘方）, `%`（取模）

**三角函数**：`sin`, `cos`, `tan`, `asin`, `acos`, `atan`

**双曲函数**：`sinh`, `cosh`, `tanh`

**对数与指数**：`exp`, `log`（自然对数）, `log10`, `log2`

**其他**：`sqrt`, `abs`, `floor`, `ceil`, `round`, `sign`, `pow`

**常数**：`pi`（π ≈ 3.14159）, `e`（欧拉数 ≈ 2.71828）

---

## GUI 使用方法

直接运行 `integral_gui.exe` 即可打开图形界面。

![GUI 截图占位](docs/screenshot.png)

界面包含：
- **被积函数** — 输入数学表达式（如 `sin(x^2)`）
- **积分下限 / 上限** — 输入积分区间
- **计算方法** — 下拉选择：自适应 Simpson（默认）/ 辛普森 / 梯形法
- **精度控制** — 自适应法的误差容限（默认 1e-6）
- **计算定积分** 按钮 — 点击开始计算
- 结果显示区：积分值（粗体）、函数求值次数、使用方法

所有中文界面均通过 Unicode API 实现，无乱码问题。

---

## 数值方法

### 梯形法 (Trapezoidal Rule)

用线性插值（梯形）近似每个子区间上的积分：

$$ \int_a^b f(x) dx \approx \frac{h}{2} \left[ f(a) + 2\sum_{i=1}^{n-1} f(a+ih) + f(b) \right] $$

- 收敛阶：$O(1/n^2)$
- 优点：实现简单，计算量小
- 缺点：收敛慢，需大量分段才能达到高精度

### Simpson 1/3 法则 (Simpson's Rule)

用二次插值（抛物线）近似每对相邻子区间上的积分（需要偶数分段）：

$$ \int_a^b f(x) dx \approx \frac{h}{3} \left[ f(a) + 4\sum f_{奇} + 2\sum f_{偶} + f(b) \right] $$

- 收敛阶：$O(1/n^4)$
- 对三次多项式可精确积分
- 相比梯形法，相同分段数下精度显著更高

### 自适应 Simpson 法 (Adaptive Simpson)

自动在函数变化剧烈的区域加密网格，平滑区域稀疏采样：

1. 在当前区间用 Simpson 公式计算近似值
2. 在中点分割，分别计算左右子区间的 Simpson 值
3. 用 Richardson 外推估计误差
4. 误差超标则递归细分，容差减半

- 自动适应函数复杂度
- 对平滑函数只需很少求值次数
- 提供可配置的误差保证

---

## 验证结果

以下积分使用自适应 Simpson 法（默认容差 1e-6）计算：

| 表达式 | 区间 | 计算结果 | 理论值 |
|--------|------|---------|--------|
| sin(x) | [0, π] | 2.000000000000 | 2 |
| x² | [0, 1] | 0.333333333333 | 1/3 |
| exp(-x²) | [-5, 5] | 1.772453851413 | √π |
| 1/x | [1, 2] | 0.693147243060 | ln(2) |
| sqrt(1-x²) | [-1, 1] | 1.570775786750 | π/2 |
| sin²(x)+cos²(x) | [0, π] | 3.141592653500 | π |

---

## 技术栈

| 条目 | 说明 |
|------|------|
| 语言 | C++17 |
| 编译器 | g++ 15.2.0 (MinGW-w64) |
| 外部依赖 | [exprtk](https://github.com/ArashPartow/exprtk) 1.0 (header-only) |
| GUI 框架 | 原生 Win32 API (无外部 GUI 库) |
| 构建系统 | Makefile + CMakeLists.txt |
| 目标平台 | Windows 10/11 |
| 代码规模 | ~550 行（不含 exprtk） |

---

## License

本项目仅供学习交流使用。
