# Artifact Evaluation for SpecFS [FAST'26] SpecFS 的制品评估 [FAST'26]



This repository contains the tools, scripts, and configurations required to **reproduce and evaluate the artifacts** described in the accompanying FAST'26 paper, "Sharpen the Spec, Cut the Code: A Case for Generative File System with SYSSPEC".
本仓库包含用于复现和评估 FAST'26 论文《Sharpen the Spec, Cut the Code: A Case for Generative File System with SYSSPEC》中所述制品所需的工具、脚本和配置。

It supports both **SpecFS filesystem generation** and **end-to-end evaluation workflows**, including benchmarking and result visualization.
它同时支持 SpecFS 文件系统的生成和端到端的评估工作流，包括基准测试和结果可视化。

------

## Overview 概述



**SpecFS** is a FUSE-based userspace filesystem generated from high-level specifications. This artifact enables researchers and reviewers to:
SpecFS 是一个基于 FUSE 的用户空间文件系统，可从高级规范生成。本制品使研究人员和评审人员能够：

- Reproduce the SpecFS filesystem generation pipeline
  复现 SpecFS 文件系统生成流水线
- Execute automated evaluations and benchmarks
  执行自动化评估与基准测试
- Regenerate plots and results reported in the paper
  重新生成论文中报告的图表与结果

## Project Structure 项目结构



```
.
├── data/                  # Datasets used by evaluation workloads
├── eval/                  # Evaluation artifacts (baseline vs. optimized variants)
│   ├── delay_alloc/        # Delayed allocation optimization
│   ├── extent/             # Extent optimization
│   ├── inline_data/        # Inline data optimization
│   ├── loc/                # Lines-of-code comparison (specifications vs. code)
│   ├── pre_alloc/          # Pre-allocation optimization
│   └── rbtree/             # Red-black tree optimization
├── log/                   # Logs produced during SpecFS generation
├── plot/                  # Scripts for generating paper figures
├── result/                # Evaluation outputs and generated plots
├── sysspec/               # Specifications and code for SpecFS generation
│   ├── evolvefs/           # Filesystem evolution examples
│   ├── genfs/              # LLM-generated filesystem code
│   └── specfs/             # High-level filesystem specifications
├── tests/                 # Test scripts used for validation and evaluation
└── tools/                 # Utility scripts supporting evaluation
```



## Prerequisites 先决条件



### System Requirements 系统要求



- **Operating System**: Linux (required)
  操作系统：Linux（必需）
- **Filesystem Support**: FUSE must be enabled and functional
  文件系统支持：必须启用并确保 FUSE 功能正常

### Required System Packages 必需的系统软件包



Install the following dependencies (example for Debian/Ubuntu):
安装以下依赖项（以 Debian/Ubuntu 为例）：

```
sudo apt-get update
sudo apt-get install -y \
    wget tar make gcc \
    fuse libfuse-dev pkgconf \
    python3
```



To allow FUSE mounts by non-root users, enable the following option in `/etc/fuse.conf`:
为允许非 root 用户挂载 FUSE 文件系统，请在 `/etc/fuse.conf` 中启用以下选项：

```text
user_allow_other
```



### Python Environment Python 环境



This project uses **[uv](https://docs.astral.sh/uv/getting-started/installation/)** for Python dependency management.
本项目使用 uv 进行 Python 依赖管理。

After installing `uv`, install all Python dependencies from the project root:
安装 `uv` 后，在项目根目录下安装所有 Python 依赖：

```
uv sync
```



## Getting Started 快速开始



### Environment Configuration 环境配置



Create a `.env` file from the provided template:
根据提供的模板创建 `.env` 文件：

```
cp .env.example .env
```



Edit `.env` and configure the following variables:
编辑 `.env` 并配置以下变量：

```text
MOUNT_POINT=/home/harryz/atomfs
DEEPSEEK_API_KEY=your_api_key_here 
GOOGLE_API_KEY=your_google_api_key_here
CLIENT=google
MAX_WORKERS=5
```



**Variable descriptions: 变量描述：**

- `MOUNT_POINT`: Directory where SpecFS will be mounted
  `MOUNT_POINT` : SpecFS 将被挂载的目录
- `DEEPSEEK_API_KEY`: API key for DeepSeek
  `DEEPSEEK_API_KEY` : 用于 DeepSeek 的 API 密钥
- `GOOGLE_API_KEY`: API key for Google AI services (**recommended**)
  `GOOGLE_API_KEY` : 谷歌 AI 服务 API 密钥（推荐）
- `CLIENT`: LLM backend to use (`google` or `deepseek`)
  `CLIENT` ：要使用的 LLM 后端（ `google` 或 `deepseek` ）
- `MAX_WORKERS`: Maximum number of concurrent API requests (Due to rate limits, `5` is recommended)
  `MAX_WORKERS` : 最大并发 API 请求数（由于速率限制，建议使用 `5` ）

API keys can be obtained from:
API 密钥可从以下位置获取：

- DeepSeek: https://api-docs.deepseek.com/
- Google AI Studio: https://aistudio.google.com/
  Google AI Studio：https://aistudio.google.com/

## Running the Generation Pipeline 运行生成流水线



To generate the SpecFS filesystem and validate it end-to-end, run:
要生成 SpecFS 文件系统并进行端到端验证，请运行：

```
uv run python gen.py gen
```



This command performs the following steps:
该命令执行以下步骤：

1. Generates filesystem implementation code in `sysspec/genfs/` from specifications in `sysspec/specfs/` using an LLM (required boilerplate such as `Makefile` and FUSE interface code is pre-provided).
   使用 LLM 根据 `sysspec/specfs/` 中的规范生成 `sysspec/genfs/` 中的文件系统实现代码（必需的样板代码如 `Makefile` 和 FUSE 接口代码已预先提供）。
2. Builds the SpecFS filesystem.
   构建 SpecFS 文件系统。
3. Executes validation tests to ensure functional correctness.
   执行验证测试以确保功能正确性。

⏱ **Expected runtime**: ⏱ 预计运行时间：

- ~15–20 minutes using **Google Gemini** (recommended; faster and more stable)
  约 15–20 分钟（推荐使用 Google Gemini，速度更快且更稳定）
- ~30 minutes or longer using **DeepSeek**, depending on API latency
  约 30 分钟或更长时间，取决于 DeepSeek 的 API 延迟情况

## Running the Evaluation 运行评估



To reproduce the evaluation results reported in the paper, run:
要复现论文中报告的评估结果，请运行：

```
uv run python eval.py
```



> **Note** Some evaluations require instrumentation or additional code injected into the filesystem implementation. Pre-instrumented versions are provided in the `eval/` directory. For specification-level comparisons, refer to `eval/loc/spec/`.
> 注意：部分评估需要在文件系统实现中插入插桩或额外代码。预插桩版本已提供在 `eval/` 目录中。关于规范级别的比较，请参阅 `eval/loc/spec/` 。

After completion: 完成后：

- All evaluation workloads are executed automatically
  所有评估工作负载均自动执行
- Resulting data and plots are saved to the `result/` directory
  生成的数据和图表将保存至 `result/` 目录

## Notes for Artifact Evaluation 实验复现评审注意事项



Please refer to the detailed evaluation notes in [`docs/notes.md`](https://github.com/Panlichen/specfs-ae/blob/main/docs/notes.md), including:
请参考 `docs/notes.md` 中的详细评估说明，包括：

- Documentation of our reproduction claims.
  我们再现性声明的文档。
- Instructions for accessing the pre-configured environments provided for AE reviewers.
  供 AE 评审人访问预配置环境的操作说明。

------

## Citation 引用



If you use this artifact or any derived results, please cite our paper:
如果您使用此工件或任何衍生结果，请引用我们的论文：

```text
@article{liu2025sharpenspeccutcode,
  title   = {Sharpen the Spec, Cut the Code: A Case for Generative File System with SYSSPEC},
  author  = {Qingyuan Liu and Mo Zou and Hengbin Zhang and Dong Du and Yubin Xia and Haibo Chen},
  year    = {2025},
  url     = {https://arxiv.org/abs/2512.13047}
}
```



## License 许可证



This repository is released under the [MIT License](https://github.com/Panlichen/specfs-ae/blob/main/LICENSE).
本仓库在 MIT 许可证下发布。