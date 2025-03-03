const express = require('express');
const app = express();
const port = 3000;

// Rota principal
app.get('/', (req, res) => {
    res.send('WebServer do Nosso Veículo!');
});

// Inicia o servidor
app.listen(port, () => {
    console.log(`Servidor rodando em http://localhost:${port}`);
});
