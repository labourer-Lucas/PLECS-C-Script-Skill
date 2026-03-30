# PLECS C-Script Skill

[English](#english) | [中文](#中文)

---

## English

A GitHub Copilot skill that helps power electronics engineers write, configure, and debug **C-Script blocks** in [PLECS](https://www.plexim.com/) simulation models.

Large language models lack built-in knowledge of the PLECS C-Script macro system (`InputSignal`, `OutputSignal`, `ContState`, `DiscState`, `ContDeriv`, `CurrentTime`, etc.) and the block configuration rules (sample time, direct feedthrough, algebraic loops). This repository provides that knowledge as a Copilot custom instruction file so that Copilot can give accurate, domain-specific suggestions.

### What is PLECS C-Script?

The **C-Script block** in PLECS lets you implement custom control algorithms, signal-processing logic, and physical models using standard C code. It integrates tightly with the PLECS simulation engine through a set of macro APIs and named code sections.

### Repository Structure

```
.github/
  copilot-instructions.md   # ← Core skill: PLECS C-Script knowledge for Copilot
docs/
  api-reference.md          # Complete macro and parameter reference
  common-patterns.md        # Design patterns and best practices
examples/
  pi_controller.c           # Discrete PI controller (non-direct-feedthrough)
  first_order_filter.c      # Continuous first-order low-pass filter
  pwm_generator.c           # PWM generation using phase accumulator
  state_machine.c           # Finite state machine example
README.md
LICENSE
```

### How to Use This Skill

**Option 1 — Use the Copilot instructions in your own repo:**

Copy `.github/copilot-instructions.md` into your PLECS project repository. GitHub Copilot will automatically pick up the instructions and apply PLECS C-Script knowledge when you write code.

**Option 2 — Reference the examples directly:**

Browse the `examples/` folder for ready-to-use C-Script implementations of common power electronics control blocks.

**Option 3 — Read the documentation:**

- [`docs/api-reference.md`](docs/api-reference.md) — Every macro, its signature, and when to use it.
- [`docs/common-patterns.md`](docs/common-patterns.md) — Patterns for controllers, PWM, FSM, filters, and more.

### Quick Start Example

A complete discrete PI controller C-Script:

**Setup tab:**

| Parameter | Value |
|---|---|
| Number of inputs | `1` (error signal) |
| Number of outputs | `1` (control output) |
| Number of disc. states | `2` (integral accumulator, previous error) |
| Direct feedthrough | `0` |
| Sample time | `Ts` (e.g., `1e-4`) |
| Parameters | `Kp`, `Ki` |

**Code declarations:**
```c
#define ERROR     InputSignal(0)
#define OUTPUT    OutputSignal(0)
#define INTEGRAL  DiscState(0)
#define PREV_ERR  DiscState(1)
#define KP        ParamRealData(0, 0)
#define KI        ParamRealData(1, 0)
#define TS        SampleTimePeriod(0)
```

**Start:**
```c
INTEGRAL = 0.0;
PREV_ERR = 0.0;
```

**Output:**
```c
OUTPUT = KP * PREV_ERR + INTEGRAL;
```

**Update:**
```c
INTEGRAL = INTEGRAL + KI * TS * ERROR;
PREV_ERR = ERROR;
```

---

## 中文

本仓库是一个 GitHub Copilot 技能包，帮助电力电子工程师在 [PLECS](https://www.plexim.com/) 仿真软件中编写、配置和调试 **C-Script 模块**。

目前的大模型（包括 GitHub Copilot）不了解 PLECS C-Script 专有的宏系统（如 `InputSignal`、`OutputSignal`、`ContState`、`DiscState`、`ContDeriv`、`CurrentTime` 等）以及模块配置规则（采样时间、直接馈通、代数环等）。本仓库将这些知识以 Copilot 自定义指令文件的形式提供，使 Copilot 能够给出准确的领域专属代码建议。

### 什么是 PLECS C-Script？

PLECS 中的 **C-Script 模块** 允许工程师使用标准 C 语言实现自定义控制算法、信号处理逻辑和物理模型。它通过一套宏 API 和具名代码段与 PLECS 仿真引擎深度集成。

### 仓库结构

```
.github/
  copilot-instructions.md   # ← 核心技能文件：为 Copilot 提供 C-Script 知识
docs/
  api-reference.md          # 完整宏与参数参考手册
  common-patterns.md        # 设计模式与最佳实践
examples/
  pi_controller.c           # 离散 PI 控制器（非直接馈通）
  first_order_filter.c      # 连续一阶低通滤波器
  pwm_generator.c           # 相位累加器 PWM 生成
  state_machine.c           # 有限状态机示例
README.md
LICENSE
```

### 使用方法

**方式一 — 将 Copilot 指令复制到你的仓库：**

将 `.github/copilot-instructions.md` 复制到你的 PLECS 项目仓库中。GitHub Copilot 会自动读取该文件，并在你编写代码时应用 PLECS C-Script 专属知识。

**方式二 — 直接参考示例代码：**

浏览 `examples/` 目录，获取常见电力电子控制模块的 C-Script 实现。

**方式三 — 阅读文档：**

- [`docs/api-reference.md`](docs/api-reference.md) — 每个宏的签名与使用场景。
- [`docs/common-patterns.md`](docs/common-patterns.md) — 控制器、PWM、状态机、滤波器等设计模式。

### 快速上手示例

一个完整的离散 PI 控制器 C-Script：

**Setup 配置：**

| 参数 | 值 |
|---|---|
| Number of inputs | `1`（误差信号） |
| Number of outputs | `1`（控制输出） |
| Number of disc. states | `2`（积分累加器、上一时刻误差） |
| Direct feedthrough | `0` |
| Sample time | `Ts`（例如 `1e-4`） |
| Parameters | `Kp`, `Ki` |

**Code declarations（代码声明）：**
```c
#define ERROR     InputSignal(0)
#define OUTPUT    OutputSignal(0)
#define INTEGRAL  DiscState(0)
#define PREV_ERR  DiscState(1)
#define KP        ParamRealData(0, 0)
#define KI        ParamRealData(1, 0)
#define TS        SampleTimePeriod(0)
```

**Start（初始化）：**
```c
INTEGRAL = 0.0;
PREV_ERR = 0.0;
```

**Output（输出计算）：**
```c
OUTPUT = KP * PREV_ERR + INTEGRAL;
```

**Update（离散状态更新）：**
```c
INTEGRAL = INTEGRAL + KI * TS * ERROR;
PREV_ERR = ERROR;
```

### 核心概念速查

| 宏 | 说明 |
|---|---|
| `InputSignal(i)` | 读取第 i 个输入端口的值 |
| `OutputSignal(i)` | 设置第 i 个输出端口的值 |
| `ContState(i)` | 读写第 i 个连续状态变量 |
| `DiscState(i)` | 读写第 i 个离散状态变量 |
| `ContDeriv(i)` | 在 Derivative 段设置第 i 个连续状态的导数 |
| `CurrentTime` | 当前仿真时间 |
| `SampleTimePeriod(0)` | 采样周期 Ts |
| `ParamRealData(i, j)` | 第 i 个实数参数的第 j 个元素 |
| `IsMajorStep` | 当前是否为主步（Major Step） |

---

## Contributing / 贡献

Issues and pull requests are welcome! If you have a common C-Script pattern that isn't covered here, please open an issue or submit a PR.

欢迎提交 Issue 和 PR！如果你有尚未收录的常用 C-Script 模式，请提交 Issue 或 PR。

## License

MIT License. See [LICENSE](LICENSE).