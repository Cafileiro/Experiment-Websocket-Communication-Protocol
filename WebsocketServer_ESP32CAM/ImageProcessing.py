import os
from fastapi import FastAPI, File, UploadFile, HTTPException
from fastapi.responses import Response
import io
import uvicorn
import cv2 
import numpy as np
from concurrent.futures import ThreadPoolExecutor

app = FastAPI(title="Image Processing Microservice", version="1.0")

WORKERS = int(os.getenv("WORKERS", 4))
executor = ThreadPoolExecutor(max_workers=WORKERS)

def process_image_bytes(jpeg_bytes: bytes):
    #TODO: Implement image processing logic here
    pass

@app.post("/process", response_class=Response, responses={200: {"content": {"image/jpeg": {}}}})
async def process(image: UploadFile = File(...)):
    # type validation
    if image.content_type not in ("image/jpeg", "image/jpg", "image/png"):
        raise HTTPException(status_code=400, detail="Unsupported file type (use JPEG/PNG)")

    # read image data
    data = await image.read()
    if not data:
        raise HTTPException(status_code=400, detail="Empty file")

    # Run processing in the ThreadPool to avoid blocking the event loop
    loop = __import__("asyncio").get_event_loop()
    try:
        processed = await loop.run_in_executor(executor, process_image_bytes, data)
    except ValueError as e:
        raise HTTPException(status_code=500, detail=str(e))
    except Exception as e:
        raise HTTPException(status_code=500, detail="Internal error: " + str(e))

    return Response() #TODO: return an order depend on processed image bytes in processed

if __name__ == "__main__":
    # For development: uvicorn server:app --host 0.0.0.0 --port 5000
    uvicorn.run("server:app", host="0.0.0.0", port=5000, workers=1)