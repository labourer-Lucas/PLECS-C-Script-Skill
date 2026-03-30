 [English](#plecs-c-script-skill) | [中文](#中文说明)

---

# PLECS C-Script Skill

A GitHub Copilot or Claude skill for writing, reviewing, and debugging [PLECS C-Script](https://docs.plexim.com/plecs/latest/c-scripts/) custom control blocks.

---

## What It Does

This skill gives GitHub Copilot or Claude expert-level knowledge of the PLECS C-Script API so it can:

- Generate complete, correctly structured C-Script code for any block topology
- Configure Setup tab parameters (inputs, outputs, states, sample times, etc.)
- Transform your C code into C-Script code
- Select the right code section for each task (`Output`, `Update`, `Derivative`, `Store/Restore Custom State`, …)
- Use all PLECS macros correctly (`InputSignal`, `OutputSignal`, `ContState`, `DiscState`, `ZCSignal`, `IsSampleHit`, `ParamRealData`, `ParamStringData`, …)
- Edit `.plecs` model files directly in text form
- Validate and debug existing C-Script implementations

## Repository Structure

```
skills/
  plecs-cscript/
    SKILL.md                    ← Main skill file (loaded by Copilot or Claude)
    references/
      macros.md                 ← Complete macro reference
      examples.md               ← 8 worked examples
      plecs-file-format.md      ← .plecs text-file format reference
      cscript.plecs             ← Minimal working reference model
user manual/
  cscript_docmentation.md       ← Official PLECS C-Script documentation (reformatted)
```

## Installation

### Method 1 — Manual installation

Copy the `plecs-cscript/` folder into the skills directory of your AI assistant:

| AI Assistant | Target path |
|---|---|
| **GitHub Copilot** | `.github/skills/plecs-cscript/` |
| **Claude** | `.claude/skills/plecs-cscript/` |

For VS Code Copilot users, skills can be stored in two locations:

| Skill type | Location |
|---|---|
| **Project skills** (stored in your repository) | `.github/skills/`, `.claude/skills/`, `.agents/skills/` |
| **Personal skills** (stored in your user profile) | `~/.copilot/skills/`, `~/.claude/skills/`, `~/.agents/skills/` |

You can configure additional file locations for project skills with the `chat.skillsLocations` setting. This is useful if you want to organize skills in a different folder structure or have multiple skill directories.

### Method 2 — Let your agent install it

Open a chat with your AI assistant and say:

```
Please install the PLECS C-Script skill from GitHub:
https://github.com/labourer-Lucas/PLECS-C-Script-Skill/tree/main/skills/plecs-cscript
```

The agent will clone or download the folder and place it in the correct skills directory automatically.

## Usage
The assistant will automatically load `SKILL.md` whenever you ask about C-Script, custom blocks, or PLECS macros.

**Trigger phrases:** `C-Script`, `cscript`, `PLECS block`, `custom block`, `PLECS macro`, or any related question.

If you are working on `.plecs` model files, you can ask the assistant to generate or edit C-Script code directly in the text file directly by selecting the correct section.

## Reference

- Official documentation: [PLECS C-Script — Plexim](https://docs.plexim.com/plecs/latest/c-scripts/)
- PLECS product page: [https://www.plexim.com](https://www.plexim.com)

---

## 中文说明

本仓库是一个专为 [PLECS C-Script](https://docs.plexim.com/plecs/latest/c-scripts/) 定制控制模块**编写、审查和调试**而设计的 **GitHub Copilot 或 Claude 技能文件**。

### 功能

加载此技能后，GitHub Copilot 或 Claude 能够：

- 根据需求生成完整、结构正确的 C-Script 代码
- 正确配置 Setup 标签页参数（输入/输出端口、状态变量、采样时间等）
- 将你的 C 代码转换为 C-Script 代码
- 为每种任务选择正确的代码段（`Output`、`Update`、`Derivative`、`Store/Restore Custom State` 等）
- 正确使用所有 PLECS 宏（`InputSignal`、`OutputSignal`、`ContState`、`DiscState`、`ZCSignal`、`IsSampleHit`、`ParamRealData`、`ParamStringData` 等）
- 直接以文本形式创建或编辑 `.plecs` 模型文件
- 验证和调试已有的 C-Script 实现

### 目录结构

```
skills/
  plecs-cscript/
    SKILL.md                    ← 主技能文件（由 Copilot 或 Claude 加载）
    references/
      macros.md                 ← 完整宏参考表
      examples.md               ← 8 个完整示例
      plecs-file-format.md      ← .plecs 文件格式说明
      cscript.plecs             ← 最小化参考模型
user manual/
  cscript_docmentation.md       ← 官方 PLECS C-Script 文档（已重新格式化）
```

### 安装

#### 方法一 — 手动安装

将 `plecs-cscript/` 文件夹复制到对应 AI 助手的技能目录下：

| AI 助手 | 目标路径 |
|---|---|
| **GitHub Copilot** | `.github/skills/plecs-cscript/` |
| **Claude** | `.claude/skills/plecs-cscript/` |

对于 VS Code Copilot 用户，技能可以存储在两个位置：

| 技能类型 | 存储位置 |
|---|---|
| **项目技能**（存储在仓库中） | `.github/skills/`、`.claude/skills/`、`.agents/skills/` |
| **个人技能**（存储在用户配置文件中） | `~/.copilot/skills/`、`~/.claude/skills/`、`~/.agents/skills/` |

你可以通过 `chat.skillsLocations` 设置为项目技能配置额外的文件路径，适用于自定义文件夹结构或多技能目录的场景。

#### 方法二 — 让 AI 助手自动安装

打开与 AI 助手的对话，发送以下消息：

```
请从 GitHub 安装 PLECS C-Script 技能文件：
https://github.com/labourer-Lucas/PLECS-C-Script-Skill/tree/main/skills/plecs-cscript
```

AI 助手会自动下载该文件夹并放置到正确的技能目录中。

### 使用

当你询问 C-Script、自定义模块或 PLECS 宏相关问题时，AI 助手会自动加载并应用此技能。

**触发词：** `C-Script`、`cscript`、`PLECS block`、`custom block`、`PLECS macro` 或任何相关问题。

如果你正在处理 `.plecs` 模型文件，可以直接让 AI 助手在文本文件中选取正确的代码段来生成或编辑 C-Script 代码。
