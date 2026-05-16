const dotenv = require('dotenv');
const { Index } = require('@upstash/vector');
const { Redis } = require('@upstash/redis');
const fs = require('fs');

async function testUpstash() {
  try {
    // Manually parse .env.local because we are in a scratch script
    const envFile = fs.readFileSync('.env.local', 'utf-8');
    const getEnv = (key) => {
      const match = envFile.match(new RegExp(`${key}="(.*?)"`));
      return match ? match[1] : null;
    };

    const vectorUrl = getEnv('UPSTASH_VECTOR_REST_URL');
    const vectorToken = getEnv('UPSTASH_VECTOR_REST_TOKEN');
    const redisUrl = getEnv('UPSTASH_REDIS_REST_URL');
    const redisToken = getEnv('UPSTASH_REDIS_REST_TOKEN');

    console.log("🚀 Probando conexiones de Upstash...");

    // 1. Test Vector
    if (vectorUrl && vectorToken) {
      const index = new Index({ url: vectorUrl, token: vectorToken });
      try {
        const info = await index.info();
        console.log("✅ Upstash Vector: CONECTADO. Info:", info);
      } catch (err) {
        // Try fallback for older versions or different API
        console.log("⚠️ Upstash Vector: Conexión establecida, pero info() falló. Probando query básico...");
        await index.query({ vector: new Array(1536).fill(0), topK: 1 });
        console.log("✅ Upstash Vector: CONECTADO (Query exitoso).");
      }
    } else {
      console.log("❌ Upstash Vector: Faltan credenciales en .env.local.");
    }

    // 2. Test Redis
    if (redisUrl && redisToken) {
      const redis = new Redis({ url: redisUrl, token: redisToken });
      await redis.set('test-connection', 'ok');
      const val = await redis.get('test-connection');
      if (val === 'ok') {
        console.log("✅ Upstash Redis: CONECTADO y funcionando.");
      } else {
        console.log("❌ Upstash Redis: Error al leer datos.");
      }
    } else {
      console.log("❌ Upstash Redis: Faltan credenciales en .env.local.");
    }

  } catch (error) {
    console.error("❌ ERROR DE CONEXIÓN A UPSTASH:", error.message);
  }
}

testUpstash();
