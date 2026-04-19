import os
import cv2
import time

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
model_path = os.path.join(BASE_DIR, "model/lbph_model.yml")

recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.read(model_path)

cam = cv2.VideoCapture(0)

start_time = time.time()
frame = None

# ✅ 這 5 秒是「動態畫面」
while time.time() - start_time < 5:
    ret, frame = cam.read()
    if not ret:
        cam.release()
        exit()

    cv2.imshow("camera", frame)
    cv2.waitKey(1)  # 很重要，不然畫面不會更新

# ✅ 5 秒後才做辨識（用最後一張）
gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
gray = cv2.resize(gray, (200, 200))

label, confidence = recognizer.predict(gray)

if confidence < 80:
    result = "PASS"
else:
    result = "FAIL"

print(result)
print(confidence)

# 👉 顯示結果再停 2 秒（可調整）
cv2.waitKey(1000)

cam.release()
cv2.destroyAllWindows()
