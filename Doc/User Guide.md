# Fluminum: a tool for matrix operations 

Welcome to Fluminum\! This guide will walk you through everything you need to know to use the application effectively. Whether you're multiplying large matrices or comparing them for differences, Fluminum provides a powerful yet easy-to-use interface. Let's get started\!

-----

## ✅ GENERAL USAGE OVERVIEW

Fluminum is designed to be interactive and informative. You'll primarily interact with it through a command-line interface, but don't worry – it's designed to be clear and straightforward.

### Launching the App

When you first run `fluminum.exe` (or the equivalent executable on your system), you'll see a quick initialization sequence followed by some helpful **System Information**:

This box tells you:

  * Your computer's **Total** and **Available Physical RAM**. This is useful for knowing if you can handle very large matrices.
  * The number of **Logical CPU Cores**. This impacts how many threads the program can effectively use for parallel processing.
  * **SIMD Support**: Whether your processor can use special instructions (like AVX or SSE2) to speed up calculations. Green is good\!

### The Main Menu

After the system info, you'll reach the **Main Menu**:

Here, you have two main choices:

1.  **Matrix Multiplication**: Use this when you need to calculate the product of two matrices (A \* B). Fluminum uses an advanced algorithm called Strassen's method (along with parallel processing) to make this super fast for large matrices.
2.  **Matrix Comparison**: Use this when you need to check if two matrices are identical, or *almost* identical (within a tiny tolerance). This is great for verifying results or checking for data integrity.

Simply type `1` or `2` and press `Enter` to make your selection.

### Logging Your Results

Before you start an operation, the program will ask:

```
Log results to CSV? (y/n): 
```

  * Type `y` if you want to save a detailed record of your operation (dimensions, timings, memory usage) to a CSV (Comma Separated Values) file. This is fantastic for tracking performance or collecting data for analysis. You'll be asked for a filename (e.g., `my_tests.csv`).
  * Type `n` if you just want to see the results on the screen and don't need to save them.

### The Basic Workflow

1.  **Launch** the application.
2.  **Choose** your operation (Multiplication or Comparison).
3.  Decide whether to **Log** results.
4.  Provide the required **Inputs** (matrix dimensions, files, or manual entries).
5.  Configure **Settings** (thresholds, threads).
6.  **Run** the operation and watch the progress.
7.  **Review** the results.
8.  Choose whether to **Save** the output matrix (if multiplying).
9.  Decide if you want to perform **Another Operation** or exit.

-----

## 🧮 MATRIX MULTIPLICATION

This is the core feature for performing `C = A * B`.

### 1\. Setting Dimensions

First, you'll need to define the sizes of your matrices:

```
Matrix A - Rows: [Enter number]
Matrix A - Cols: [Enter number]
Matrix B - Rows: [Enter number]
Matrix B - Cols: [Enter number]
```

  * **Important:** For multiplication, the number of **Columns in Matrix A** *must* equal the number of **Rows in Matrix B**. The program will warn you if they don't match.

### 2\. Memory Estimation

The program will show you an **Estimated Peak RAM** usage. Pay attention to this\!

  * If the estimate is **high** compared to your available RAM (especially if it shows a red warning), you risk the program running very slowly (due to using disk swap) or even crashing. Consider using smaller matrices if this happens.

### 3\. Inputting Matrices

You have three ways to provide your matrices (A and B):

1.  **Random Generation**:
      * **Best for**: Testing performance, or when you need large matrices quickly and don't care about the specific values.
      * **How**: Select `1`. The program will rapidly create matrices A and B with your specified dimensions, filled with random numbers. A spinner `| / - \` will show it's working.
2.  **Manual Console Input**:
      * **Best for**: Very small matrices (e.g., 3x3, 4x4).
      * **How**: Select `2`. You'll be prompted to enter each row's numbers, separated by spaces or commas. Press `Enter` after each row.
      * **Warning**: This is *very slow and tedious* for anything but tiny matrices. Avoid for large inputs\!
3.  **Read from File**:
      * **Best for**: Using your own specific data or re-using matrices.
      * **How**: Select `3`. You'll be asked for the filename for Matrix A and Matrix B.
      * **File Format**: The file should be a plain text file. Each line represents a row, and numbers in the row should be separated by spaces or commas. Ensure the dimensions in the file match what you entered\!

### 4\. Multiplication Settings

Next, you'll fine-tune how the multiplication runs:

  * **Strassen Threshold**:
      * **What it is**: Strassen's algorithm is fast for big matrices but has overhead. This number tells the program: "If a matrix (or sub-matrix) is smaller than this threshold, just use the simpler 'naive' multiplication, as Strassen won't be worth it."
      * **Recommendation**: A value like `64` or `128` is often a good starting point.
      * **`0`**: If you enter `0`, the program will *only* use the 'naive' method (good for comparing performance or if Strassen causes issues).
      * **High Value**: If you set a value larger than your matrix size, it will also use the 'naive' method.
  * **Threads to Use**:
      * **What it is**: How many CPU cores the program should try to use simultaneously.
      * **`0` (Auto)**: This is usually the **best choice**. The program will use all your available CPU cores for maximum speed.
      * **Specific Number (e.g., `4`)**: You can limit it if you need to reserve CPU power for other tasks, but it will likely run slower.

### 5\. Running and Understanding the Output

Once you confirm the settings, the multiplication begins\!

  * **Progress Bar**: For larger matrices using Strassen's, you'll see a live progress bar showing how much work is done.
  * **Completion**: You'll hear a 'beep' sound when it finishes.
  * **Results Box**: This shows key statistics:
      * Input/Output Dimensions.
      * Execution Time (in seconds).
      * Threads Used.
      * Peak Memory Usage (in MB).
      * Whether Strassen's algorithm was actually used at the top level.
  * **Detailed Timings Chart**: This breaks down *where* the time was spent (padding, splitting, calculations, combining). It's great for understanding performance bottlenecks, especially when using Strassen's.

### 6\. Saving the Result

Finally, you'll be asked if you want to save the resulting Matrix C to a file. Type `y` and provide a filename if you want to keep it.

-----

## ⚖️ MATRIX COMPARISON

This feature checks two matrices (A and B) element by element to see how many values match.

### 1\. Setting Dimensions

You'll enter the dimensions for both matrices:

```
Matrix 1 - Rows: [Enter number]
Matrix 1 - Cols: [Enter number]
Matrix 2 - Rows: [Enter number]
Matrix 2 - Cols: [Enter number]
```

  * **Important:** For comparison, Matrix 1 and Matrix 2 *must* have the **exact same dimensions**.

### 2\. Memory Estimation

Similar to multiplication, you'll see an estimated RAM usage. Keep an eye on this for large matrices.

### 3\. Inputting Matrices

You have two main ways to get the matrices you want to compare:

1.  **Random Generation (Identical Seeds)**:
      * **Best for**: Testing the comparison feature itself or creating a baseline where matrices *should* match.
      * **How**: Select `1`. The program will create *two identical* random matrices. It might add a *tiny* difference to one element just to show how Epsilon (see below) works.
2.  **Read from File**:
      * **Best for**: Comparing your own datasets, or comparing a calculated result (from multiplication) against an expected result.
      * **How**: Select `2`. Provide the filenames for both matrices.

### 4\. Comparison Settings

  * **Comparison Threshold**:
      * **What it is**: Similar to the Strassen threshold, this controls when the program switches from a recursive comparison (faster for large matrices) to a simple, direct comparison.
      * **Recommendation**: `64` or `128` is usually fine. `0` forces a simple, non-recursive comparison.
  * **Epsilon for float compare**:
      * **What it is**: Computers sometimes have tiny rounding differences when dealing with decimal numbers (floats/doubles). Epsilon is a *tolerance* level.
      * **`0` (Exact Match)**: If you enter `0`, elements must be *perfectly identical* to match.
      * **Small Value (e.g., `1e-9` or `0.000000001`)**: This is the **recommended** setting when comparing results from calculations. It means "count elements as matching if their difference is *smaller* than this tiny number". This avoids issues with minor rounding discrepancies.
  * **Threads to Use**:
      * Same as in multiplication. `0` for Auto (recommended) is usually best.

### 5\. Interpreting the Results

The comparison runs (usually much faster than multiplication) and then displays the **Comparison Results**:

  * **Dimensions** and **Total Elements**.
  * **Matching Elements**: How many values were considered the same (respecting the Epsilon you set).
  * **Mismatching Elements**: How many values were different.
  * **Match Percentage**: A quick look at how similar the matrices are.
  * **Execution Time**, **Threads Used**, and **Peak Memory**.

-----

## ⚙️ SETTINGS & CONTROLS

Here's a quick summary of the main controls and when you might want to change them:

  * **Matrix Dimensions**:
      * **Impact**: Directly affects processing time and memory usage. Larger = Slower & More Memory.
      * **When to Change**: Set based on your specific task or testing goals.
  * **Input Method**:
      * **Impact**: Determines how data gets into the program.
      * **When to Change**: Use 'Random' for speed tests, 'File' for real data, 'Console' *only* for tiny examples.
  * **Strassen/Comparison Threshold**:
      * **Impact**: Affects *how* the core algorithm works, balancing recursion overhead vs. base case speed.
      * **When to Change**: Generally, stick to defaults (`64`/`128`) unless you're specifically profiling or troubleshooting. Use `0` to force 'naive' methods.
  * **Threads**:
      * **Impact**: Controls parallelism (speed).
      * **When to Change**: Almost always use `0` (Auto). Only limit it if the program is consuming too many CPU resources needed by other applications.
  * **Epsilon (Comparison Only)**:
      * **Impact**: Defines how strict the "match" criteria are.
      * **When to Change**: Use `0` only if you need *absolute bit-for-bit* identity. Use a small positive value (e.g., `1e-9`) when comparing results of floating-point math to allow for rounding.
  * **Logging (y/n)**:
      * **Impact**: Saves (or doesn't save) performance data.
      * **When to Change**: Enable it when you need to track, compare, or report on multiple runs.

-----

## 📌 TIPS, WARNINGS, AND  PRACTICES


> [!IMPORTANT]
>  * **💾 Memory is Key**: The biggest limitation is often RAM. If you try to process matrices that require more RAM than you have available, performance will suffer drastically, or the program will crash. **Always check the memory estimate\!**

> [!IMPORTANT]
>  * **📏 Dimensions Matter**: Double-check your dimensions *before* starting. For multiplication, `A.cols` must equal `B.rows`. For comparison, all dimensions must match.

> [!TIP]
>  * **⌨️ Avoid Manual Input for Large Matrices**: It's extremely time-consuming and error-prone. Use file input or random generation instead.

> [!NOTE]
>  * **📄 File Format**: Ensure your input files are correctly formatted (plain text, numbers separated by spaces or commas, correct number of rows/columns). Malformed files will cause errors.

> [!TIP]
>  * **⏱️ Threshold Tuning**: While defaults are good, experimenting with the Strassen threshold *might* yield slightly better performance for specific matrix sizes on *your* specific hardware, but often the difference isn't huge.

> [!WARNING]
>  * **🔬 Epsilon Choice**: When comparing matrices that result from floating-point calculations (like multiplication), *always* use a small positive Epsilon (like `1e-9`). Expecting exact `0` matches is often unrealistic due to how computers handle decimals.

> [!NOTE]
>  * **🔄 Large Files**: Reading very large matrix files can take time. You'll see a spinner `| / - \` while it reads – be patient\!

> [!CAUTION]
>  * **⚠️ Crash/Freeze**: The most likely cause is running out of memory. If the app freezes or crashes, try again with smaller matrices. If an error message appears (like "bad\_alloc" or another "Exception"), it usually points towards a memory issue or an input problem (like bad file format or mismatched dimensions).

-----

We hope this guide helps you make the most of Fluminum. Happy matrix crunching\! 🚀
