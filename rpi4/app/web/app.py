from flask import Flask, render_template, Response, jsonify
import cv2
import time
import os

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
RESULT_PATH = os.path.join(BASE_DIR, "result.txt")
CAMERA_STATUS_PATH = os.path.join(BASE_DIR, "camera_status.txt")
FRAME_PATH = os.path.join(BASE_DIR, "frame.jpg")

camera = cv2.VideoCapture(0)
latest_frame = None

# ================= INIT (解決 FAIL bug) =================
#if not os.path.exists(RESULT_PATH):
with open(RESULT_PATH, "w") as f:
    f.write("NONE")

#if not os.path.exists(CAMERA_STATUS_PATH):
with open(CAMERA_STATUS_PATH, "w") as f:
    f.write("OFF")

# ================= CAMERA STREAM =================
def gen_frames():
    global latest_frame

    while True:
        # 讀 camera 開關
        try:
            with open(CAMERA_STATUS_PATH) as f:
                status = f.read().strip()
        except:
            status = "OFF"

        if status != "ON":
            time.sleep(0.2)
            continue

        success, frame = camera.read()
        if not success:
            continue

        latest_frame = frame.copy()

        # 給 face_auth 用
        cv2.imwrite(FRAME_PATH, frame)

        _, buffer = cv2.imencode('.jpg', frame)
        frame_bytes = buffer.tobytes()

        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')


# ================= PAGES =================
@app.route("/")
def index():
    return render_template("index.html")


@app.route("/video_feed")
def video_feed():
    return Response(gen_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')


# ================= API =================
@app.route("/camera_status")
def camera_status():
    try:
        with open(CAMERA_STATUS_PATH) as f:
            status = f.read().strip()
    except:
        status = "OFF"
    return jsonify({"status": status})


@app.route("/result")
def result():
    try:
        with open(RESULT_PATH) as f:
            r = f.read().strip()
    except:
        r = "NONE"
    return jsonify({"result": r})


# ⭐ face_auth 取 frame 用
@app.route("/get_frame")
def get_frame():
    global latest_frame

    if latest_frame is None:
        return "NO_FRAME", 500

    cv2.imwrite(FRAME_PATH, latest_frame)
    return "OK"


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, threaded=True)
