import threading
import time
import tkinter as tk
from tkinter import ttk

import bluetooth
import pyautogui
import serial
import serial.tools.list_ports
from PIL import Image

from custom_window import Custom_Window


# for joystick command mapping
def keyEvent(event):
    if event & 1:
        print("Received: 1 :: space")
        pyautogui.press("space")
    if (event >> 1) & 1:
        print("Received: 2 :: right")
        pyautogui.press("right")
    if (event >> 2) & 1:
        print("Received: 4 :: left")
        pyautogui.press("left")
    if (event >> 3) & 1:
        print("Received: 8 :: down")
        pyautogui.press("down")
    if (event >> 4) & 1:
        print("Received: 16 :: up")
        pyautogui.press("up")


# for reading joystick commands from arduino
def read_from_arduino(protName):
    arduino = serial.Serial(port=protName, baudrate=115200, timeout=0.1)
    while True:
        data = arduino.readline()
        if data != b"":
            keyEvent(int(data.decode("utf-8")))
        time.sleep(0.1)


# create indoor layout window
def create_main_window(root):
    global socket

    root.destroy()  # Destroy the BT device selecting window (first window)

    # laod the indoor layout images
    images = ["img/top1.png", "img/bottom.png", "img/right1.png"]
    imgList = [Image.open(img) for img in images]

    # call custom window class to create a new window (second window)
    mainWindow = Custom_Window(
        "Smart Home",
        imgList,
        socket=socket,
    )

    #  assign blink state to the top grid
    mainWindow.gridDict["top"] = "blink"
    mainWindow.blink(grid="top", alpha=0.1, increment=0.02)

    # bind key events to the window
    mainWindow._root.bind("<space>", mainWindow.on_space_press)

    for key in ["<Left>", "<Right>", "<Up>", "<Down>"]:
        mainWindow._root.bind(key, mainWindow.on_arrow_press)

    mainWindow.run()  # keep the window alive


def on_device_click(root, addr):
    global socket

    # create bt socket and connect to the device
    socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    socket.connect((addr, 1))

    create_main_window(root)


def discover_devices(root, progress, label):
    # discover Bluetooth devices
    devices = bluetooth.discover_devices(lookup_names=True, duration=8)

    # destroy the elements about searching devices
    label.destroy()
    progress.stop()
    progress.destroy()

    # create a label to show the found bt devices
    label = tk.Label(root, text="Found devices\n", bg="black", fg="white")
    label.place(relx=0.5, rely=0.45, anchor="center")

    # hold the index of the selected button
    buttons = []
    selected_button_index = tk.IntVar(root)
    selected_button_index.set(0)

    # create a button for each device
    for idx, (addr, name) in enumerate(devices):
        button = tk.Button(
            root,
            text=name,
            padx=10,
            pady=10,
            command=lambda device=name: on_device_click(root, addr),
        )
        button.place(relx=0.5, rely=0.5 + idx * 0.1, anchor="center")
        buttons.append(button)

    # update the selected button when the up arrow key is pressed
    def on_up_arrow_press(event):
        old_index = selected_button_index.get()
        selected_button_index.set((old_index - 1) % len(buttons))

        # reset the color of old button
        buttons[old_index].config(bg="SystemButtonFace")

        # change the color of selected button
        buttons[selected_button_index.get()].config(bg="#e0d312")

        buttons[selected_button_index.get()].focus()

    # update the selected button when the down arrow key is pressed
    def on_down_arrow_press(event):
        old_index = selected_button_index.get()
        selected_button_index.set((old_index + 1) % len(buttons))

        # reset the color of old button
        buttons[old_index].config(bg="SystemButtonFace")

        # change the color of selected button
        buttons[selected_button_index.get()].config(bg="#e0d312")

        buttons[selected_button_index.get()].focus()

    # bind the keys to the functions
    root.bind("<Up>", on_up_arrow_press)
    root.bind("<Down>", on_down_arrow_press)


def on_port_click(root, protName, label, frame):
    # create a thread to read joystick commands from Arduino
    th = threading.Thread(target=read_from_arduino, args=(protName,))
    th.daemon = True
    th.start()

    # destroy the elements about COM prot selecting
    label.destroy()
    frame.destroy()

    # create a label to show the "Searching device" message
    label = tk.Label(root, text="Searching device", bg="black", fg="white")
    label.place(relx=0.5, rely=0.45, anchor="center")

    # create a label to show the progress animation
    progress = ttk.Progressbar(
        root,
        mode="indeterminate",
        length=100,
        maximum=100,
        style="custom.Horizontal.TProgressbar",
    )
    progress.place(relx=0.5, rely=0.5, anchor="center")
    progress.start(2)

    # create a thread to discover bt devices
    th = threading.Thread(target=discover_devices, args=(root, progress, label))
    th.daemon = True
    th.start()


def main():
    # create a Tkinter window
    root = tk.Tk()
    root.minsize(800, 600)
    root.configure(bg="black")

    # create a label to show the "Select ports" message
    label = tk.Label(root, text="Select ports\n", bg="black", fg="white")
    label.place(relx=0.5, rely=0.3, anchor="center")

    # create a frame to hold all COM port buttons
    frame = tk.Frame(root, bg="black")
    frame.place(relx=0.5, rely=0.4, anchor="center")

    # discover COM ports
    ports = serial.tools.list_ports.comports()

    # create a button for each port
    for port in ports:
        button = tk.Button(
            frame,
            text=port.device,
            padx=10,
            pady=10,
            command=lambda port=port: on_port_click(root, port.name, label, frame),
        )
        button.pack(padx=5, pady=5)

    root.mainloop()  # keep this window alive


if __name__ == "__main__":
    main()
    # input("Press Enter to continue...")
