const statusEl = document.getElementById("status")
const cam = document.getElementById("cam")
const overlay = document.getElementById("overlay")

// ===== RESULT =====
setInterval(() => {
    fetch("/result")
    .then(r => r.json())
    .then(data => {

        if (data.result === "PASS") {
            statusEl.innerText = "✅ PASS"
            statusEl.className = "status pass"

        } else if (data.result === "FAIL") {
            statusEl.innerText = "❌ FAIL"
            statusEl.className = "status fail"

        } else {
            statusEl.innerText = "⏳ WAITING"
            statusEl.className = "status waiting"
        }

    })
}, 800)


// ===== CAMERA =====
setInterval(() => {
    fetch("/camera_status")
    .then(r => r.json())
    .then(data => {

        if (data.status === "ON") {

            if (!cam.src.includes("video_feed")) {
                cam.src = "/video_feed"
            }

            cam.style.display = "block"
            overlay.style.opacity = "1"

        } else {
            cam.style.display = "none"
            cam.src = ""
            overlay.style.opacity = "0"
        }

    })
}, 500)
