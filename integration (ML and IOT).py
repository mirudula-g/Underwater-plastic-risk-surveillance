# ================================
# IMPORTS
# ================================
import numpy as np
import pandas as pd
import serial
import random

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.ensemble import GradientBoostingClassifier

# ================================
# LOAD DATASET
# ================================
data = pd.read_csv("microplastic_synthetic.csv")

# ================================
# ADD NOISE
# ================================
data_noisy = data.copy()

noise_percentage = 0.1
num_noisy = int(len(data_noisy) * noise_percentage)

indices = random.sample(range(len(data_noisy)), num_noisy)

for idx in indices:
    current_label = data_noisy.loc[idx, "Microplastic_Risk"]
    possible_labels = ["Low", "Medium", "High"]
    possible_labels.remove(current_label)
    data_noisy.loc[idx, "Microplastic_Risk"] = random.choice(possible_labels)

data = data_noisy

# ================================
# FEATURES
# ================================
X = data[["Turbidity_NTU", "pH"]].copy()
y = data["Microplastic_Risk"]

# Weight turbidity
X["Turbidity_NTU"] = X["Turbidity_NTU"] * 1.3

# ================================
# TRAIN TEST SPLIT
# ================================
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

# ================================
# SCALING
# ================================
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)

# ================================
# MODEL TRAINING
# ================================
model = GradientBoostingClassifier(
    n_estimators=150,
    learning_rate=0.05,
    max_depth=3,
    random_state=42
)

model.fit(X_train, y_train)

print("✅ Model Ready. Waiting for ESP32 data...")

# ================================
# SERIAL CONNECTION
# ================================
# ⚠️ CHANGE COM PORT
ser = serial.Serial('COM3', 115200)

# ================================
# REAL-TIME PREDICTION LOOP
# ================================
while True:
    line = ser.readline().decode().strip()

    if line:
        try:
            turbidity, ph = map(float, line.split(","))

            # Apply same preprocessing
            turbidity = turbidity * 1.3

            sample = np.array([[turbidity, ph]])
            sample_scaled = scaler.transform(sample)

            prediction = model.predict(sample)

            print("📥 Input:", turbidity, ph)
            print("🤖 Prediction:", prediction[0])
            print("------------------------")

        except:
            pass