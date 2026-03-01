import os
import glob
import random
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import tensorflow as tf
from sklearn.model_selection import train_test_split
from sklearn.metrics import confusion_matrix

# =========================
# CONFIG
# =========================
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATASET_PATH = os.path.join(BASE_DIR, "data")
MODEL_DIR = os.path.join(BASE_DIR, "Models")
os.makedirs(MODEL_DIR, exist_ok=True)


FRAME_LEN = 52             # 1 second at 52Hz sampling rate
HOP_LEN = FRAME_LEN // 2       # 50% overlap
EPOCHS = 30

ACC_NORM = 4000.0           # mg for ±4g
GYRO_NORM = 500000.0       # mdps for ±500 dps (normalized to ±1.0)

# =========================
# LOAD LABELS
# =========================
labels = sorted([
    d for d in os.listdir(DATASET_PATH)
    if os.path.isdir(os.path.join(DATASET_PATH, d))
])

print("Detected labels:", labels)

x_recordings = []
y_recordings = []
recording_names = []

# =========================
# LOAD DATA
# =========================
for label_index, label in enumerate(labels):
    files = glob.glob(os.path.join(DATASET_PATH, label, "*.csv"))

    for file in files:
        df = pd.read_csv(file)

        data = df[[
            "acc_x", "acc_y", "acc_z",
            "gyro_x", "gyro_y", "gyro_z"
        ]].values

        x_recordings.append(data)
        y_recordings.append(label_index)
        recording_names.append(os.path.basename(file))

print("Total recordings:", len(x_recordings))

# =========================
# PLOT RANDOM SAMPLES
# =========================
random.seed(10)
plt.figure(figsize=(18, 10))

for i, n in enumerate(random.sample(range(len(x_recordings)), min(10, len(x_recordings)))):
    plt.subplot(5, 2, i + 1)
    plt.plot(x_recordings[n])
    plt.title(recording_names[n])
    plt.ylim(-ACC_NORM, ACC_NORM)

plt.tight_layout()
plt.show()

# =========================
# FRAMING FUNCTION
# =========================
def frame(x, frame_len, hop_len):
    n_frames = 1 + (x.shape[0] - frame_len) // hop_len
    shape = (n_frames, frame_len, x.shape[1])
    strides = (hop_len * x.strides[0], x.strides[0], x.strides[1])
    return np.lib.stride_tricks.as_strided(x, shape=shape, strides=strides)

# =========================
# FRAME ALL DATA
# =========================
x_frames = []
y_frames = []

for i in range(len(x_recordings)):
    frames = frame(x_recordings[i], FRAME_LEN, HOP_LEN)
    x_frames.append(frames)
    y_frames.append(np.full(frames.shape[0], y_recordings[i]))

x_frames = np.concatenate(x_frames)
y_frames = np.concatenate(y_frames)

print("Framed dataset shape:", x_frames.shape)

# =========================
# NORMALIZATION
# =========================
acc = x_frames[:, :, :3] / ACC_NORM
gyro = x_frames[:, :, 3:] / GYRO_NORM
x_frames = np.concatenate([acc, gyro], axis=2)

# =========================
# TRAIN TEST SPLIT
# =========================
x_train, x_test, y_train, y_test = train_test_split(
    x_frames, y_frames, test_size=0.25, stratify=y_frames
)

print("Training samples:", x_train.shape)
print("Testing samples:", x_test.shape)

# =========================
# MODEL
# =========================
model = tf.keras.Sequential([
    tf.keras.layers.Conv1D(16, 3, activation='relu', input_shape=(FRAME_LEN, 6)),
    tf.keras.layers.Conv1D(8, 3, activation='relu'),
    tf.keras.layers.Dropout(0.5),
    tf.keras.layers.Flatten(),
    tf.keras.layers.Dense(64, activation='relu'),
    tf.keras.layers.Dense(len(labels), activation='softmax')
])

model.compile(
    optimizer='adam',
    loss='sparse_categorical_crossentropy',
    metrics=['accuracy']
)

model.summary()

# =========================
# TRAIN
# =========================
history = model.fit(
    x_train, y_train,
    epochs=EPOCHS,
    validation_data=(x_test, y_test)
)

# =========================
# PLOT TRAINING CURVES
# =========================
plt.figure()
plt.plot(history.history['accuracy'], label='Train Accuracy')
plt.plot(history.history['val_accuracy'], label='Val Accuracy')
plt.legend()
plt.title("Accuracy")
plt.show()

plt.figure()
plt.plot(history.history['loss'], label='Train Loss')
plt.plot(history.history['val_loss'], label='Val Loss')
plt.legend()
plt.title("Loss")
plt.show()

# =========================
# CONFUSION MATRIX
# =========================
y_pred = np.argmax(model.predict(x_test), axis=1)
cm = confusion_matrix(y_test, y_pred)

plt.figure()
sns.heatmap(cm, annot=True, fmt="d",
            xticklabels=labels,
            yticklabels=labels,
            cmap="Blues",
            cbar=False)
plt.ylabel("True")
plt.xlabel("Predicted")
plt.tight_layout()
plt.show()

# =========================
# SAVE MODEL
# =========================
model_path = os.path.join(MODEL_DIR, "har_model.h5")
model.save(model_path)

print("Model saved at :", model_path)