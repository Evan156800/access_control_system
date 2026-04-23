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

# ⭐ 加入人臉偵測
face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
)

# ================= OPEN CAMERA =================
with open(CAMERA_STATUS_PATH, "w") as f:
    f.write("ON")

# 等 Flask 更新 frame（避免抓到空畫面）
time.sleep(2)

# 強制更新一張 frame
try:
    requests.get("http://localhost:5000/get_frame")
except:
    pass

# ================= READ FRAME =================
frame = cv2.imread(FRAME_PATH)

# ================= RESULT =================
result = "FAIL"

if frame is None:
    print("NO FRAME")

else:
    cv2.imwrite(os.path.join(WEB_DIR, "debug.jpg"), frame)

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # ⭐ 找臉
    faces = face_cascade.detectMultiScale(gray, 1.3, 5)

    if len(faces) == 0:
        print("NO FACE DETECTED")

    else:
        (x, y, w, h) = faces[0]
        
        pad = 20
        x1 = max(x - pad, 0)
        y1 = max(y- pad, 0)
        x2 = min(x + w + pad, gray.shape[1])
        y2 = min(y + h + pad, gray.shape[0])

        face_img = gray[y1:y2, x1:x2]
        face_img = cv2.resize(face_img, (200, 200))

        label, confidence = recognizer.predict(face_img)

        print("confidence:", confidence)

        # ⭐ 門檻建議調高（你原本太嚴）
        if confidence < 120:
            result = "PASS"
        else:
            result = "FAIL"

# 存結果
with open(RESULT_PATH, "w") as f:
    f.write(result)

# ================= CLOSE CAMERA =================
with open(CAMERA_STATUS_PATH, "w") as f:
    f.write("OFF")
