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
    console.log("✅ API Key detectada. Consultando todos los modelos...");
    const genAI = new GoogleGenerativeAI(API_KEY);

    const response = await fetch("https://generativelanguage.googleapis.com/v1beta/models?key=" + API_KEY);
    const data = await response.json();

    if (data.error) {
      console.log("❌ Error de la API de Google: " + data.error.message);
      return;
    }

    console.log("Todos los modelos devueltos por la API:");
    data.models.forEach(m => console.log(`- ${m.name} (Methods: ${m.supportedGenerationMethods.join(', ')})`));

    console.log("\n🔥 Probando generación de embeddings con gemini-embedding-001...");
    const model = genAI.getGenerativeModel({ model: "gemini-embedding-001" });
    const result = await model.embedContent("Hola mundo");
    console.log("✅ EMBEDDING EXITOSO: ", result.embedding.values.slice(0, 5), "... (dimensión: " + result.embedding.values.length + ")");

  } catch (error) {
    console.log("❌ ERROR DE CONEXIÓN: ", error.message);
  }
}

testGemini();
