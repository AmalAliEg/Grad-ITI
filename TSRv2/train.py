import numpy as np
import pandas as pd
from PIL import Image
from sklearn.model_selection import train_test_split
import os
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Conv2D, MaxPooling2D, Flatten, Dense, Dropout

# Load the dataset
df = pd.read_csv(r"/home/aml/Desktop/GSTRB_dataset/Train.csv")
image_paths = df['Path'].values

# Define function to load and preprocess images
def load_image(img_path, target_size=(32, 32)):
    try:
        with Image.open(img_path) as img:
            img = img.convert("RGB")
            img = img.resize(target_size)
            return np.array(img)
    except Exception as e:
        print(f"Error loading image {img_path}: {e}")
        return None

# Load images and labels
x_data = []
y_data = []
for img_path, label in zip(image_paths, df['ClassId']):
    full_path = os.path.join(r"C/home/aml/Desktop/GSTRB_dataset", img_path)
    img = load_image(full_path)
    if img is not None:
        x_data.append(img)
        y_data.append(label)

# Convert data to numpy arrays
x_data = np.array(x_data)
y_data = np.array(y_data)

# Split the dataset
x_train, x_test, y_train, y_test = train_test_split(x_data, y_data, test_size=0.2, random_state=42)

# Normalize pixel values
x_train = x_train.astype("float32") / 255.0
x_test = x_test.astype("float32") / 255.0

# Define the model architecture
model = Sequential([
    Conv2D(32, (3, 3), activation='relu', input_shape=(32, 32, 3)),
    MaxPooling2D(),
    Conv2D(64, (3, 3), activation='relu'),
    MaxPooling2D(),
    Flatten(),
    Dense(128, activation='relu'),
    Dropout(0.5),
    Dense(43, activation='softmax')
])

# Compile the model
model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])

# Train the model
history = model.fit(x_train, y_train, epochs=20, batch_size=32, validation_split=0.2)

# Evaluate the model
test_loss, test_acc = model.evaluate(x_test, y_test)
print("Test Accuracy:", test_acc)

# Save the model and its weights
model.save('TSRS_model.h5')
model.save_weights("TSRS_weights.weights.h5")