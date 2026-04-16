import cv2
import os
import numpy as np

recognizer = cv2.face.LBPHFaceRecognizer_create()

faces = []
labels = []

label_map = {
    "evan": 1
}

for name, label in label_map.items():
    folder = "dataset"
    
    for file in os.listdir(folder):
        if file.startswith(name):
            img_path = os.path.join(folder, file)
            img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)

            if img is None:
                continue

            img = cv2.resize(img, (200, 200))

            faces.append(img)
            labels.append(label)

recognizer.train(faces, np.array(labels))
recognizer.save("model/lbph_model.yml")

print("TRAIN DONE")
