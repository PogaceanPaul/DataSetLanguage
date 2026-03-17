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

---

## Linguistic Reference Data & Sources

The system's accuracy relies on high-fidelity language profiles. Each language is represented by a probability distribution (normalized to a sum of 10,000 units) derived from established linguistic corpora.

## Linguistic Reference Data & Computational Profiling

The system's accuracy relies on high-fidelity compositional language profiles. To ensure mathematical rigor for the Compositional Data Analysis (CoDA), all language profiles must operate within a strict simplex. 

For languages where exact, diacritic-sensitive frequency percentages were readily available, 1:1 mappings were extracted from peer-reviewed literature. For languages where existing literature primarily provided relative rankings or omitted specific diacritic distributions (e.g., Romanian `ă, ș, ț`), **custom computational profiling** was performed. This involved extracting, analyzing, and normalizing absolute character and bigram frequencies from large-scale national reference corpora using NLP techniques to fit the exact mathematical constraints of the Aitchison geometry.

### Primary Sources & Corpora

| Language | Profiling Method | Primary Source / Reference Corpus | Link / Reference |
| :--- | :--- | :--- | :--- |
| **English** | Literature Extraction | Abbas & Kareem (2019). *Text Language Identification Using Letters Analysis*. | [DOI: 10.35741/issn.0258-2724.54.4.21](https://doi.org/10.35741/issn.0258-2724.54.4.21) |
| **German** | Literature Extraction | Abbas & Kareem (2019). *Text Language Identification Using Letters Analysis*. | [DOI: 10.35741/issn.0258-2724.54.4.21](https://doi.org/10.35741/issn.0258-2724.54.4.21) |
| **Turkish** | Literature Extraction | Dalkılıç & Dalkılıç (2006). *On the Cryptographic Patterns and Frequencies in Turkish*. | [IJINS Journal](https://iaescore.com/journals/index.php/IJINS) |
| **Romanian** | Computational Profiling | **CoRoLa** (Reference Corpus of the Contemporary Romanian Language) - Tufiș et al. (2014). | [http://corola.racai.ro/](http://corola.racai.ro/) |
| **Dutch** | Computational Profiling | **SUBTLEX-NL** Corpus (Word Frequency Based on Film Subtitles) - Keuleers et al. (2010). | [http://crr.ugent.be/subtlex-nl/](http://crr.ugent.be/subtlex-nl/) |
| **Hungarian** | Computational Profiling | **MNSZ** (Magyar Nemzeti Szövegtár / Hungarian National Corpus) - Oravecz et al. | [http://corpus.nytud.hu/mnsz/](http://corpus.nytud.hu/mnsz/) |

*Note: The generated profiles preserve the natural linguistic hierarchy and properly weight language-specific diacritics, ensuring zero mathematical singularities during classification thanks to Laplace smoothing ($\delta = 0.5$).*

### Frequency Normalization Strategy
To ensure a rigorous comparison in the **Aitchison geometry**, all language profiles follow a "closed" composition:
1. **Unigrams**: Top 30+ characters per language, normalized to $\kappa = 10,000$.
2. **Bigrams**: Strategically selected high-entropy bigrams (e.g., `th`, `he` for English; `de`, `în` for Romanian) to sharpen the unweighted Aitchison distance.
3. **Diacritics Bonus**: A decaying weight system that prioritizes language-specific graphemes (e.g., `ț`, `ő`, `ğ`), ensuring high precision even for short text fragments.

### Data Transparency
The raw frequency data is decoupled from the source code and stored in `frecvente_limbi.csv`. This allows for:
- **Scalability**: Adding a new language only requires a new column in the CSV.
- **Verifiability**: Researchers can cross-reference the `int` weights in the file with the percentages found in the cited academic literature.
