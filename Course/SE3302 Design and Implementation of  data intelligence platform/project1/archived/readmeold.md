---
license: mit
language:
  - en
size_categories:
  - 1K<n<10K
---

**IT Skills Named Entity Recognition (NER) Dataset**

## Description:

This dataset includes **5,029** curriculum vitae (CV) samples, each annotated with IT skills using **Named Entity Recognition (NER)**. The skills are manually labeled and extracted from PDFs, and the data is provided in JSON format. This dataset is ideal for training and evaluating NER models, especially for extracting IT skills from CVs.

## Highlights:

- **5,029 CV samples** with annotated IT skills
- **Manual annotations** for IT skills using Named Entity Recognition (NER)
- **Text extracted from PDFs** and annotated for IT skills
- **JSON format** for easy integration with NLP tools like Spacy
- **Great resource** for training and evaluating NER models for IT skills extraction

## Dataset Details

- **Total CVs:** 5,029
- **Data Format:** JSON files
- **Annotations:** IT skills labeled using Named Entity Recognition

## Data Description

Each JSON file in the dataset contains the following fields:

| Field         | Description                                                                |
| ------------- | -------------------------------------------------------------------------- |
| `text`        | The extracted text from the CV PDF                                         |
| `annotations` | A list of IT skills annotated in the text, where each annotation includes: |

- `start`: Starting position of the skill in the text (zero-based index)
- `end`: Ending position of the skill in the text (zero-based index, exclusive)
- `label`: The type of the entity (IT skill)

### Example JSON File

Here is an example of the JSON structure used in the dataset:

```json
{
  "text": "One97 Communications Limited \nData Scientist Jan 2019 to Till Date \nDetect important information from images and redact\nrequired fields. YOLO CNN Object-detection, OCR\nInsights, find anomaly or performance drop in all\npossible sub-space. \nPredict the Insurance claim probability. Estimate the\npremium amount to be charged\nB.Tech(Computer Science) from SGBAU university in\n2017. \nM.Tech (Computer Science Engineering) from Indian\nInstitute of Technology (IIT), Kanpur in 2019WORK EXPERIENCE\nEDUCATIONMACY WILLIAMS\nDATA SCIENTIST\nData Scientist working  on problems related to market research and customer analysis. I want to expand my arsenal of\napplication building and work on different kinds of problems. Looking for a role where I can work with a coordinative team\nand exchange knowledge during the process.\nJava, C++, Python, Machine Learning, Algorithms, Natural Language Processing, Deep Learning, Computer Vision, Pattern\nRecognition, Data Science, Data Analysis, Software Engineer, Data Analyst, C, PySpark, Kubeflow.ABOUT\nSKILLS\nCustomer browsing patterns.\nPredict potential RTO(Return To Origin) orders for e-\ncommerce.\nObject Detection.PROJECTS\nACTIVITES",
  "annotations": [
    [657, 665, "SKILL: Building"],
    [822, 828, "SKILL: python"],
    [811, 815, "SKILL: java"],
    [781, 790, "SKILL: Knowledge"],
    [877, 887, "SKILL: Processing"],
    [194, 205, "SKILL: performance"],
    [442, 452, "SKILL: Technology"],
    [1007, 1014, "SKILL: PySpark"],
    [30, 44, "SKILL: Data Scientist"],
... ] }
```

## Usage

This dataset can be used for:

- Training Named Entity Recognition (NER) models to identify IT skills from text.
- Evaluating NER models for their performance in extracting IT skills from CVs.
- Developing new NLP applications for skill extraction and job matching.

## How to Load and Use the Data

To load and use the data, you can use the following Python code:

```python
import json
import os
# Define the path to the directory containing the JSON files
directory_path = "path/to/your/json/files"
# Load all JSON files
data = []
for filename in os.listdir(directory_path):
    if filename.endswith(".json"):
        with open(os.path.join(directory_path, filename), "r") as file:
            data.append(json.load(file))
# Example of accessing the first CV's text and annotations
first_cv = data[0]
text = first_cv['text']
annotations = first_cv['annotations']
print(f"Text: {text}")
print(f"Annotations: {annotations}")
```
