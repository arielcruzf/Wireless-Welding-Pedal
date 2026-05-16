"use client";

import { useState, useRef, useEffect } from "react";
import { MessageSquare, Send, Bot, User, Sparkles, BrainCircuit } from "lucide-react";
import ReactMarkdown from "react-markdown";
import remarkGfm from "remark-gfm";
import remarkMath from "remark-math";
import rehypeKatex from "rehype-katex";
import mermaid from "mermaid";
import "katex/dist/katex.min.css";

// Mermaid Configuration for White/Clean Aesthetic
mermaid.initialize({
  startOnLoad: true,
  theme: 'neutral',
  securityLevel: 'loose',
  fontFamily: 'inherit'
});

const Mermaid = ({ chart }: { chart: string }) => {
  const ref = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (ref.current && chart) {
      ref.current.removeAttribute("data-processed");
      mermaid.contentLoaded();
    }
  }, [chart]);

  return (
    <div className="mermaid my-4 flex justify-center bg-white p-4 rounded-2xl border border-neutral-100 overflow-x-auto" ref={ref}>
      {chart}
    </div>
  );
};

type Message = {
  role: "user" | "ai";
  content: string;
};

export default function CommunitySupport() {
  const [messages, setMessages] = useState<Message[]>([
    { role: "ai", content: "¡Hola! Soy el Asistente Técnico del proyecto. He sido entrenado con toda la documentación del Wireless Welder Pedal. ¿En qué puedo ayudarte hoy?" }
  ]);
  const [input, setInput] = useState("");
  const [isTyping, setIsTyping] = useState(false);
  const scrollRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (scrollRef.current) {
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
    }
  }, [messages, isTyping]);

  const handleSend = async () => {
    if (!input.trim()) return;

    const userMsg = input.trim();
    // Use 'ai' role for the internal frontend state, but our backend expects 'user'/'assistant' roles
    const newMessages: Message[] = [...messages, { role: "user", content: userMsg }];
    setMessages(newMessages);
    setInput("");
    setIsTyping(true);

    try {
      // Map roles to match backend expectations (user/assistant)
      const backendMessages = newMessages.map(m => ({
        role: m.role === "ai" ? "assistant" : "user",
        content: m.content
      }));

      const response = await fetch("/api/chat", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ 
          messages: backendMessages,
          sessionId: "wireless-pedal-visitor" // Fixed session for simplicity, can be dynamic later
        }),
      });

      const data = await response.json();

      if (data.error) {
        throw new Error(data.error);
      }

      setMessages(prev => [...prev, { role: "ai", content: data.content }]);
    } catch (error) {
      console.error("Chat Error:", error);
      setMessages(prev => [...prev, { 
        role: "ai", 
        content: "Lo siento, tuve un problema al consultar mi base de datos técnica. ¿Podrías intentarlo de nuevo?" 
      }]);
    } finally {
      setIsTyping(false);
    }
  };

  return (
    <section id="support" className="py-24 px-4 bg-neutral-50 relative overflow-hidden shadow-[0_-15px_30px_-15px_rgba(0,0,0,0.05)] z-10">
      {/* Decorative Background Elements */}
      <div className="absolute top-0 left-0 w-full h-full opacity-5 pointer-events-none">
        <div className="absolute top-20 left-10 w-64 h-64 bg-blue-500 rounded-full blur-3xl animate-pulse" />
        <div className="absolute bottom-20 right-10 w-96 h-96 bg-cyan-500 rounded-full blur-3xl" />
      </div>

      <div className="max-w-6xl mx-auto relative z-10">
        <div className="flex flex-col lg:flex-row gap-16 items-start">
          
          {/* Info Side */}
          <div className="flex-1 space-y-8">
            <div className="inline-flex items-center gap-2 px-4 py-2 bg-blue-100 text-blue-600 rounded-full text-sm font-bold tracking-tight uppercase">
              <BrainCircuit className="w-4 h-4" />
              RAG Intelligence Active
            </div>
            <h2 className="text-4xl md:text-5xl font-black text-neutral-900 leading-tight">
              Community & <span className="text-blue-600">AI Support</span>
            </h2>
            <p className="text-xl text-neutral-500 leading-relaxed max-w-lg">
              Any questions about the assembly? Our AI has read all the technical documentation to help you clear up doubts in real-time.
            </p>

            <div className="grid grid-cols-1 sm:grid-cols-2 gap-6 pt-4">
              <div className="p-6 bg-white rounded-3xl border border-neutral-200 shadow-sm">
                <h4 className="font-bold text-neutral-900 mb-2">Technical Learning</h4>
                <p className="text-sm text-neutral-500">The IA learns from the MASTER_BLUEPRINT and RAG documentation folders.</p>
              </div>
              <div className="p-6 bg-white rounded-3xl border border-neutral-200 shadow-sm">
                <h4 className="font-bold text-neutral-900 mb-2">Instant Answers</h4>
                <p className="text-sm text-neutral-500">Get hardware references and connection diagrams in seconds.</p>
              </div>
            </div>
          </div>

          {/* Chat Widget Side */}
          <div className="w-full lg:w-[500px] bg-white rounded-[40px] shadow-2xl border border-neutral-200 overflow-hidden flex flex-col h-[600px]">
            {/* Header */}
            <div className="bg-neutral-800 p-6 text-white flex items-center justify-between border-b border-neutral-700">
              <div className="flex items-center gap-3">
                <div className="w-10 h-10 bg-blue-600 rounded-2xl flex items-center justify-center">
                  <Bot className="w-6 h-6 text-white" />
                </div>
                <div>
                  <h3 className="font-bold text-sm">Project Assistant</h3>
                  <div className="flex items-center gap-1.5">
                    <div className="w-2 h-2 bg-emerald-500 rounded-full animate-pulse" />
                    <span className="text-[10px] text-neutral-300 uppercase tracking-widest font-bold">Online</span>
                  </div>
                </div>
              </div>
              <Sparkles className="w-5 h-5 text-blue-400" />
            </div>

            {/* Messages Area */}
            <div 
              ref={scrollRef}
              className="flex-1 overflow-y-auto p-6 space-y-6 scroll-smooth bg-neutral-50/50"
            >
              {messages.map((msg, i) => (
                <div key={i} className={`flex items-start gap-3 ${msg.role === "user" ? "flex-row-reverse" : ""}`}>
                  <div className={`w-8 h-8 rounded-xl flex items-center justify-center shrink-0 ${
                    msg.role === "ai" ? "bg-blue-100 text-blue-600" : "bg-neutral-900 text-white"
                  }`}>
                    {msg.role === "ai" ? <Bot className="w-5 h-5" /> : <User className="w-5 h-5" />}
                  </div>
                  <div className={`max-w-[90%] p-4 rounded-3xl text-sm leading-relaxed ${
                    msg.role === "ai" ? "bg-white border border-neutral-100 shadow-sm text-neutral-800" : "bg-blue-600 text-white shadow-xl shadow-blue-500/10"
                  }`}>
                    <ReactMarkdown
                      remarkPlugins={[remarkGfm, remarkMath]}
                      rehypePlugins={[rehypeKatex]}
                      components={{
                        code({ node, inline, className, children, ...props }: any) {
                          const match = /language-(\w+)/.exec(className || "");
                          if (!inline && match && match[1] === "mermaid") {
                            return <Mermaid chart={String(children).replace(/\n$/, "")} />;
                          }
                          return !inline ? (
                            <pre className="bg-neutral-100 p-4 rounded-xl overflow-x-auto my-2">
                              <code className={className} {...props}>
                                {children}
                              </code>
                            </pre>
                          ) : (
                            <code className="bg-neutral-100 px-1.5 py-0.5 rounded text-blue-600 font-mono" {...props}>
                              {children}
                            </code>
                          );
                        },
                        table({ children }) {
                          return (
                            <div className="overflow-x-auto my-4 rounded-2xl border border-neutral-100 italic">
                              <table className="min-w-full divide-y divide-neutral-200">
                                {children}
                              </table>
                            </div>
                          );
                        },
                        thead({ children }) {
                          return <thead className="bg-neutral-50">{children}</thead>;
                        },
                        th({ children }) {
                          return (
                            <th className="px-4 py-3 text-left text-xs font-bold text-neutral-500 uppercase tracking-wider">
                              {children}
                            </th>
                          );
                        },
                        td({ children }) {
                          return <td className="px-4 py-3 text-sm text-neutral-600 border-t border-neutral-50">{children}</td>;
                        },
                        p({ children }) {
                          return <p className="mb-2 last:mb-0">{children}</p>;
                        },
                        ul({ children }) {
                          return <ul className="list-disc pl-4 mb-2 space-y-1">{children}</ul>;
                        },
                        li({ children }) {
                          return <li className="text-neutral-700">{children}</li>;
                        }
                      }}
                    >
                      {msg.content}
                    </ReactMarkdown>
                  </div>
                </div>
              ))}
              {isTyping && (
                <div className="flex items-center gap-2 text-xs text-neutral-400 font-medium">
                  <div className="flex gap-1">
                    <div className="w-1.5 h-1.5 bg-neutral-300 rounded-full animate-bounce" />
                    <div className="w-1.5 h-1.5 bg-neutral-300 rounded-full animate-bounce [animation-delay:0.2s]" />
                    <div className="w-1.5 h-1.5 bg-neutral-300 rounded-full animate-bounce [animation-delay:0.4s]" />
                  </div>
                  AI active. Consulting documentation...
                </div>
              )}
            </div>

            {/* Input Area */}
            <div className="p-6 bg-white border-t border-neutral-100">
              <div className="relative group">
                <input
                  type="text"
                  value={input}
                  onChange={(e) => setInput(e.target.value)}
                  onKeyPress={(e) => e.key === "Enter" && handleSend()}
                  placeholder="Ask the Project AI..."
                  className="w-full pl-6 pr-14 py-4 bg-neutral-100 border-none rounded-2xl text-sm focus:ring-2 focus:ring-blue-500 transition-all outline-none"
                />
                <button 
                  onClick={handleSend}
                  className="absolute right-2 top-2 w-10 h-10 bg-neutral-900 hover:bg-blue-600 rounded-xl flex items-center justify-center text-white transition-all group-hover:scale-105 active:scale-95"
                >
                  <Send className="w-4 h-4" />
                </button>
              </div>
            </div>
          </div>

        </div>
      </div>
    </section>
  );
}
