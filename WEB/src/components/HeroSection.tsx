import { Play, ShieldCheck } from "lucide-react";

export default function HeroSection() {
  return (
    <section className="relative w-full h-[85vh] flex flex-col items-center justify-center text-white overflow-hidden shadow-[0_15px_30px_-15px_rgba(0,0,0,0.1)] z-30">
      {/* Video Background */}
      <div className="absolute inset-0 z-0">
        <video
          autoPlay
          loop
          muted
          playsInline
          className="w-full h-full object-cover opacity-60 grayscale-[0.2]"
        >
          <source src="/assets/welding-loop.mp4" type="video/mp4" />
        </video>
        <div className="absolute inset-0 bg-neutral-900/10 z-10" />
      </div>

      {/* Content */}
      <div className="relative z-20 text-center px-4 max-w-4xl animate-in fade-in slide-in-from-bottom-8 duration-1000">
        <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-blue-500/20 border border-blue-500/30 text-blue-600 text-xs font-bold tracking-widest uppercase mb-8 backdrop-blur-md">
          <ShieldCheck className="w-4 h-4" />
          Open Source DIY Project
        </div>
        
        <h1 className="text-6xl md:text-8xl font-black tracking-tighter mb-6 drop-shadow-2xl">
          <span className="text-neutral-900">Wireless</span> <span className="text-blue-500 block md:inline">Welding Pedal</span>
        </h1>
        
        <p className="text-xl md:text-2xl text-neutral-900 font-medium mb-12 max-w-2xl mx-auto leading-tight drop-shadow-md">
          Full Control. Zero Cables. Optimize your TIG/MMA workflow with LoRa technology and high-precision sensors.
        </p>

        <div className="flex flex-col sm:flex-row items-center justify-center gap-6">
          <a
            href="#purchase"
            className="group px-10 py-5 bg-blue-600 hover:bg-blue-500 text-white rounded-2xl font-bold text-lg transition-all hover:scale-105 hover:shadow-2xl hover:shadow-blue-500/40 flex items-center gap-3"
          >
            Start Project
            <Play className="w-5 h-5 fill-current" />
          </a>
          <a
            href="#hardware"
            className="px-10 py-5 bg-white/10 hover:bg-white/20 text-neutral-900 border border-neutral-900/20 rounded-2xl font-bold text-lg backdrop-blur-xl transition-all"
          >
            View Hardware
          </a>
        </div>
      </div>
    </section>
  );
}
