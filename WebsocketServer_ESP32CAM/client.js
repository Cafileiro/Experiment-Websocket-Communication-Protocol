// client.js -- Node.js WebSocket client that connects to ESP32-CAM WS and saves last.jpg
const WebSocket = require('ws');
const fs = require('fs');
const path = require('path');
const express = require('express');

const ESP_IP = '192.168.1.41';
const ESP_WS = `ws://${ESP_IP}/ws`;

// Setup simple Express server to serve last.jpg and a small viewer page
const app = express();
const PORT = 3000;
const outfile = path.join(__dirname, 'last.jpg');

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

ws.on('message', (data) => {
  // data is a Buffer containing the JPEG bytes (binary)
  if (Buffer.isBuffer(data)) {
    fs.writeFile(outfile, data, (err) => {
      if (err) console.error('Error saving image', err);
      else console.log('Saved', outfile, 'size', data.length);
    });
  } else {
    console.log('Received non-binary message:', data.toString());
  }
});

ws.on('close', () => console.log('WS closed'));
ws.on('error', (err) => console.error('WS error', err));
