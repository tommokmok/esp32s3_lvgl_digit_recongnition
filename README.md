# ESP32-S3 Digit Recognition in LVGL -- Testing Branch

## Development Timeline

| Date       | Milestone/Note   |
|------------|------------------|
| 2025-09-15 | Add branch test/dataset_create |

## Introduction

This branch contains testing code and scripts for data collection and model evaluation.


The main features are:

1. Python GUI applciatoin (`data_collect_ui.py`) for capture the data (pixel array) from ESP32s3 via UART and convert it to png file which use for further traingin.
2. Python script (`c_code_generate.py`) for convert the captured data to C array code for testing the model performance in ESP32s3


## How This branch May Help You

- Please check `SerialThread->run` if you also want to parsed some data via uart with simple protocol
- Please check the below promt for AI code generation example.

- The provided Python scripts are simple and generated with the help of AI.
- For `data_collect_ui.py`, the core part is `SerialThread->run`.  I spent some time figuring out the correct way to parse the incoming data.
- For `c_code_generate.py`, VS Code Copilot (GPT-4.1) generated code that worked perfectly without modification.


**Prompt for `data_collect_ui.py`:**

```text

Please create a python code  with ui which using com port for data connection with  following requirement: 1. Have com-port selection 2. have button to connect a com-port 3. have button to disconnect from the com port 3. using tkthinker ui framework 4. using thread for the ui so that ui not struck if data is in processing. 5.have a data selection of 0 to 9 which indicate what kind of data will recieved from uart. 6. The data input is in a format which start with a string "START" , a series of raw data of integer value in string, and end with a string "END\r\n". The delimeter is a comma ",". e.g. "START,0,0,0,0,0,1,END\r\n". 7 program will extract the raw data between "START" and "END"

```

**Prompt for `c_code_generate.py`:**

Here are the prompt for the `c_code_generate.py`

```text

please make a python script for geneate c code from the models/dataset. The foder name is the predicted digit value, inside the folder is the png files . Please convert the png file to a c array. The png file size is 30 col and 25 row.SO totoal size of the array is 750. The generated c file should have .h and .c file. THe scitpt should count the number of png file in each digit folder. The generated c file is using for test the performane of the digit recongition nenural network in esp32. there are totol 9 data set that will pass to the esp32 and let it to predict if it really predic the correct digits

```


For the detail implementation notes, please check on my blog: https://tommokmok.github.io/2025/09/15/Digit-Recongition-Optimization-Seires-00-Testing/




## Screenshots


<a href="https://imgbb.com/"><img src="https://i.ibb.co/cKd0G8rk/data-collect-gui.png" alt="data-collect-gui" border="0"></a>

## Data collection flow

1. ESP32s3 connect to the PC
2. In GUI, select and connect the COM port
3. Select the data type that will be recieved from ESP32
4. In the devcie, draw the digit then click `Predict`
5. Data then will send to GUI and it will convert the data and save as PNG file under the folder `model\dataset\x` where x depends on what digit is selected in the GUI.


## Test result

Self created model

<a href="https://imgbb.com/"><img src="https://i.ibb.co/SXFztmDC/test-result.png" alt="test-result" border="0"></a>

Original model

<a href="https://imgbb.com/"><img src="https://i.ibb.co/vKGz3KB/test-result-original-model.png" alt="test-result-original-model" border="0"></a>

## Conclusion

Both model are not good enough. So I will collect 2000 data. 200 for each digits and train the model again. Let's see what accuracy I can get.











