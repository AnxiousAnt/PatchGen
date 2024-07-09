# PatchGen

# Generating Pd patches using an LLM
## Project Summary
Purr-Data (Pd), a visual programming language for multimedia applications, offers a powerful and intuitive environment for sound and music creation. However, the initial learning curve for Pd can be daunting, especially for those unfamiliar with visual programming paradigms. Also, the experienced Purr-data users would want to generate patches faster. This project titled "Generating Pd Patches Using an LLM", aims to bridge this gap by leveraging the power of machine learning to assist users in creating Pd patches.

## description 
- sub-directories `\Pd_forum_archive_extraction`, `\external_patches` & `\pd patch repo scraping` contain scripts and datasets to extract patchs from the community forum, the official pd repository and the online [patch repository](https://pdpatchrepo.info/) respectively. (work-in-progress)

- sub-directory `\PdFileFormat` contains code for implementing a linter/syntax-checker for the patches generated by the model. (work-in-progress)

- the python notebook `\llama3_8b_patchgen.ipynb` contains the code for fine-tuning llama3 8b for patch generation


## links
- ### fine-tuned models and datasets: [hugging_face_link](https://huggingface.co/ParZiVal04) (work-in-progress)

