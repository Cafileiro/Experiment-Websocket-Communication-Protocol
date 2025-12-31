// client.js -- Node.js WebSocket client that connects to ESP32-CAM WS and saves last.jpg
const WebSocket = require('ws');
const fs = require('fs');
const path = require('path');
const express = require('express');
const FormData = require('form-data');
const fetch = require('node-fetch');

// Configuration
const ESP_IP = process.env.ESP_IP ;
const ESP_WS = `ws://${ESP_IP}/ws`;
const PY_SERVER = process.env.PY_SERVER;
// Setup simple Express server to serve last.jpg and a small viewer page
const app = express();
const PORT = process.env.PORT;
const outfile = path.join(__dirname, 'last.jpg');

let busy = false;

app.get('/', (req, res) => {
  res.send(`
    <html>
      <body>
        <h3>Última imagen desde ESP32-CAM</h3>
        <img id="cam" src="/last.jpg?ts=${Date.now()}" style="max-width: 100%;">
        <script>
          // auto-refresh the image each second
          setInterval(() => {
            document.getElementById('cam').src = '/last.jpg?ts=' + Date.now();
          }, 1000);
        </script>
      </body>
    </html>
  `);
});

app.get('/last.jpg', (req, res) => {
  if (fs.existsSync(outfile)) {
    res.sendFile(outfile);
  } else {
    res.status(404).send('No image yet');
  }
});

app.listen(PORT, () => {
  console.log(`Viewer server running http://localhost:${PORT}`);
});

// Connect as WebSocket client to ESP
const ws = new WebSocket(ESP_WS);

ws.on('open', () => {
  console.log('Connected to ESP WebSocket', ESP_WS);
});

ws.on('message', async (data) => {
  if (!Buffer.isBuffer(data)) return;
  if (busy) return console.log('Servidor ocupado, descartando frame');

  busy = true;
  try {
    const form = new FormData();
    form.append('image', data, { filename: 'last.jpg', contentType: 'image/jpeg' });

    const res = await fetch(PY_SERVER, { method: 'POST', body: form, timeout: 10000 });
    if (!res.ok) throw new Error('HTTP ' + res.status);
    const buf = await res.buffer();
    fs.writeFileSync(outfile, buf);
    console.log('Imagen procesada guardada, bytes=', buf.length);
  } catch (err) {
    console.error('Error al procesar:', err.message || err);
  } finally {
    busy = false;
  }
});

ws.on('close', () => console.log('WS closed'));
ws.on('error', (err) => console.error('WS error', err));
