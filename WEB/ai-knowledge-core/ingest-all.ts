/**
 * ingest-all.ts — RAG Pipeline Entry Point
 * -------------------------------------------------------
 * This script synchronizes the project's local knowledge with
 * the cloud vector database (Upstash Vector).
 *
 * What it does:
 *  1. Reads all documents from the `ai-knowledge-core/` directory:
 *     - Markdown files (.md):  WIRING_DIAGRAMS.md, hardware docs, etc.
 *     - JSON files (.json):    component specs (lora32u4, vl53l4cd…)
 *     - CSV files  (.csv):     HARDWARE_LIST_CLASSIFIED.csv
 *  2. Splits each document into chunks with context.
 *  3. Generates vector embeddings for each chunk (via Gemini Embeddings).
 *  4. Uploads vectors to Upstash Vector (cloud RAG database).
 *
 * When to run it?
 *  Every time a knowledge document is updated, for example:
 *  - WIRING_DIAGRAMS.md is modified with new hardware connections.
 *  - A new component is added to the CSV or JSON specs.
 *  This ensures the project's AI Chat always has the latest information.
 *
 * Usage Modes:
 *  A) Direct Execution (immediate ingestion):
 *     npx tsx ai-knowledge-core/ingest-all.ts
 *
 *  B) Interactive Mode (terminal listener):
 *     npx tsx ai-knowledge-core/ingest-all.ts --watch
 *     → Type "update ia" and press Enter to trigger synchronization.
 *     → Type "exit" or Ctrl+C to quit.
 */

import * as dotenv from "dotenv";
import path from "path";
import * as readline from "readline";

// Load environment variables from .env.local BEFORE importing logic
dotenv.config({ path: path.resolve(process.cwd(), ".env.local") });

import { ingestDocs } from "./rag-pipeline";

// ─── Core ingestion runner ───────────────────────────────────────────────────

async function runIngestion() {
  console.log("\n🛠  Starting synchronization with Upstash Vector...");
  try {
    await ingestDocs();
    console.log("✅ Synchronization complete. The AI Chat now has the updated knowledge.\n");
  } catch (error) {
    console.error("❌ Critical error during synchronization:", error);
  }
}

// ─── Interactive watch mode ───────────────────────────────────────────────────

function startWatchMode() {
  const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    terminal: false,
  });

  console.log("👀 Modo interactivo activo.");
  console.log('   Escribe "update ia" + Enter para sincronizar con Upstash.');
  console.log('   Escribe "exit"      + Enter para salir.\n');

  rl.on("line", async (line) => {
    const input = line.trim().toLowerCase();

    if (input === "update ia") {
      await runIngestion();
    } else if (input === "exit") {
      console.log("👋 Exiting interactive mode.");
      rl.close();
      process.exit(0);
    } else if (input !== "") {
      console.log(`ℹ️  Unrecognized command: "${line.trim()}". Use "update ia" to synchronize.`);
    }
  });

  rl.on("close", () => {
    process.exit(0);
  });
}

// ─── Entry point ─────────────────────────────────────────────────────────────

const args = process.argv.slice(2);

if (args.includes("--watch")) {
  // Mode B: Interactive — waits for "update ia"
  startWatchMode();
} else {
  // Mode A: Direct — runs immediately and exits
  runIngestion().then(() => process.exit(0));
}
