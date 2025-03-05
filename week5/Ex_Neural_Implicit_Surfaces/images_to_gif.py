import cv2
import imageio
import glob
import os

# Load images
no_images = len(glob.glob(os.path.join(os.getcwd(),"images","*.png")))
image_files = []
for ii in range(no_images):
    image_files.append(os.path.join(os.getcwd(),"images","fig-" + str(ii) + ".png"))

# Define cropping coordinates (y1:y2, x1:x2)
x1, y1, x2, y2 = 300, 100, 1600, 1300  # Adjust as needed

frames = []
for img_path in image_files:
    img = cv2.imread(img_path)  # Read image
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)  # Convert BGR to RGB
    #img = img[y1:y2, x1:x2]  # Crop the image
    frames.append(img)  # Add to frames list

# Save as GIF
imageio.mimsave("output_interpolation.gif", frames, duration=0.25)
