from openai import OpenAI
from datasets import load_from_disk
import json
import argparse
import base64
from tqdm import tqdm
import os

client = OpenAI(
    base_url="http://47.242.151.133:24576/v1/",
    api_key="ml2025",
)


def load_data(path):
    '''
    Load data from disk
    Args:
        path: str, the path to the data
    Returns:
        ds: Dataset, the data
    '''
    ds = load_from_disk(path)
    return ds


from PIL import Image

def preprocess_image(example):
    '''
    Concatenate all available document pages vertically
    Args:
        example: dict, the example containing multiple images
    Returns:
        concatenated_image: Image, concatenated image
    '''
    # Collect all available images
    images = []
    for i in range(1, 21):  # MP-DocVQA has up to 20 images
        img_key = f"image_{i}"
        if img_key in example and example[img_key] is not None:
            images.append(example[img_key])
    
    # If no images found, return the first image as fallback
    if not images:
        return example["image_1"]
    
    # If only one image, return it directly
    if len(images) == 1:
        return images[0]
    
    # Resize all images to same width (420px) while maintaining aspect ratio
    target_width = 420
    resized_images = []
    for img in images:
        # Calculate new height maintaining aspect ratio
        w_percent = (target_width / float(img.size[0]))
        h_size = int((float(img.size[1]) * float(w_percent)))
        resized_img = img.resize((target_width, h_size), Image.LANCZOS)
        resized_images.append(resized_img)
    
    # Calculate total height
    total_width = target_width
    total_height = sum(img.size[1] for img in resized_images)
    
    # Create blank image and paste resized images
    concatenated_image = Image.new('RGB', (total_width, total_height))
    y_offset = 0
    for img in resized_images:
        concatenated_image.paste(img, (0, y_offset))
        y_offset += img.size[1]
    
    return concatenated_image

def generate_answer(example):
    '''
    Generate answer for the an example  
    Args:
        example: dict, the example
    Returns:
        answer: str, the answer
    '''
    # Preprocess the image
    image = preprocess_image(example)

    # Convert image to base64
    tmp_dir = "./tmp"
    os.makedirs(tmp_dir, exist_ok=True)
    tmp_file = os.path.join(tmp_dir, "tmp_image.png")
    image.save(tmp_file, format="PNG")

    with open(tmp_file, "rb") as f:
        encoded_image = base64.b64encode(f.read())
    encoded_image_text = encoded_image.decode("utf-8")
    base64_image = f"data:image;base64,{encoded_image_text}"

    # Prompt
    text = f"{example['question']}\nOnly return the answer, no other words."

    chat_response = client.chat.completions.create(
        model="Qwen/Qwen2.5-VL-3B-Instruct",
        messages=[
            {"role": "system", "content": "You are a helpful assistant."},
            {
                "role": "user",
                "content": [
                    {
                        "type": "image_url",
                        "image_url": {
                            "url": base64_image
                        },
                    },
                    {"type": "text", "text": text},
                ],
            },
        ],
    )
    return chat_response.choices[0].message.content


def evaluate_results(results):
    '''
    Evaluate the results
    Args:
        results: list, the results
    Returns:
        score: float, the score
    '''
    # Calculate the score
    score = 0
    for result in results:
        if result["generation"].lower() in result["answers"].lower():
            score += 1
    return round(score / len(results), 2)


def main(args):
    if not args.eval_only:
        # Load data
        ds = load_data(args.data_path)

        # Generate
        results = []
        for example in tqdm(ds, desc="Generating answers", total=len(ds), leave=True, position=0):
            print(example)
            answer = generate_answer(example)
            print(answer)
            results.append({"generation": answer, "answers": example['answers']})

        # Evaluate
        pass_rate = evaluate_results(results)
        print(f"Pass rate: {pass_rate}")

        # Save results to disk
        output_dir = os.path.dirname(args.output_path)
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        with open(args.output_path, "w") as f:
            json.dump(results, f)
    else:
        # Load results
        with open(args.output_path, "r") as f:
            results = json.load(f)
        # Evaluate
        pass_rate = evaluate_results(results)
        print(f"Pass rate: {pass_rate}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_path", type=str, default="./data/mp_docvqa_100")
    parser.add_argument("--output_path", type=str, default="./results/mp_docvqa_100_results_concatenate.json")
    parser.add_argument("--eval_only", action="store_true")
    args = parser.parse_args()

    main(args)
