import { GoogleGenerativeAI } from "@google/generative-ai";
import { Index } from "@upstash/vector";
import fs from "fs/promises";
import path from "path";
import * as XLSX from "xlsx";

// Lazy initializers for Google AI
function getEmbeddingModel() {
  const genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY || "");
  // Using gemini-embedding-001 with explicitly configured dimensions
  return genAI.getGenerativeModel({ 
    model: "gemini-embedding-001"
  });
}

// Helper to get Upstash Index lazily
function getIndex() {
  return new Index({
    url: process.env.UPSTASH_VECTOR_REST_URL,
    token: process.env.UPSTASH_VECTOR_REST_TOKEN,
  });
}

// Paths — Internal Knowledge Base
const DOCS_DIR  = path.join(process.cwd(), "ai-knowledge-core", "docs", "hardware");
const SPECS_DIR = path.join(process.cwd(), "ai-knowledge-core", "docs", "hardware", "components-specs");
const CSV_PATH = path.join(process.cwd(), "..", "HARDWARE", "HARDWARE_LIST_CLASSIFIED.csv");
const MD_HARDWARE_LIST_PATH = path.join(process.cwd(), "dev-process", "HARDWARE_LIST.md");

// Paths — External Project Assets (Global Ingestion)
const PROJECT_HARDWARE_DIR = path.join(process.cwd(), "..", "HARDWARE");
const PROJECT_DOCS_DIR     = path.join(process.cwd(), "..", "HARDWARE", "DOCS");
const PROJECT_3D_DIR       = path.join(process.cwd(), "..", "3D");
const PROJECT_CODE_DIR     = path.join(process.cwd(), "..", "SOFTWARE");
const PROJECT_MEDIA_DIR    = path.join(process.cwd(), "..", "MEDIA");
const PROJECT_DEV_PROCESS_DIR = path.join(process.cwd(), "dev-process");

interface VectorEntry {
  id: string;
  text: string;
  metadata?: any;
}

// 1. Embedding Generation
async function generateEmbedding(text: string): Promise<number[]> {
  // Add a 1.5s delay to stay within free-tier Rate Limits (15 RPM)
  await new Promise(resolve => setTimeout(resolve, 1500));
  const model = getEmbeddingModel();
  const result = await model.embedContent({
    content: { parts: [{ text }], role: "user" },
    outputDimensionality: 1536,
  });
  return result.embedding.values;
}

// 2. Ingestion of all sources (Docs, JSON Specs, CSV)
export async function ingestDocs() {
  console.log("🚀 Starting Upstash Vector Ingestion...");

  // A. Ingest Markdown Docs
  const mdFiles = (await fs.readdir(DOCS_DIR).catch(() => [])).filter(f => f.endsWith('.md'));
  for (const file of mdFiles) {
    const content = await fs.readFile(path.join(DOCS_DIR, file), "utf-8");
    const chunks = content.split('\n\n').filter(c => c.trim().length > 10);
    
    for (let i = 0; i < chunks.length; i++) {
      const chunkId = `doc-${file}-${i}`;
      try {
        const vector = await generateEmbedding(chunks[i]);
        await getIndex().upsert({
          id: chunkId,
          vector,
          metadata: { text: chunks[i], source: `docs/${file}` }
        });
        console.log(`✅ Ingested chunk ${i} of ${file}`);
      } catch(e) {
        console.error(`❌ Failed to ingest ${chunkId}`, e);
      }
    }
  }

  // B. Ingest JSON Specs
  const jsonFiles = (await fs.readdir(SPECS_DIR).catch(() => [])).filter(f => f.endsWith('.json'));
  for (const file of jsonFiles) {
    const content = await fs.readFile(path.join(SPECS_DIR, file), "utf-8");
    const chunkId = `spec-${file}`;
    try {
      const text = `Component Specification for ${file}:\n${content}`;
      const vector = await generateEmbedding(text);
      await getIndex().upsert({
        id: chunkId,
        vector,
        metadata: { text, source: `specs/${file}` }
      });
      console.log(`✅ Ingested spec: ${file}`);
    } catch(e) {
      console.error(`❌ Failed to ingest spec ${file}`, e);
    }
  }

  // C. Ingest CSV Hardware List & Sync Markdown
  try {
    console.log(`📊 Reading Master CSV: ${CSV_PATH}`);
    const workbook = XLSX.readFile(CSV_PATH);
    const sheetName = workbook.SheetNames[0];
    const worksheet = workbook.Sheets[sheetName];
    const jsonData: any[] = XLSX.utils.sheet_to_json(worksheet);

    if (jsonData.length > 0) {
      // 1. Sync to HARDWARE_LIST.md (Automation) — Do this first so docs update even if API fails
      let mdContent = `# HARDWARE COMPONENTS LIST (Sync from CSV)\n\n`;
      mdContent += `This document is automatically generated from \`HARDWARE_LIST_CLASSIFIED.csv\`.\n\n`;
      
      const systems = [...new Set(jsonData.map(item => item["System / Board"] || item["Placa (System)"] || "General"))];
      
      for (const system of systems) {
        mdContent += `## 📦 ${system.toUpperCase()}\n\n`;
        mdContent += `| Component | Part Name | Technical Specifications | Category |\n`;
        mdContent += `| :--- | :--- | :--- | :--- |\n`;
        
        const items = jsonData.filter(item => (item["System / Board"] || item["Placa (System)"]) === system);
        for (const item of items) {
          const comp = item["Component"] || "";
          const name = item["Part Name"] || "";
          const spec = item["Technical Specifications"] || "";
          const cat  = item["Category"] || "";
          mdContent += `| **${comp}** | ${name} | ${spec} | ${cat} |\n`;
        }
        mdContent += `\n---\n\n`;
      }

      await fs.writeFile(MD_HARDWARE_LIST_PATH, mdContent, "utf-8");
      console.log(`📝 Updated HARDWARE_LIST.md from CSV data.`);

      // 2. Ingest into Vector DB
      try {
        const csvText = JSON.stringify(jsonData, null, 2);
        const vector = await generateEmbedding(csvText);
        await getIndex().upsert({
          id: `csv-hardware-list`,
          vector,
          metadata: { text: `Master Hardware List from CSV:\n${csvText}`, source: "HARDWARE_LIST_CLASSIFIED.csv" }
        });
        console.log(`✅ Ingested Hardware CSV (${jsonData.length} components)`);
      } catch (embErr) {
        console.error("⚠️ Local docs updated, but Vector Ingestion failed (check API Key).");
      }
    }
  } catch(e) {
    console.error("❌ Failed to ingest CSV or Sync MD", e);
  }

  // D. Global Project Ingestion (External Folders - Recursive Scan)
  const externalDirs = [
    { path: PROJECT_HARDWARE_DIR, name: "HARDWARE" },
    { path: PROJECT_DOCS_DIR, name: "DOCS" },
    { path: PROJECT_3D_DIR, name: "3D" },
    { path: PROJECT_CODE_DIR, name: "CODE" },
    { path: PROJECT_MEDIA_DIR, name: "MEDIA" },
    { path: PROJECT_DEV_PROCESS_DIR, name: "DEV-PROCESS" }
  ];

  async function walkDir(currentPath: string, dirLabel: string) {
    const entries = await fs.readdir(currentPath, { withFileTypes: true }).catch(() => []);
    for (const entry of entries) {
      const fullPath = path.join(currentPath, entry.name);
      if (entry.isDirectory()) {
        await walkDir(fullPath, dirLabel);
      } else {
        const ext = path.extname(entry.name).toLowerCase();
        if (['.md', '.txt', '.json', '.ino', '.cpp', '.h'].includes(ext)) {
          try {
            const content = await fs.readFile(fullPath, "utf-8");
            const vector = await generateEmbedding(content);
            await getIndex().upsert({
              id: `ext-${dirLabel}-${entry.name}`,
              vector,
              metadata: { text: content, source: `WELDING_PEDAL/${dirLabel}/${path.relative(currentPath, fullPath)}` }
            });
            console.log(`✅ Ingested external file: ${dirLabel}/${entry.name}`);
          } catch(e) {
            console.error(`❌ Failed to ingest ${entry.name}`, e);
          }
        } else if (['.stl', '.step', '.pdf', '.xlsx', '.png', '.jpg', '.jpeg', '.mp4'].includes(ext)) {
          try {
            const text = `Artifact discovery: The project contains a ${ext.toUpperCase().substring(1)} file named "${entry.name}" in the ${dirLabel} folder.`;
            const vector = await generateEmbedding(text);
            await getIndex().upsert({
              id: `ext-discovery-${dirLabel}-${entry.name}`,
              vector,
              metadata: { text, source: `WELDING_PEDAL/${dirLabel}/${entry.name}`, discovery: true }
            });
            console.log(`🔍 Logged external discovery: ${dirLabel}/${entry.name}`);
          } catch(e) {}
        }
      }
    }
  }

  for (const dir of externalDirs) {
    await walkDir(dir.path, dir.name);
  }

  console.log("🏁 Ingestion Complete!");
}

// 3. Query Pipeline
export async function retrieveRelevantContext(query: string, topK: number = 3): Promise<string> {
  try {
    const queryEmbedding = await generateEmbedding(query);
    
    const results = await getIndex().query({
      vector: queryEmbedding,
      topK: topK,
      includeMetadata: true,
    });

    return results
      .map(res => res.metadata?.text || "")
      .filter(t => t !== "")
      .join("\n\n---\n\n");
  } catch (error) {
    console.error("Upstash Vector Query Error:", error);
    return "";
  }
}

// 4. Auto-Ingestion (Memory Learning)
export async function autoIngestValidInteraction(question: string, answer: string) {
  const entryText = `User Q: ${question}\nValidated Answer: ${answer}`;
  const chunkId = `mem-conv-${Date.now()}`;
  
  try {
    const vector = await generateEmbedding(entryText);
    await getIndex().upsert({
      id: chunkId,
      vector,
      metadata: { text: entryText, source: "live_community_chat" }
    });
    console.log(`[RAG Auto-Ingestion] Learned new interaction: ${chunkId}`);
  } catch (err) {
    console.error("Auto ingestion failed", err);
  }
}
