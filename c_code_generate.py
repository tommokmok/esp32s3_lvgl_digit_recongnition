import os
from pathlib import Path
from PIL import Image

DATASET_DIR = Path("./models/dataset")
OUTPUT_C = DATASET_DIR / "digit_test_data.c"
OUTPUT_H = DATASET_DIR / "digit_test_data.h"
IMG_ROWS = 25
IMG_COLS = 30
ARRAY_SIZE = IMG_ROWS * IMG_COLS

def get_digit_folders(dataset_dir):
    return sorted([f for f in dataset_dir.iterdir() if f.is_dir() and f.name.isdigit()])

def png_to_array(png_path):
    img = Image.open(png_path).convert('L')
    img = img.resize((IMG_COLS, IMG_ROWS))  # Ensure correct size
    arr = list(img.getdata())
    # Convert to 0/1 if needed (threshold)
    arr = [1 if v > 127 else 0 for v in arr]
    return arr

def generate_c_arrays():
    digit_folders = get_digit_folders(DATASET_DIR)
    all_arrays = []
    all_labels = []
    counts = {}

    for digit_folder in digit_folders:
        digit = int(digit_folder.name)
        png_files = sorted(digit_folder.glob("*.png"))
        counts[digit] = len(png_files)
        for png_file in png_files:
            arr = png_to_array(png_file)
            if len(arr) != ARRAY_SIZE:
                print(f"Warning: {png_file} size mismatch, skipping.")
                continue
            all_arrays.append(arr)
            all_labels.append(digit)

    return all_arrays, all_labels, counts

def write_h_file(num_samples):
    with open(OUTPUT_H, "w") as f:
        f.write("#pragma once\n\n")
        f.write("#define DIGIT_TEST_IMG_ROWS %d\n" % IMG_ROWS)
        f.write("#define DIGIT_TEST_IMG_COLS %d\n" % IMG_COLS)
        f.write("#define DIGIT_TEST_IMG_SIZE %d\n" % ARRAY_SIZE)
        f.write("#define DIGIT_TEST_NUM %d\n\n" % num_samples)
        f.write("extern const unsigned char digit_test_data[DIGIT_TEST_NUM][DIGIT_TEST_IMG_SIZE];\n")
        f.write("extern const unsigned char digit_test_label[DIGIT_TEST_NUM];\n")

def write_c_file(arrays, labels):
    with open(OUTPUT_C, "w") as f:
        f.write('#include "digit_test_data.h"\n\n')
        f.write("const unsigned char digit_test_data[DIGIT_TEST_NUM][DIGIT_TEST_IMG_SIZE] = {\n")
        for arr in arrays:
            f.write("    { " + ", ".join(str(x) for x in arr) + " },\n")
        f.write("};\n\n")
        f.write("const unsigned char digit_test_label[DIGIT_TEST_NUM] = {\n    ")
        f.write(", ".join(str(x) for x in labels))
        f.write("\n};\n")

def main():
    arrays, labels, counts = generate_c_arrays()
    print("Digit counts:", counts)
    print("Total samples:", len(arrays))
    write_h_file(len(arrays))
    write_c_file(arrays, labels)
    print(f"Generated {OUTPUT_H} and {OUTPUT_C}")

if __name__ == "__main__":
    main()