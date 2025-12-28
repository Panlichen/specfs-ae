# Artifact Evaluation for SpecFS  [FAST'26]

This repository contains the tools, scripts, and configurations required to **reproduce and evaluate the artifacts** described in the accompanying FAST'26 paper, "Sharpen the Spec, Cut the Code: A Case for Generative File System with SYSSPEC". 

It supports both **SpecFS filesystem generation** and **end-to-end evaluation workflows**, including benchmarking and result visualization.

---

## Overview

**SpecFS** is a FUSE-based userspace filesystem generated from high-level specifications. This artifact enables researchers and reviewers to:

- Reproduce the SpecFS filesystem generation pipeline
- Execute automated evaluations and benchmarks
- Regenerate plots and results reported in the paper



## Project Structure

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

````



## Prerequisites

### System Requirements

- **Operating System**: Linux (required)
- **Filesystem Support**: FUSE must be enabled and functional

### Required System Packages

Install the following dependencies (example for Debian/Ubuntu):

```bash
sudo apt-get update
sudo apt-get install -y \
    wget tar make gcc \
    fuse libfuse-dev pkgconf \
    python3
````

To allow FUSE mounts by non-root users, enable the following option in `/etc/fuse.conf`:

```text
user_allow_other
```

### Python Environment

This project uses **[uv](https://docs.astral.sh/uv/getting-started/installation/)** for Python dependency management.

After installing `uv`, install all Python dependencies from the project root:

```bash
uv sync
```



## Getting Started

### Environment Configuration

Create a `.env` file from the provided template:

```bash
cp .env.example .env
```

Edit `.env` and configure the following variables:

```text
MOUNT_POINT=/home/harryz/atomfs
DEEPSEEK_API_KEY=your_api_key_here 
GOOGLE_API_KEY=your_google_api_key_here
CLIENT=google
MAX_WORKERS=5
```

**Variable descriptions:**

* `MOUNT_POINT`: Directory where SpecFS will be mounted
* `DEEPSEEK_API_KEY`: API key for DeepSeek
* `GOOGLE_API_KEY`: API key for Google AI services (**recommended**)
* `CLIENT`: LLM backend to use (`google` or `deepseek`)
* `MAX_WORKERS`: Maximum number of concurrent API requests
  (Due to rate limits, `5` is recommended)

API keys can be obtained from:

* DeepSeek: [https://api-docs.deepseek.com/](https://api-docs.deepseek.com/)
* Google AI Studio: [https://aistudio.google.com/](https://aistudio.google.com/)



## Running the Generation Pipeline

To generate the SpecFS filesystem and validate it end-to-end, run:

```bash
uv run python gen.py gen
```

This command performs the following steps:

1. Generates filesystem implementation code in `sysspec/genfs/` from specifications in `sysspec/specfs/` using an LLM
   (required boilerplate such as `Makefile` and FUSE interface code is pre-provided).
2. Builds the SpecFS filesystem.
3. Executes validation tests to ensure functional correctness.

⏱ **Expected runtime**:

* ~15–20 minutes using **Google Gemini** (recommended; faster and more stable)
* ~30 minutes or longer using **DeepSeek**, depending on API latency



## Running the Evaluation

To reproduce the evaluation results reported in the paper, run:

```bash
uv run python eval.py
```

> **Note**
> Some evaluations require instrumentation or additional code injected into the filesystem implementation.
> Pre-instrumented versions are provided in the `eval/` directory.
> For specification-level comparisons, refer to `eval/loc/spec/`.

After completion:

* All evaluation workloads are executed automatically
* Resulting data and plots are saved to the `result/` directory

## Notes for Artifact Evaluation

Please refer to the detailed evaluation notes in [`docs/notes.md`](docs/notes.md), including:
* Documentation of our reproduction claims.
* Instructions for accessing the pre-configured environments provided for AE reviewers.




---

## Citation

If you use this artifact or any derived results, please cite our paper:

```text
@article{liu2025sharpenspeccutcode,
  title   = {Sharpen the Spec, Cut the Code: A Case for Generative File System with SYSSPEC},
  author  = {Qingyuan Liu and Mo Zou and Hengbin Zhang and Dong Du and Yubin Xia and Haibo Chen},
  year    = {2025},
  url     = {https://arxiv.org/abs/2512.13047}
}
```

## License

This repository is released under the [MIT License](LICENSE).

