import threading
import tkinter as tk
from tkinter import ttk

import bluetooth
import pyautogui
from PIL import Image

from custom_window import Custom_Window

socket = ""


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

    for key in ["<Left>", "<Right>"]:
        mainWindow._root.bind(key, mainWindow.on_arrow_press)

    mainWindow.run()


def keyEvent(event):
    if event & 1:
        print("Received: 1")
        pyautogui.press("space")  # Send a space key press event
    if (event >> 1) & 1:
        print("Received: 2")
        pyautogui.press("right")
    if (event >> 2) & 1:
        print("Received: 4 ")
        pyautogui.press("left")


def receive_data():
    buffer = []
    while True:
        data = socket.recv(1024)
        if b"\n" in data:
            buffer.append(data[:-1])
            print(f"Received: {buffer}")
            buffer = [
                int.from_bytes(item, byteorder="big") for item in buffer if item != b""
            ]
            print(f"Received: {buffer}")
            if not buffer[0] == 255:
                keyEvent(buffer[0])
            buffer.clear()
        else:
            buffer.append(data)


def on_device_click(root, addr):
    global socket

    # connect to the arduino
    socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    socket.connect((addr, 1))

    socket.send("753")  # tell arduino start flag
    print(f'Connecting to "{addr}"...')

    buffer = []
    while True:
        if not root.winfo_exists():
            break

        data = socket.recv(2)
        if b"\n" in data:
            buffer.append(data[:-1])
            buffer = [
                int.from_bytes(item, byteorder="big") for item in buffer if item != b""
            ]
            print(f"Received: {buffer}")
            if buffer[0] == 255:
                th = threading.Thread(target=receive_data)
                th.daemon = True
                th.start()
                create_main_window(root)
            buffer.clear()
        else:
            buffer.append(data)


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

    # Create a button for each device
    for idx, (addr, name) in enumerate(devices):
        button = tk.Button(
            root,
            text=name,
            padx=10,
            pady=10,
            command=lambda device=name: on_device_click(root, addr),
        )
        button.place(relx=0.5, rely=0.5 + idx * 0.1, anchor="center")


def main():
    # Create a Tkinter window
    root = tk.Tk()
    root.minsize(800, 600)
    root.configure(bg="black")

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

    # Start the Tkinter event loop
    root.mainloop()


if __name__ == "__main__":
    main()
