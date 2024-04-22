import tkinter as tk
from PIL import Image, ImageTk
import pyautogui


class Custom_Window:
    def __init__(self, title, imgList, socket) -> None:
        self._targetGrid = "top"
        self.gridDict = {"top": "off", "bottom": "off", "right": "off"}
        self._gridList = ["top", "bottom", "right"]
        self.socket = socket

        self._root = tk.Tk()

        # Set window title
        self._root.title(title)

        # Set minimum window size
        self._root.minsize(800, 600)

        # Set window background color to black
        self._root.configure(bg="black")

        self._root.lift()
        self._root.focus_force()

        # Create a frame for the left part of the window
        self._leftFrame = tk.Frame(self._root, bg="black")
        self._leftFrame.grid(row=0, column=0, rowspan=2, sticky="nsew")

        # Load the images
        self._topImg, self._bottomImg, self._rightImg = imgList

        # Create semi-transparent masks
        self._topImgMask = ImageTk.PhotoImage(
            Image.blend(
                self._topImg.convert("RGB"),
                Image.new("RGB", self._topImg.size, (0, 0, 0)),
                alpha=0.8,
            )
        )
        self.bottomImgMask = ImageTk.PhotoImage(
            Image.blend(
                self._bottomImg.convert("RGB"),
                Image.new("RGB", self._bottomImg.size, (0, 0, 0)),
                alpha=0.8,
            )
        )
        self._rightImgMask = ImageTk.PhotoImage(
            Image.blend(
                self._rightImg.convert("RGB"),
                Image.new("RGB", self._rightImg.size, (0, 0, 0)),
                alpha=0.8,
            )
        )

        # Create two labels in the left frame with images as background
        self._topLabel = tk.Label(self._leftFrame, image=self._topImgMask, bg="black")
        self._topLabel.grid(row=0, column=0, sticky="nsew")

        self._bottomLabel = tk.Label(
            self._leftFrame, image=self.bottomImgMask, bg="black"
        )
        self._bottomLabel.grid(row=1, column=0, sticky="nsew")

        # Create a label for the right part of the window with image as background
        self._rightLabel = tk.Label(self._root, image=self._rightImgMask, bg="black")
        self._rightLabel.grid(row=0, column=1, rowspan=2, sticky="nsew")

        # Keep a reference to the PhotoImage objects
        self._root._topImgMask = self._topImgMask
        self._root.bottomImgMask = self.bottomImgMask
        self._root._rightImgMask = self._rightImgMask

    def run(self):
        # Additional setup can go here
        self._root.mainloop()

    # Function to create a fade in and fade out effect
    def blink(self, grid, alpha, increment):
        if self.gridDict[grid] == "blink":
            self._targetGrid = grid

            # Create a dictionary to map the grid names to the corresponding labels and images
            grids = {
                "top": (self._topLabel, self._topImg),
                "bottom": (self._bottomLabel, self._bottomImg),
                "right": (self._rightLabel, self._rightImg),
            }

            # Get the label and image for the given grid
            gridLabel, gridImg = grids[grid]

            # Create the image mask
            gridImgMask = ImageTk.PhotoImage(
                Image.blend(
                    gridImg.convert("RGB"),
                    Image.new("RGB", gridImg.size, (0, 0, 0)),
                    alpha,
                )
            )

            gridLabel.configure(image=gridImgMask)
            gridLabel.image = gridImgMask
            if alpha >= 0.8:
                increment = -0.02
            elif alpha <= 0.1:
                increment = 0.02
            self._root.after(8, self.blink, grid, alpha + increment, increment)

    def always_on(self, grid):
        self._targetGrid = grid

        # Create a dictionary to map the grid names to the corresponding labels and images
        grids = {
            "top": (self._topLabel, self._topImg),
            "bottom": (self._bottomLabel, self._bottomImg),
            "right": (self._rightLabel, self._rightImg),
        }

        # Get the label and image for the given grid
        gridLabel, gridImg = grids[grid]

        # Create the image mask
        gridImgMask = ImageTk.PhotoImage(
            Image.blend(
                gridImg.convert("RGB"),
                Image.new("RGB", gridImg.size, (0, 0, 0)),
                alpha=0,
            )
        )

        gridLabel.configure(image=gridImgMask)
        gridLabel.image = gridImgMask

    def always_off(self, grid):
        # Create a dictionary to map the grid names to the corresponding labels and images
        grids = {
            "top": (self._topLabel, self._topImg),
            "bottom": (self._bottomLabel, self._bottomImg),
            "right": (self._rightLabel, self._rightImg),
        }

        # Get the label and image for the given grid
        gridLabel, gridImg = grids[grid]
        print(f"grid: {grid}")
        # Create the image mask
        gridImgMask = ImageTk.PhotoImage(
            Image.blend(
                gridImg.convert("RGB"),
                Image.new("RGB", gridImg.size, (0, 0, 0)),
                alpha=0.8,
            )
        )

        gridLabel.configure(image=gridImgMask)
        gridLabel.image = gridImgMask

    def selectd_effect(self, alpha, increment, counter, state):
        if counter > 0:
            if alpha <= 0:
                counter -= 1
            # Create a dictionary to map the grid names to the corresponding labels and images
            grids = {
                "top": (self._topLabel, self._topImg),
                "bottom": (self._bottomLabel, self._bottomImg),
                "right": (self._rightLabel, self._rightImg),
            }

            # Get the label and image for the given grid
            gridLabel, gridImg = grids[self._targetGrid]

            # Set the color based on the state
            color = (52, 209, 107) if state == "on" else (245, 66, 96)

            # Create the image mask
            gridImgMask = ImageTk.PhotoImage(
                Image.blend(
                    gridImg.convert("RGB"),
                    Image.new("RGB", gridImg.size, color),
                    alpha,
                )
            )

            gridLabel.configure(image=gridImgMask)
            gridLabel.image = gridImgMask
            if alpha >= 0.5:
                increment = -0.02
            elif alpha <= 0:
                increment = 0.02
            self._root.after(
                3, self.selectd_effect, alpha + increment, increment, counter, state
            )

    def on_space_press(self, event):
        if self.gridDict[self._targetGrid] == "on":
            self.gridDict[self._targetGrid] = "blink"
            self.blink(grid=self._targetGrid, alpha=0.1, increment=0.02)

        elif self.gridDict[self._targetGrid] == "blink":
            self.gridDict[self._targetGrid] = "on"
            self.always_on(self._targetGrid)

    def on_arrow_press(self, event):
        if "on" not in self.gridDict.values():
            if event.keysym == "Left":
                self.gridDict[self._targetGrid] = "off"
                self.always_off(self._targetGrid)
                if self._gridList.index(self._targetGrid) == 0:
                    self._targetGrid = self._gridList[-1]
                else:
                    self._targetGrid = self._gridList[
                        self._gridList.index(self._targetGrid) - 1
                    ]

                self.gridDict[self._targetGrid] = "blink"
                self.blink(grid=self._targetGrid, alpha=0.1, increment=0.02)

                print("Left arrow key pressed")

            elif event.keysym == "Right":
                self.gridDict[self._targetGrid] = "off"
                self.always_off(self._targetGrid)
                if self._gridList.index(self._targetGrid) == 2:
                    self._targetGrid = self._gridList[0]
                else:
                    self._targetGrid = self._gridList[
                        self._gridList.index(self._targetGrid) + 1
                    ]

                self.gridDict[self._targetGrid] = "blink"
                self.blink(grid=self._targetGrid, alpha=0.1, increment=0.02)

                print("Right arrow key pressed")
        else:
            if event.keysym == "Up":
                if self._targetGrid == "top":
                    self.socket.send("13")
                    print("command: 13")
                    self.selectd_effect(0.1, 0.02, 1, "on")
                elif self._targetGrid == "bottom":
                    pyautogui.press("volumeup")
                    self.selectd_effect(0.1, 0.02, 1, "on")
                elif self._targetGrid == "right":
                    self.socket.send("23")
                    print("command: 23")
                    self.selectd_effect(0.1, 0.02, 1, "on")
            elif event.keysym == "Down":
                if self._targetGrid == "top":
                    self.socket.send("17")
                    print("command: 17")
                    self.selectd_effect(0.1, 0.02, 1, "off")
                elif self._targetGrid == "bottom":
                    pyautogui.press("volumedown")
                    self.selectd_effect(0.1, 0.02, 1, "off")
                elif self._targetGrid == "right":
                    self.socket.send("29")
                    print("command: 29")
                    self.selectd_effect(0.1, 0.02, 1, "off")
