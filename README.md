# Multi-Language Detection Dataset & Methodology

This repository contains a comprehensive dataset and the mathematical framework for a high-precision Language Detection System. The project transitions from simple frequency analysis to a robust statistical model based on **Compositional Data Analysis (CoDA)**.

## Dataset Overview
The dataset (`language_database.csv`) consists of **400+ entries** covering six languages: **Romanian, English, German, Hungarian, Turkish, and Dutch**. 

Each entry includes:
- **Unique ID**: A normalized string combining Title and Author.
- **Expected Language**: The ground truth label.
- **Detected Language**: Result from the classification engine.
- **Accuracy Flag**: Binary indicator (1 for success).

The samples include a diverse mix of classical literature (Grimm Brothers, H.C. Andersen, Mihai Eminescu, Ion Creangă), modern song lyrics, and Wikipedia articles to ensure statistical variety.

---

## Mathematical Framework

Unlike traditional NLP classifiers that use raw counts, this system treats language data as **Compositional Data**, where the relative proportions of characters and bigrams carry the essential information.

### 1. Centered Log-Ratio (CLR) Transformation
To move from the constrained space of the Simplex (where percentages must sum to 100%) to the Unconstrained Euclidean space, we apply the **CLR transformation**:
$$CLR(x) = \ln\left(\frac{x}{g(x)}\right)$$
where $g(x)$ is the geometric mean of the composition. This allows for rigorous distance calculations without the "closure" bias of percentages.

### 2. Laplace Smoothing (Additive Smoothing)
To handle the "zero-frequency problem" (when a character or bigram is missing from a sample), we implement **Laplace Smoothing** with a pseudo-count of $\delta = 0.5$. This ensures the stability of the logarithmic calculations and prevents mathematical singularities.

### 3. Aitchison Distance
The similarity between a text sample and a language profile is calculated using the **Aitchison Distance**, which is the Euclidean distance between their respective CLR-transformed vectors:
$$d_A(x, y) = \sqrt{\sum_{i=1}^{D} (CLR(x)_i - CLR(y)_i)^2}$$
This metric is scale-invariant and perfectly suited for comparing linguistic "fingerprints" regardless of text length.

---

## Tech Stack
- **Language**: C++17
- **Data Format**: CSV (Normalized for high interoperability)
- **Frameworks**: Qt for the Graphical User Interface (GUI)
- **Methodology**: Compositional Data Analysis (CoDA)
