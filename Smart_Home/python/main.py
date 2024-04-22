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


def keyEvent(event):
    if event & 1:
        print("Received: 1, space")
        pyautogui.press("space")  # Send a space key press event
    if (event >> 1) & 1:
        print("Received: 2, right")
        pyautogui.press("right")
    if (event >> 2) & 1:
        print("Received: 4, left")
        pyautogui.press("left")
    if (event >> 3) & 1:
        print("Received: 8, down")
        pyautogui.press("down")
    if (event >> 4) & 1:
        print("Received: 16, up")
        pyautogui.press("up")


def read_from_arduino(protName):
    arduino = serial.Serial(port=protName, baudrate=115200, timeout=0.1)
    while True:
        data = arduino.readline()
        if data != b"":
            keyEvent(int(data.decode("utf-8")))
        time.sleep(0.1)


def create_main_window(root):
    global socket
    # Destroy the current window
    root.destroy()

    # laod the images
    images = ["img/top1.png", "img/bottom.png", "img/right1.png"]
    imgList = [Image.open(img) for img in images]

    mainWindow = Custom_Window(
        "Smart Home",
        imgList,
        socket=socket,
    )
    mainWindow.gridDict["top"] = "blink"
    mainWindow.blink(grid="top", alpha=0.1, increment=0.02)

    mainWindow._root.bind("<space>", mainWindow.on_space_press)

    for key in ["<Left>", "<Right>", "<Up>", "<Down>"]:
        mainWindow._root.bind(key, mainWindow.on_arrow_press)

    mainWindow.run()


def on_device_click(root, addr):
    global socket

    # connect to the arduino
    socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    socket.connect((addr, 1))

    create_main_window(root)


def discover_devices(root, progress, label):
    # Discover Bluetooth devices
    devices = bluetooth.discover_devices(lookup_names=True, duration=2)

    label.destroy()

    # Stop the animation
    progress.stop()
    progress.destroy()

    # Create a label to show the "Searching device" message
    label = tk.Label(root, text="Found devices\n", bg="black", fg="white")
    label.place(relx=0.5, rely=0.45, anchor="center")

    # Create a list to hold the buttons
    buttons = []

    # Create a variable to hold the index of the selected button
    selected_button_index = tk.IntVar(root)
    selected_button_index.set(0)

    # Create a button for each device
    for idx, (addr, name) in enumerate(devices):
        button = tk.Button(
            root,
            text=name if name != "" else addr,
            padx=10,
            pady=10,
            command=lambda device=name: on_device_click(root, addr),
        )
        button.place(relx=0.5, rely=0.5 + idx * 0.1, anchor="center")

        # Add the button to the list
        buttons.append(button)

    # Update the selected button when the up arrow key is pressed
    def on_up_arrow_press(event):
        old_index = selected_button_index.get()
        selected_button_index.set((old_index - 1) % len(buttons))
        buttons[old_index].config(bg="SystemButtonFace")  # Reset the old button's color
        buttons[selected_button_index.get()].config(
            bg="#e0d312"
        )  # Change the new button's color
        buttons[selected_button_index.get()].focus()

    # Update the selected button when the down arrow key is pressed
    def on_down_arrow_press(event):
        old_index = selected_button_index.get()
        selected_button_index.set((old_index + 1) % len(buttons))
        buttons[old_index].config(bg="SystemButtonFace")  # Reset the old button's color
        buttons[selected_button_index.get()].config(
            bg="#e0d312"
        )  # Change the new button's color
        buttons[selected_button_index.get()].focus()

    # Bind the keys to the functions
    root.bind("<Up>", on_up_arrow_press)
    root.bind("<Down>", on_down_arrow_press)


def on_port_click(root, protName, label, frame):
    print(f"on_port_click: {protName}")

    th = threading.Thread(target=read_from_arduino, args=(protName,))
    th.daemon = True
    th.start()

    label.destroy()
    frame.destroy()

    # Create a label to show the "Searching device" message
    label = tk.Label(root, text="Searching device", bg="black", fg="white")
    label.place(relx=0.5, rely=0.45, anchor="center")

    # Create a label to show the animation
    progress = ttk.Progressbar(
        root,
        mode="indeterminate",
        length=100,
        maximum=100,
        style="custom.Horizontal.TProgressbar",
    )
    progress.place(relx=0.5, rely=0.5, anchor="center")

    # Start the animation
    progress.start(2)

    # Discover Bluetooth devices in a separate thread
    th = threading.Thread(target=discover_devices, args=(root, progress, label))
    th.daemon = True
    th.start()


def main():
    # Create a Tkinter window
    root = tk.Tk()
    root.minsize(800, 600)
    root.configure(bg="black")

    # Create a label to show the "Searching device" message
    label = tk.Label(root, text="Select ports\n", bg="black", fg="white")
    label.place(relx=0.5, rely=0.3, anchor="center")

    # Create a frame to hold the buttons
    frame = tk.Frame(root, bg="black")
    frame.place(relx=0.5, rely=0.5, anchor="center")

    # Discover COM ports
    ports = serial.tools.list_ports.comports()

    # Create a button for each port
    for port in ports:
        button = tk.Button(
            frame,
            text=port.device,
            padx=10,
            pady=10,
            command=lambda port=port: on_port_click(root, port.name, label, frame),
        )
        button.pack(padx=5, pady=5)

    # Start the Tkinter event loop
    root.mainloop()


if __name__ == "__main__":
    main()
    # input("Press Enter to continue...")
