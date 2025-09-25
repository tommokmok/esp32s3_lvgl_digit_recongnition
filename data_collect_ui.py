import tkinter as tk
from tkinter import ttk, messagebox
import threading
import serial
import serial.tools.list_ports
import numpy as np
from PIL import Image
from pathlib import Path

class SerialThread(threading.Thread):
    def __init__(self, ser, data_type, callback):
        super().__init__()
        self.ser = ser
        self.data_type = data_type
        self.callback = callback
        self.running = True
        self.buffer = ""
        self.count=0

    def run(self):
        while self.running:
            try:
               
                data = self.ser.read(1024)
                if data:
                    char = data.decode(errors='ignore')
                    self.buffer += char
                    
                    if data[-1] == 0x0a: 

                        # print(f"Full buffer before processing: {self.buffer}")  # Debug print to trace full buffer
                        parts = self.buffer.split(',')
                      
                        if parts[0] == "START" and parts[-1] == "END\r\n":
                            # print(f"Processing the data")
                            raw_data = parts[1:-1] # Notes: Not inclue the last one
                            ret=self.save_as_png_if_valid(raw_data)
                            if ret!=0:
                                self.buffer = ""
                                continue
                            self.callback(raw_data)
                            # print(f"Received raw data: {','.join(raw_data)}")
                            self.buffer = ""
            except Exception:
                pass
    def save_as_png_if_valid(self, str_list):
        """
        Convert string list to integer array and save as PNG if size is 750 (30x25).
        """
        if len(str_list) != 750:
            print(f"Data size is not 750, got {len(str_list)}")
            return -1
        try:
            # Dynamically get the latest data_type from the UI spinbox
            if hasattr(self.callback, '__self__') and hasattr(self.callback.__self__, 'data_type'):
                self.data_type= self.callback.__self__.data_type.get()
                
            # Dynamically get the latest count from the UI
            if hasattr(self.callback, '__self__') and hasattr(self.callback.__self__, 'packet_count'):
                self.count= self.callback.__self__.packet_count
            
            int_arr = [int(x) for x in str_list]
            #Scale 0->0, 1->255 for visibility
            img_arr = (np.array(int_arr, dtype=np.uint8) * 255).reshape((25, 30))
          
            img = Image.fromarray(img_arr)
            if img.mode != "L":
                img = img.convert("L")

            image_path=Path(f"./models/dataset/{self.data_type}")
            if not image_path.exists():
                image_path.mkdir(parents=True, exist_ok=True)
            
            img.save(f"./models/dataset/{self.data_type}/_{self.count}.png")
            print("Saved output.png")
            self.count+=1
        except Exception as e:
            print(f"Failed to convert/save PNG: {e}")   
            return -1        
        return 0 
    def stop(self):
        self.running = False

class SerialUI:
    def __init__(self, root):
        self.root = root
        self.root.title("COM Port Data Connect UI")
        self.ser = None
        self.serial_thread = None

        # Add entry for starting packet_count
        self.packet_count_label = ttk.Label(root, text="Start Packet Count:")
        self.packet_count_label.grid(row=0, column=4, padx=5, pady=5)
        self.packet_count_var = tk.IntVar(value=0)
        self.packet_count_entry = ttk.Entry(root, textvariable=self.packet_count_var, width=8)
        self.packet_count_entry.grid(row=0, column=5, padx=5, pady=5)

        self.packet_count = self.packet_count_var.get()  # Counter for received packets

        self.com_label = ttk.Label(root, text="Select COM Port:")
        self.com_label.grid(row=0, column=0, padx=5, pady=5)
        self.combobox = ttk.Combobox(root, values=self.get_com_ports(), state="readonly")
        self.combobox.grid(row=0, column=1, padx=5, pady=5)

        # Add refresh button for COM ports
        self.refresh_btn = ttk.Button(root, text="Refresh", command=self.refresh_com_ports)
        self.refresh_btn.grid(row=0, column=2, padx=5, pady=5)

        self.data_label = ttk.Label(root, text="Select Data Type (0-9):")
        self.data_label.grid(row=1, column=0, padx=5, pady=5)
        self.data_type = tk.IntVar(value=0)
        self.data_spinbox = ttk.Spinbox(root, from_=0, to=9, textvariable=self.data_type, width=5)
        self.data_spinbox.grid(row=1, column=1, padx=5, pady=5)

        self.connect_btn = ttk.Button(root, text="Connect", command=self.connect)
        self.connect_btn.grid(row=2, column=0, padx=5, pady=5)
        self.disconnect_btn = ttk.Button(root, text="Disconnect", command=self.disconnect, state="disabled")
        self.disconnect_btn.grid(row=2, column=1, padx=5, pady=5)

        # Packet counter label and clear button
        self.counter_label = ttk.Label(root, text=f"Packets Received: {self.packet_count}")
        self.counter_label.grid(row=2, column=2, padx=5, pady=5)
        self.clear_counter_btn = ttk.Button(root, text="Clear Counter", command=self.clear_counter)
        self.clear_counter_btn.grid(row=2, column=3, padx=5, pady=5)

        self.data_display = tk.Text(root, height=20, width=60, state="disabled", wrap="none")
        self.data_display.grid(row=3, column=0, columnspan=6, padx=5, pady=5)

        self.scroll_x = tk.Scrollbar(root, orient="horizontal", command=self.data_display.xview)
        self.data_display.configure(xscrollcommand=self.scroll_x.set)
        self.scroll_x.grid(row=4, column=0, columnspan=6, sticky="ew")

    def get_com_ports(self):
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def refresh_com_ports(self):
        ports = self.get_com_ports()
        self.combobox['values'] = ports
        if ports:
            self.combobox.current(0)
        else:
            self.combobox.set('')

    def connect(self):
        # Update packet_count from entry before connecting
        try:
            self.packet_count = int(self.packet_count_var.get())
        except Exception:
            self.packet_count = 0
            self.packet_count_var.set(0)
        port = self.combobox.get()
        if not port:
            messagebox.showerror("Error", "Please select a COM port.")
            return
        try:
            self.ser = serial.Serial(port, baudrate=115200, timeout=0.1)
            self.serial_thread = SerialThread(self.ser, self.data_type.get(), self.display_data)
            self.serial_thread.daemon = True
            self.serial_thread.start()
            self.connect_btn.config(state="disabled")
            self.disconnect_btn.config(state="normal")
            self.data_display.config(state="normal")
            self.data_display.insert(tk.END, f"Connected to {port}\n")
            self.data_display.config(state="disabled")
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))

    def disconnect(self):
        if self.serial_thread:
            self.serial_thread.stop()
            self.serial_thread.join()
        if self.ser:
            self.ser.close()
        self.connect_btn.config(state="normal")
        self.disconnect_btn.config(state="disabled")
        self.data_display.config(state="normal")
        self.data_display.insert(tk.END, "Disconnected.\n")
        self.data_display.config(state="disabled")

    def clear_counter(self):
        self.packet_count = 0
        self.packet_count_var.set(0)
        self.counter_label.config(text=f"Packets Received: {self.packet_count}")

    def display_data(self, raw_data):
        self.packet_count += 1
        self.counter_label.config(text=f"Packets Received: {self.packet_count}")
        self.data_display.config(state="normal")
        self.data_display.insert(tk.END, f"Data Received\n")
        self.data_display.see(tk.END)
        self.data_display.config(state="disabled")

if __name__ == "__main__":
    root = tk.Tk()
    app = SerialUI(root)
    root.mainloop()