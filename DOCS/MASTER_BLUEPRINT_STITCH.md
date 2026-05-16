# MASTER BLUEPRINT: WIRELESS WELDER PEDAL (STITCH OPTIMIZED)

This document is the master manual for the technical and aesthetic reconstruction of the project. It is designed to be interpreted by AI agents (such as Stitch or Antigravity) to replicate the application from scratch with total fidelity.

---

## 1. Visual Identity & Design System (Tailwind v4)
The design follows an **"Industrial Premium / Light Card-Based"** aesthetic.

### Color Palette (HSL Tokens)
*   **Brand Duo (Gradients):** 
    *   `blue-600` (Left): Used for branding and Transmitter context.
    *   `cyan-500` (Right): Used for Receiver context and health accents.
*   **Industrial Grays:** 
    *   `bg-neutral-50`: Intermediate sections with elevation.
    *   `bg-neutral-950`: Footer and visual authority.
*   **Surface (Cards):** Pure `bg-white` with `shadow-2xl` and `rounded-[40px]`.

### Typography & Layout
*   **Display (Titles):** `Outfit` (Black 900). Tracking-tighter.
*   **Shadows:** Deep `shadow-2xl` for a "levitation" effect over colored backgrounds.

---

## 2. Section Structure (Next.js Components)

### A. HeroSection (Header with Video)
*   **Visual:** Video loop with a clean bottom cut. Shadow projected over the "Purchase" section.
*   **Content:** "Wireless" (Black) + "Welding Pedal" (Blue). Integrated "View Hardware" button in black.

### B. PurchaseSection (Get the Project Files)
*   **Layout:** `bg-neutral-50` background. Shadow projected downwards over the Hardware section.
*   **Logic:** Checkout integration for 3D files and Firmware.

### C. HardwareSpecs (Hardware & Assembly)
*   **Layout:** Background set in **Dual-Tone Gradient (`bg-gradient-to-r from-blue-600 to-cyan-500`)**.
*   **Component Cards:** "Project Assistant" style (White background, `shadow-2xl`, borderless).
*   **Logic (Link-Health & Affiliate):**
    *   **Index Auto-Correction:** Function that verifies link status (404/Out of Stock) and automatically searches for alternatives based on MPN.
    *   **Affiliate Generation:** Injection of `aff_trace_key` (`DEFAULT_TRACKING_ID`) via `lib/affiliate.ts`.
    *   **Smart Popups:** Pre-purchase warnings for components with critical variations (e.g., MOSFET 30V vs 100V).

### D. CommunitySupport (AI Assistant)
*   **Layout:** `bg-neutral-50` background. Upward shadow over the Hardware section.
*   **Chat Card:** 100% white integration with `shadow-2xl`.

### E. Footer
*   **Color:** `bg-neutral-950`. Final closure of the visual flow.

### F. BOM Section (Dynamic Bill of Materials)
*   **Location:** Prior to the Assembly Instructions section.
*   **Interactivity:** "Verify Link Health" button connecting to the API.
*   **Design:** Cards with dynamic states (`In Stock`, `Checking`, `Error/Searching`).
*   **Affiliate Logic:** Automatic Tracking ID injection via `lib/affiliate.ts`.

---

## 3. AI Logic Architecture (Brain)
The project utilizes a local **Dynamic RAG (Retrieval-Augmented Generation)** system.

### Critical Directories (ai-knowledge-core/)
1.  `docs/`: Reference technical documentation (.md).
2.  `components-specs/`: Technical profiles in **JSON** for each chip (Voltages, protocols).
3.  `memory/`: `vector-index.json` file storing calculated knowledge vectors.
4.  `prompts/`: Definition of the `system_prompt.md` governing the welding expert personality.

### Processing Logic (`rag-pipeline.ts`)
*   **Embeddings:** Uses `text-embedding-004`.
*   **Auto-Ingestion:** Every validated response is vectorized and injected into `memory` in real-time.
*   **Hardware Validation:** AI's ability to detect conflicts (e.g., voltage drop in the DAC).

---

## 4. Technology Stack
*   **Framework:** Next.js 15 (App Router).
*   **Styles:** Tailwind CSS v4.
*   **Icons:** Lucide React.
*   **AI:** Google Generative AI SDK (`@google/generative-ai`).
*   **Environment Variables:** `.env.local` requiring `GEMINI_API_KEY`.

---

## 5. Reconstruction Guide for Stitch
1.  Run `create-next-app` with Tailwind and TS.
2.  Inject `globals.css` with the HSL tokens defined above.
3.  Copy the entire `ai-knowledge-core` to restore the brain.
4.  Generate components based on the visual descriptions in point 2.
5.  Link the `CommunitySupport` component with the Gemini API through the RAG pipeline.
