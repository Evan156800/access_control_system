import os
import cv2

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

model_path = os.path.join(BASE_DIR, "model/lbph_model.yml")

recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.read(model_path)

cam = cv2.VideoCapture(0)

ret, frame = cam.read()

if not ret:
    print("FAIL")
    cam.release()
    exit()

cv2.imshow("camera", frame)
cv2.waitKey(1)

gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
gray = cv2.resize(gray, (200, 200))

label, confidence = recognizer.predict(gray)

cam.release()

# 越小越像
if confidence < 80:
    print("PASS")
    print(confidence)
else:
    print("FAIL")
    print(confidence)
