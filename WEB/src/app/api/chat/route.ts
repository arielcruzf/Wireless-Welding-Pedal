import { GoogleGenerativeAI } from "@google/generative-ai";
import { NextResponse } from "next/server";
import { Redis } from "@upstash/redis";
import { retrieveRelevantContext } from "../../../../ai-knowledge-core/rag-pipeline";

// Initialize Gemini for Text Generation
const genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY || "");
const model = genAI.getGenerativeModel({ model: "gemini-flash-latest" });

// Initialize Upstash Redis for History
const redis = new Redis({
  url: process.env.UPSTASH_REDIS_REST_URL!,
  token: process.env.UPSTASH_REDIS_REST_TOKEN!,
});

// Contextual prompt to keep the AI on track
const SYSTEM_PROMPT = `
You are an expert assistant specialized in the assembly, hardware components, and configuration of the "Wireless Welder Pedal" project.
Your goal is to help users resolve doubts based on the provided RAG knowledge base.

### DIAGRAMMING & DEPLOYMENT SKILLS:
1. **Tabulation Skill**: Whenever listing components, pinouts, or voltage comparisons, use Markdown tables for clear reading.
2. **Logic Skill**: To explain electrical connections or data flows, use 'mermaid' code blocks (flowchart, sequenceDiagram, etc.).
   - IMPORTANT: Keep diagrams simple and use clear labels.
3. **Math Skill**: For power calculations, Ohm's law, or voltage dividers, use LaTeX notation (e.g., $V = I \cdot R$ or $$P = V^2 / R$$).
4. **Alerts**: Use bold text and emojis for physical safety warnings (welding, high voltages).
5. **Coding Skill**: Whenever the user asks about the programming logic of the modules (Transmitter/Receiver), generate well-commented C++ / Arduino code blocks optimized for LoRa32u4 hardware.
6. **C++ Embebido Skill**: Expert knowledge in low-level AVR register control (e.g., ADCSRA, USBCON, PLLCSR), power-down sleep modes, non-blocking interrupt-driven logic (ISRs), and efficient memory usage without dynamic allocations.

### STYLE & LANGUAGE:
- Tone: Professional, expert, and friendly (DIY community).
- Language: Your primary language is English. However, you MUST respond in the same language as the user's question (e.g., if asked in Spanish, respond in Spanish; if asked in English, respond in English).
- Aesthetics: Clean and professional. Use markdown tables where possible, and use code blocks with specified language (\`\`\`cpp) for firmware.

If the information is not in the context, indicate it politely but use your general knowledge about RF electronics and microcontrollers (LoRa32u4, SX1278, I2C, Mosfets) to guide the user.
`;

export async function POST(req: Request) {
  try {
    const body = await req.json();
    const { messages, sessionId = "default-session" } = body;
    
    if (!messages || messages.length === 0) {
      return NextResponse.json({ error: "No messages provided" }, { status: 400 });
    }

    const latestMessage = messages[messages.length - 1].content;

    // 1. CHAT HISTORY (Memory): Fetch previous messages from Redis
    const historyKey = `chat-history:${sessionId}`;
    const history: any[] = (await redis.lrange(historyKey, -10, -1)) || [];
    const formattedHistory = history
      .map((m: any) => `${m.role === "user" ? "User" : "Assistant"}: ${m.content}`)
      .join("\n");

    // 2. DYNAMIC RAG: Fetch relevant memory/docs from Upstash Vector
    const context = await retrieveRelevantContext(latestMessage, 3);

    const fullPrompt = `
${SYSTEM_PROMPT}

=== CONTEXT START (Internal Knowledge Base) ===
${context}
=== CONTEXT END ===

=== RECENT CONVERSATION HISTORY ===
${formattedHistory}

User Message: ${latestMessage}
Assistant:`;

    // 3. Generate Response with Gemini 2.0 Flash
    const result = await model.generateContent(fullPrompt);
    const textResponse = result.response.text();

    // 4. PERSIST TO HISTORY (Background)
    const updatedHistory = [
      ...history,
      { role: "user", content: latestMessage },
      { role: "assistant", content: textResponse }
    ].slice(-10); // Keep last 10 messages (5 pairs)
    
    redis.del(historyKey).then(() => {
      redis.rpush(historyKey, ...updatedHistory);
    }).catch(err => console.error("Redis storage error:", err));

    // 5. AUTO-INGESTION: eliminado — usar 'update ia' en ingest-all.ts para sincronizar manualmente.

    // 6. Return via JSON Next.js Route
    return NextResponse.json({
      role: "assistant",
      content: textResponse,
    });

  } catch (error: any) {
    console.error("Chat API Error:", error);
    return NextResponse.json({ error: "Internal Server Error" }, { status: 500 });
  }
}
