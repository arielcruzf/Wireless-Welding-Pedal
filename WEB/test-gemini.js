const fs = require('fs');
const { GoogleGenerativeAI } = require('@google/generative-ai');

async function testGemini() {
  try {
    const envFile = fs.readFileSync('.env.local', 'utf-8');
    const keyMatch = envFile.match(/GEMINI_API_KEY="(.*?)"/);

    if (!keyMatch || !keyMatch[1]) {
      console.log("❌ ERROR: No pude encontrar GEMINI_API_KEY en .env.local.");
      return;
    }

    const API_KEY = keyMatch[1];

    if (API_KEY === "AIzaSy_PEGA_AQUI_TU_CLAVE") {
      console.log("❌ ERROR: Tienes que reemplazar el texto por defecto con tu clave real.");
      return;
    }

    console.log("✅ API Key detectada. Consultando modelos disponibles...");
    const genAI = new GoogleGenerativeAI(API_KEY);

    const response = await fetch("https://generativelanguage.googleapis.com/v1beta/models?key=" + API_KEY);
    const data = await response.json();

    if (data.error) {
      console.log("❌ Error de la API de Google: " + data.error.message);
      return;
    }

    const textModels = data.models.filter(m => m.supportedGenerationMethods.includes("generateContent"));
    console.log("Modelos encontrados: " + textModels.map(m => m.name).join(", "));

    if (textModels.length === 0) {
      console.log("❌ No se encontraron modelos de texto.");
      return;
    }

    // Use gemini-2.0-flash as it's widely available
    const modelName = textModels.find(m => m.name.includes("gemini-1.5-flash"))?.name.replace("models/", "") 
      || textModels[0].name.replace("models/", "");

    console.log("🔥 Modelo seleccionado: " + modelName);

    const model = genAI.getGenerativeModel({ model: modelName });
    const result = await model.generateContent("Responde únicamente con: 'La API de Gemini funciona perfectamente 🚀'");
    console.log("✅ RESPUESTA DE GEMINI: " + result.response.text());

  } catch (error) {
    console.log("❌ ERROR DE CONEXIÓN: ", error.message);
  }
}

testGemini();
