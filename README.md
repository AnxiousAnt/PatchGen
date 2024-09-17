# PatchGen: Purr Data Patch Generation Using an LLM

## Project Overview

This project focuses on training and fine-tuning a language model to generate Pd patches based on natural language descriptions. Purr Data (Pd) is a visual programming language commonly used for creating interactive computer music and multimedia. Training an LLM to generate Pd patches poses a unique challenge, as the model must learn to understand and produce patches from textual input.

### Base Model used for this project: Mistral 7B
I utilized the **Mistral 7B** model from Mistral AI, which is one of the best open-sourced language models of its size.
To read more about the model, visit: [MistralAI](https://mistral.ai/news/announcing-mistral-7b/)

### Training Phases

The training process was carried out in two stages:

#### Phase 1: Continued Pretraining
- **Objective**: Expose the model to Pd syntax and structure.
- **Method**: I continued-pretraining the model using qLoRA on a large dataset of patches.
- **Dataset**: [ParZiVal04/Pd-patches-14k-dataset](https://huggingface.co/datasets/ParZiVal04/Pd-patches-14k-dataset) – Contains approximately 14,000 Pd patches.
- **Training Duration**: ~20 hours on a **Tesla T4 GPU**.

This phase provided the model with foundational knowledge of Pd’s syntax and structural elements.

#### Phase 2: Instruction Fine-Tuning
- **Objective**: Equip the model to generate Pd patches from natural language instructions.
- **Method**: I used qLoRA for fine-tuning on a dataset with annotated examples of prompts and their corresponding Pd patches.
- **Dataset**: [parzi-parzi/patch-gen-dataset-v0.8.7](https://huggingface.co/datasets/parzi-parzi/patch-gen-dataset-v0.8.7) – Contains approximately 3,000 annotated examples.
- **Training Duration**: ~12 hours on a **Tesla T4 GPU**.

This phase enables the model to interpret and respond to specific user prompts. 

### Model Availability
The model can be inferenced using the Colab Notebook linked below:

[![Open In Colab](images/colab.svg)](https://colab.research.google.com/drive/1V6L_Kc7IFd9UTT0TyVOaxZ8rAfF1lAxt?usp=sharing)

### Using the Colab Notebook

The notebook allows users to interact with the model and generate Pd patches. The notebook has a **text-based user interface** where you can either:
1. **Provide a custom prompt**, or
2. **Generate a random patch**

#### Custom Prompts
When you enter a custom prompt, the model will attempt to generate a Pd patch based on your input. However, as the model has not been extensively trained on a large variety of annotated examples, the quality of the results may vary, especially for complex patches.

#### Random Patch Generation
When selecting the random patch generation option, the model will "prompt itself" using a random example from the training data. Since the model has seen these examples before, the results are generally more accurate and functional compared to custom prompts. However, the generated patch may still have limitations in its functionality due to the model's incomplete understanding of more intricate Pd concepts.

### How to Prompt the Model

To get the best results from the model, it's advisable to provide clear, simple prompts that resemble the instructions seen during training. Below are some examples that were used in the instruction fine-tuning phase:

- **Create a Pure Data patch that calculates the absolute value of a given negative number.**
- **Create a Pure Data patch that renders a 3D teapot with adjustable size and detail level.**
- **Create a Pure Data patch that renders a square with adjustable size and color, responding to create, destroy, and draw messages.**
- **Create a Pure Data patch that takes a floating-point number as input, calculates its absolute value modulo 7, and displays the result.**
- **Create a Pure Data patch that adds 10 and 20 when a bang is triggered and prints the result.**
- **Create a Pure Data patch that takes a floating-point input and outputs the arc sine of that value, with a button to trigger the calculation.**
- **Create a Pure Data patch that takes two input numbers, squares the first number and adds the square root of the second number, then outputs the result.**
- **Create a Pure Data patch that generates white noise, applies a high-pass filter with an adjustable frequency threshold, and allows for volume adjustment before outputting the audio signal.**
- **Create a Pure Data patch that performs a bitwise AND operation on two input values and prints the result.**
- **Create a Pure Data patch that generates a random melody automatically, with a toggle to start and stop the melody, and a volume control.**
- **Create a Pure Data patch that generates pseudo-random numbers using a sine wave function.**
- **Create a Pure Data patch that takes an input value, pipes it through a 500ms delay, and outputs the delayed value.**
- **Create a Pure Data patch that generates pink noise and allows for volume control before sending the audio signal to the digital-to-analog converter.**
- **Create a Pure Data patch that scales the size of particles by a specified factor every frame.**
- **Create a Pure Data patch that generates a band-pass filtered noise signal with adjustable center frequency, Q, and volume.**

Using prompts in this format will guide the model toward generating more accurate and functional patches.

### Challenges, Limitations and Future Work

Due to the complexity of Pd and the limited size of the training datasets, the model's current performance is best suited for simple patches. To improve its functionality, continuing additional training on a larger corpus of annotated examples is necessary.

---

For more information or if you have any suggestions, feel free to create an issue or reach out!


## Extras
### Some Insights from the collected patchs:
 Here's a frequency plot of the 20 most used objects in Pd (data collected from ~35k patches):

![plot](histogram-plot\object_histogram_all.png)

- see [`histogram-plot\object_counts_all.txt`](https://github.com/AnxiousAnt/PatchGen/blob/main/histogram-plot/object_counts_all.txt) for more.

### Repository Structure
- sub-directories `\Pd_forum_archive_extraction`, `\external_patches` & `\pd patch repo scraping` contain scripts and datasets to extract patchs from the community forum, the official pd repository and the online [patch repository](https://pdpatchrepo.info/) respectively. 

- sub-directory `\PdFileFormat` contains code for implementing a linter/syntax-checker for the patches generated by the model. 

- the python notebook `\llama3_8b_patchgen.ipynb` contains the code for fine-tuning llama3 8b for patch generation

- submodule `\PurrRepo.github.io` contains code for a pd repository hosted on github pages. 

- sub-directory `\alternate_patchfile_representations` contains code for sorting the records based on their spatial coordinates and generating mermaid diagrams of patches.

- sub-directory `histogram-plot` contains code (and plot) for counting common objects used in patches. 


## links
- ### fine-tuned models and datasets: [hugging_face_link1](https://huggingface.co/ParZiVal04) & [hugging_face_link2](https://huggingface.co/parzi-parzi) (work-in-progress)

- ### [purrrepo](https://purrrepo.github.io/)
