import os
import cv2
import time
import requests

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

MODEL_PATH = os.path.join(BASE_DIR, "model/lbph_model.yml")

WEB_DIR = os.path.join(BASE_DIR, "../../app/web")
RESULT_PATH = os.path.join(WEB_DIR, "result.txt")
CAMERA_STATUS_PATH = os.path.join(WEB_DIR, "camera_status.txt")
FRAME_PATH = os.path.join(WEB_DIR, "frame.jpg")

recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.read(MODEL_PATH)

# ================= OPEN CAMERA =================
with open(CAMERA_STATUS_PATH, "w") as f:
    f.write("ON")

# 等 Flask 更新 frame
time.sleep(3)

# 強制 request frame
try:
    requests.get("http://localhost:5000/get_frame")
except:
    pass

# ================= READ FRAME =================
frame = cv2.imread(FRAME_PATH)

# ================= RESULT =================
if frame is None:
    print("NO FRAME")
    with open(RESULT_PATH, "w") as f:
        f.write("FAIL")

else:
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.resize(gray, (200, 200))

    label, confidence = recognizer.predict(gray)

    print("confidence:", confidence)

    with open(RESULT_PATH, "w") as f:
        if confidence < 80:
            f.write("PASS")
        else:
            f.write("FAIL")

# ================= CLOSE CAMERA =================
with open(CAMERA_STATUS_PATH, "w") as f:
    f.write("OFF")
